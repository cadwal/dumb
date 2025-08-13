/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/unix_net.c: Unix network driver.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

/* FIXME: Use strerror(errno) in error messages.  And check which
 * places must use h_errno instead.  */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "netplay.h"
#include "net.h"

#define UDP_STATISTICS

ConfItem net_conf[] =
{
   /* FIXME: Port 7777 is already officially registered for "cbt",
    * whatever that is.  */
   CONFI("udp-port", NULL, 0, N_("UDP port to listen on"), 7777),
   CONFS("broadcast", NULL, 0, N_("IP broadcast address"), NULL),
   CONFITEM_END
};
#define cnf_udp_port (net_conf[0].intval)
#define cnf_broadcast (net_conf[1].strval&&net_conf[1].strval[0])
#define cnf_bcast_addr (net_conf[1].strval)

/* some local data */
static int sock = -1;
static unsigned char netbuf[NETBUF_LEN];

/* address to send to for broadcasts */
union SIA {
   struct sockaddr sa;
   struct sockaddr_in sin;
};
static union SIA bcast_sa;

#ifdef UDP_STATISTICS
static int bpktcount = 0, bbytecount = 0;
static int pktcount = 0, bytecount = 0;
#endif

int
udp_init_station(RemoteStation *rs)
{
   struct hostent *he;
   struct sockaddr_in *sin;
   char *bare_name;		/* without ":port" */
   int port;
   char *colon;
   if (!rs->name)
      return -1;
   colon = strchr(rs->name, ':');
   if (colon) {
      /* Make a temporary copy of the name without the colon and the port.
       * This could modify rs->name directly but that would be dirty.  */
      size_t barelen = colon - rs->name;
      bare_name = (char *) safe_malloc(barelen + 1);
      memcpy(bare_name, rs->name, barelen);
      bare_name[barelen] = '\0';
      /* TODO: getservbyname() */
      if (sscanf(colon + 1, "%d", &port) == 1) {
	 if (cnf_broadcast)
	    logprintf(LOG_WARNING, 'N',
		      _("Host-specific port numbers will be ignored"
			" when broadcasting"));
      } else {
	 logprintf(LOG_ERROR, 'N',
		   _("Invalid port number `%s' -- assuming %d"),
		   colon + 1, cnf_udp_port);
	 port = cnf_udp_port;
      }
   } else {			/* !colon */
      /* Could use the string directly but this saves a check when
       * returning.  */
      bare_name = safe_strdup(rs->name);
      port = cnf_udp_port;
   }
   he = gethostbyname(bare_name);
   if (he == NULL) {
      /* FIXME: h_errno? */
      logprintf(LOG_ERROR, 'N',
		_("gethostbyname() failed for '%s': errno=%d"),
		rs->name, errno);
      free(bare_name);
      return -1;
   }
   logprintf(LOG_DEBUG, 'N', "udp_init_station: %s -> %d.%d.%d.%d:%d",
	     rs->name,
	     (unsigned char) he->h_addr[0],
	     (unsigned char) he->h_addr[1],
	     (unsigned char) he->h_addr[2],
	     (unsigned char) he->h_addr[3],
	     port);
   sin = (struct sockaddr_in *)
      (rs->addr = safe_malloc(rs->addrlen = sizeof(struct sockaddr_in)));
   sin->sin_family = AF_INET;
   sin->sin_port = htons((unsigned short) port);
   memcpy(&sin->sin_addr, he->h_addr, sizeof(struct in_addr));
   return 0;
}

void
udp_reset_station(RemoteStation *rs)
{
   if (rs->addr)
      safe_free(rs->addr);
   rs->addr = NULL;
   rs->addrlen = 0;
}

const unsigned char *
udp_recpkt(size_t * len, int *station)
{
   union {
      struct sockaddr sa;
      struct sockaddr_in sin;
   } sia;
   size_t sialen = sizeof(sia);
   int r, i;
   *len = 0;
   *station = -1;
   /* FIXME: Is the signedness right?  What does POSIX say?  */
   r = recvfrom(sock, netbuf, NETBUF_LEN, 0, &sia.sa, &sialen);
   if (r == -1) {
      if (errno == EWOULDBLOCK);	/* nothing to receive */
      else if (errno == ECONNREFUSED);	/* one of the stations we're talking
					   to isn't listening.  Since there's
					   no way to work out which, we just
					   muddle on */
      else			/* FIXME: h_errno? */
	 logprintf(LOG_ERROR, 'N', _("error (%d) in recvfrom sock=%d"),
		   errno, sock);
      return NULL;
   }
   /* now find the originating station */
   for (i = 0; i < nstations; i++) {
      if (stations[i].addrlen == sizeof(struct sockaddr_in) &&
	  stations[i].addr) {
	 struct sockaddr_in *sin = (struct sockaddr_in *) stations[i].addr;
	 if (!memcmp(&sin->sin_addr,
		     &sia.sin.sin_addr,
		     sizeof(struct in_addr))) {
	    *len = r;
	    *station = i;
	    /*printf(_("data from station %d of len %d\n"), *station, *len); */
	    return netbuf;
	 }
      }
   }
#if 0				/* sadly, broadcast breaks this :( */
   /* maybe a new station joining? */
   *station = nstations;
   *len = r;
   logprintf(LOG_DEBUG, 'N', _("data from new station (%08x)"),
	     sia.sin.sin_addr);
   return netbuf;
#else
   /* didn't find anything! */
   /*logprintf(LOG_ERROR, 'N', _("unclaimed packet (type=%c)"), netbuf[0]); */
   return NULL;
#endif
}

#if 0
/* this returns the number of bytes waiting on a socket */
int
num_waiting(int fd)
{
   int len = 0;
   ioctl(fd, FIONREAD, &len);
   return (len);
}
#endif

#ifdef HAVE_SELECT
int
udp_waitpkt(int msec)
{
   fd_set fds[1];
   int ret;
   struct timeval tv;
   FD_ZERO(fds);
   FD_SET(sock, fds);
   tv.tv_sec = msec / 1000;
   tv.tv_usec = (msec % 1000) * 1000;
   /* something about this call to select seems to break the HPUX version */
   ret = select(sock + 1, fds, NULL, NULL, &tv);
   /*logprintf(_("got select = %d\n"), ret); */
   return ret < 0;
}
#else  /* !HAVE_SELECT */
int
udp_waitpkt(int msec)
{
   union {
      struct sockaddr sa;
      struct sockaddr_in sin;
   } sia;
   int sialen = sizeof(sia), r;
   /* set blocking mode */
   if (fcntl(sock, F_SETFL, 0) == -1)
      logfatal('N', _("error %d blocking UDP socket"), errno);
   /* now peek at the socket */
   r = recvfrom(sock, netbuf, NETBUF_LEN, MSG_PEEK, &sia.sa, &sialen);
   if (r == -1)
      logprintf(LOG_ERROR, 'N', _("error (%d) in peek recvfrom sock=%d"),
		errno, sock);
   /* unset blocking mode */
   if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
      logfatal('N', _("error %d unblocking UDP socket"), errno);
   return 0;
}
#endif /* !HAVE_SELECT */

void
udp_sendpkt(RemoteStation *rs, const void *pkt, size_t len)
{
#ifdef UDP_STATISTICS
   pktcount++;
   bytecount += len;
#endif
   if (sendto(sock, pkt, len, 0,
	      (const struct sockaddr *) rs->addr, rs->addrlen) == -1)
      logprintf(LOG_ERROR, 'N', _("error (%d) in sendto sock=%d"),
		errno, sock);
}

void
udp_castpkt(const void *pkt, size_t len)
{
#ifdef UDP_STATISTICS
   bpktcount++;
   bbytecount += len;
#endif
   if (sendto(sock, pkt, len, 0, &(bcast_sa.sa), sizeof(bcast_sa)) == -1)
      logprintf(LOG_ERROR, 'N', _("error (%d) in broadcast sendto sock=%d"),
		errno, sock);
}

void
unix_gethostname(char *name, size_t l)
{
   gethostname(name, l);
}

void
udp_init(void)
{
   union {
      struct sockaddr sa;
      struct sockaddr_in sin;
   } sia;
   if (sock >= 0)
      net_reset();
#ifdef UDP_STATISTICS
   bpktcount = pktcount = 0;
   bbytecount = bytecount = 0;
#endif
   /* make socket */
   sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (sock == -1)
      logfatal('N', _("error %d creating UDP socket"), errno);
   /* name it */
   memset(&sia, 0, sizeof(sia));
   sia.sin.sin_family = AF_INET;
   sia.sin.sin_port = htons((unsigned) cnf_udp_port);
   if (bind(sock, &sia.sa, sizeof(sia.sin)))
      logfatal('N', _("error %d binding UDP socket"), errno);
   if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
      logfatal('N', _("error %d unblocking UDP socket"), errno);
   /* set up for broadcast */
#ifndef NO_IP_BROADCAST
   if (cnf_broadcast) {
      int yes = 1;
      /* ask system for broadcast access */
      /* The (const char *) cast is there to make Solaris happy;
       * other systems will then autoconvert the pointer to
       * (const void *).  */
      if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		     (const char *) &yes, sizeof(yes)))
	 logprintf(LOG_ERROR, 'N',
		_("udp: broadcast access denied (try running as root)"));
      else {
	 /* fill in broadcast address */
	 bcast_sa.sin.sin_family = AF_INET;
	 bcast_sa.sin.sin_addr.s_addr = inet_addr(cnf_bcast_addr);
	 bcast_sa.sin.sin_port = htons((unsigned) cnf_udp_port);
	 /* turn on broadcast and slavecast functions */
	 netdriver[0].slavecast = udp_castpkt;
	 netdriver[0].broadcast = udp_castpkt;
      }
   }
#endif /* !NO_IP_BROADCAST */
}

void
udp_reset(void)
{
   if (sock >= 0)
      close(sock);
   sock = -1;
#ifdef UDP_STATISTICS
   if (bpktcount)
      logprintf(LOG_INFO, 'N',
		_("udp broadcast: %d packets, %d bytes, avg len %d"),
		bpktcount, bbytecount, bbytecount / bpktcount);
   if (pktcount)
      logprintf(LOG_INFO, 'N',
		_("udp unicast:   %d packets, %d bytes, avg len %d"),
		pktcount, bytecount, bytecount / pktcount);
#if 0
   if (!bpktcount)
      return;
   pktcount += bpktcount;
   bytecount += bbytecount;
   if (pktcount)
      logprintf(LOG_INFO, 'N',
		_("udp total:     %d packets, %d bytes, avg len %d"),
		pktcount, bytecount, bytecount / pktcount);
#endif
#endif /* UDP_STATISTICS */
}

NetDriver netdriver[2] =
{
   {"UDP",
    udp_init, udp_reset,
    udp_init_station, udp_reset_station,
    udp_recpkt, udp_waitpkt,
    udp_sendpkt,
    NULL, NULL,
    unix_gethostname},
   {NULL}
};

// Local Variables:
// c-basic-offset: 3
// End:
