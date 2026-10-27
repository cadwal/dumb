/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/win_net.c: Windows network driver, UDP over Winsock 2.
 * Based on unix_net.c.
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "netplay.h"
#include "net.h"

#define UDP_STATISTICS

ConfItem net_conf[] =
{
   CONFI("udp-port", NULL, 0, N_("UDP port to listen on"), 7777),
   CONFS("broadcast", NULL, 0, N_("IP broadcast address"), NULL),
   CONFITEM_END
};
#define cnf_udp_port (net_conf[0].intval)
#define cnf_broadcast (net_conf[1].strval&&net_conf[1].strval[0])
#define cnf_bcast_addr (net_conf[1].strval)

static SOCKET sock = INVALID_SOCKET;
static int wsa_started;
static unsigned char netbuf[NETBUF_LEN];

static struct sockaddr_in bcast_sin;

#ifdef UDP_STATISTICS
static int bpktcount = 0, bbytecount = 0;
static int pktcount = 0, bytecount = 0;
#endif

/* Winsock doesn't set errno, and strerror() doesn't know its codes.  */
static const char *
wsa_strerror(int err)
{
   static char buf[256];
   char *end;
   if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
		       | FORMAT_MESSAGE_IGNORE_INSERTS,
		       NULL, (DWORD) err, 0, buf, sizeof(buf), NULL))
      snprintf(buf, sizeof(buf), "Winsock error %d", err);
   /* FormatMessage() ends the message with a line break.  */
   end = buf + strlen(buf);
   while (end > buf && (end[-1] == '\n' || end[-1] == '\r'
			|| end[-1] == ' ' || end[-1] == '.'))
      *--end = '\0';
   return buf;
}

#define last_error() wsa_strerror(WSAGetLastError())

int
udp_init_station(RemoteStation *rs)
{
   struct addrinfo hints, *ai;
   char *bare_name;		/* without ":port" */
   int port;
   char *colon;
   int err;
   if (!rs->name[0])
      return -1;
   colon = strchr(rs->name, ':');
   if (colon) {
      size_t barelen = colon - rs->name;
      bare_name = (char *) safe_malloc(barelen + 1);
      memcpy(bare_name, rs->name, barelen);
      bare_name[barelen] = '\0';
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
   } else {
      bare_name = safe_strdup(rs->name);
      port = cnf_udp_port;
   }
   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_DGRAM;
   err = getaddrinfo(bare_name, NULL, &hints, &ai);
   if (err != 0) {
      logprintf(LOG_ERROR, 'N',
		_("getaddrinfo() failed for '%s': %s"),
		rs->name, wsa_strerror(err));
      free(bare_name);
      return -1;
   }
   {
      struct sockaddr_in *sin = (struct sockaddr_in *)
	 safe_malloc(rs->addrlen = sizeof(struct sockaddr_in));
      memcpy(sin, ai->ai_addr, sizeof(struct sockaddr_in));
      sin->sin_port = htons((unsigned short) port);
      rs->addr = sin;
      logprintf(LOG_DEBUG, 'N', "udp_init_station: %s -> %s:%d",
		rs->name, inet_ntoa(sin->sin_addr), port);
   }
   freeaddrinfo(ai);
   free(bare_name);
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
udp_recpkt(size_t *len, int *station)
{
   struct sockaddr_in sin;
   int sinlen = sizeof(sin);
   int r, i;
   *len = 0;
   *station = -1;
   r = recvfrom(sock, (char *) netbuf, NETBUF_LEN, 0,
		(struct sockaddr *) &sin, &sinlen);
   if (r == SOCKET_ERROR) {
      int err = WSAGetLastError();
      /* WSAECONNRESET: an earlier packet was refused by a station
	 that isn't listening (yet).  Like ECONNREFUSED on Unix, there
	 is nothing to do but muddle on.  WSAEMSGSIZE: the packet was
	 longer than NETBUF_LEN and has been truncated; drop it.  */
      if (err != WSAEWOULDBLOCK && err != WSAECONNRESET
	  && err != WSAEMSGSIZE)
	 logprintf(LOG_ERROR, 'N', _("recvfrom(): %s"), wsa_strerror(err));
      return NULL;
   }
   for (i = 0; i < nstations; i++) {
      if (stations[i].addrlen == sizeof(struct sockaddr_in) &&
	  stations[i].addr) {
	 struct sockaddr_in *ssin = (struct sockaddr_in *) stations[i].addr;
	 /* Compare the port too: players on one machine share an
	    address.  Each station sends from the port it listens on,
	    which is the port in its address here.  */
	 if (!memcmp(&ssin->sin_addr, &sin.sin_addr,
		     sizeof(struct in_addr))
	     && ssin->sin_port == sin.sin_port) {
	    *len = r;
	    *station = i;
	    return netbuf;
	 }
      }
   }
   return NULL;
}

int
udp_waitpkt(int msec)
{
   fd_set fds;
   struct timeval tv;
   FD_ZERO(&fds);
   FD_SET(sock, &fds);
   tv.tv_sec = msec / 1000;
   tv.tv_usec = (msec % 1000) * 1000;
   /* The first argument is ignored by Winsock.  */
   return select(0, &fds, NULL, NULL, &tv) == SOCKET_ERROR;
}

void
udp_sendpkt(RemoteStation *rs, const void *pkt, size_t len)
{
#ifdef UDP_STATISTICS
   pktcount++;
   bytecount += len;
#endif
   if (sendto(sock, (const char *) pkt, (int) len, 0,
	      (const struct sockaddr *) rs->addr, rs->addrlen)
       == SOCKET_ERROR)
      logprintf(LOG_ERROR, 'N', _("sendto(): %s"), last_error());
}

void
udp_castpkt(const void *pkt, size_t len)
{
#ifdef UDP_STATISTICS
   bpktcount++;
   bbytecount += len;
#endif
   if (sendto(sock, (const char *) pkt, (int) len, 0,
	      (const struct sockaddr *) &bcast_sin, sizeof(bcast_sin))
       == SOCKET_ERROR)
      logprintf(LOG_ERROR, 'N', _("broadcast sendto(): %s"), last_error());
}

char *
win_gethostname(void)
{
   /* Winsock host names are at most 256 bytes.  */
   char *name = (char *) safe_malloc(257);
   if (gethostname(name, 257) == SOCKET_ERROR)
      logprintf(LOG_FATAL, 'N', _("gethostname: %s"), last_error());
   return name;
}

static void
udp_cleanup(void)
{
   if (wsa_started)
      WSACleanup();
   wsa_started = 0;
}

void
udp_init(void)
{
   WSADATA wsadata;
   struct sockaddr_in sin;
   u_long nonblocking = 1;
   int err;
   if (sock != INVALID_SOCKET)
      net_reset();
   if (!wsa_started) {
      err = WSAStartup(MAKEWORD(2, 2), &wsadata);
      if (err != 0)
	 logfatal('N', _("starting Winsock: %s"), wsa_strerror(err));
      wsa_started = 1;
      atexit(udp_cleanup);
   }
#ifdef UDP_STATISTICS
   bpktcount = pktcount = 0;
   bbytecount = bytecount = 0;
#endif
   sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (sock == INVALID_SOCKET)
      logfatal('N', _("creating UDP socket: %s"), last_error());
   memset(&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons((unsigned short) cnf_udp_port);
   if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) == SOCKET_ERROR)
      logfatal('N', _("binding UDP socket: %s"), last_error());
   if (ioctlsocket(sock, FIONBIO, &nonblocking) == SOCKET_ERROR)
      logfatal('N', _("unblocking UDP socket: %s"), last_error());
   if (cnf_broadcast) {
      BOOL yes = TRUE;
      if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
		     (const char *) &yes, sizeof(yes)) == SOCKET_ERROR)
	 logprintf(LOG_ERROR, 'N', _("udp: broadcast access denied: %s"),
		   last_error());
      else {
	 memset(&bcast_sin, 0, sizeof(bcast_sin));
	 bcast_sin.sin_family = AF_INET;
	 bcast_sin.sin_addr.s_addr = inet_addr(cnf_bcast_addr);
	 bcast_sin.sin_port = htons((unsigned short) cnf_udp_port);
	 netdriver[0].slavecast = udp_castpkt;
	 netdriver[0].broadcast = udp_castpkt;
      }
   }
}

void
udp_reset(void)
{
   if (sock != INVALID_SOCKET)
      closesocket(sock);
   sock = INVALID_SOCKET;
#ifdef UDP_STATISTICS
   if (bpktcount)
      logprintf(LOG_INFO, 'N',
		_("udp broadcast: %d packets, %d bytes, avg len %d"),
		bpktcount, bbytecount, bbytecount / bpktcount);
   if (pktcount)
      logprintf(LOG_INFO, 'N',
		_("udp unicast:   %d packets, %d bytes, avg len %d"),
		pktcount, bytecount, bytecount / pktcount);
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
    win_gethostname},
   {NULL,}
};

// Local Variables:
// c-basic-offset: 3
// End:
