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

#include "lib/log.h"
#include "lib/safem.h"
#include "plat/net.h"

#define UDP_STATISTICS

ConfItem net_conf[]={
   CONFI("udp-port",NULL,0,"UDP port to listen on",7777),
   {NULL}
};
#define cnf_udp_port (net_conf[0].intval)

static int sock=-1;
static unsigned char netbuf[NETBUF_LEN];

#ifdef UDP_STATISTICS
static int pktcount=0,bytecount=0;
#endif

int udp_init_station(RemoteStation *rs) {
   struct hostent *he;
   struct sockaddr_in *sin;
   if(!rs->name) return -1;
   he=gethostbyname(rs->name);
   if(he==NULL) {
      logprintf(LOG_ERROR,'N',"gethostbyname() failed for '%s': errno=%d",
		rs->name,errno);
      return -1;
   };
   logprintf(LOG_DEBUG,'N',"udp_init_station: %s -> %d.%d.%d.%d",
	     rs->name,(unsigned char)he->h_addr[0],
	     (unsigned char)he->h_addr[1],
	     (unsigned char)he->h_addr[2],
	     (unsigned char)he->h_addr[3]);
   sin=rs->addr=safe_malloc(rs->addrlen=sizeof(struct sockaddr_in));
   sin->sin_family=AF_INET;
   sin->sin_port=htons((unsigned)cnf_udp_port);
   memcpy(&sin->sin_addr,he->h_addr,sizeof(struct in_addr));
   return 0;
};
void udp_reset_station(RemoteStation *rs) {
   if(rs->addr)
      safe_free(rs->addr);
   rs->addr=NULL;
   rs->addrlen=0;
};

void udp_init(void) {
   union {
      struct sockaddr sa;
      struct sockaddr_in sin;
   } sia;
   if(sock>=0) net_reset();
#ifdef UDP_STATISTICS
   pktcount=0;
   bytecount=0;
#endif
   /* make socket */
   sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
   if(sock==-1) logfatal('N',"error %d creating UDP socket",errno);
   /* name it */
   memset(&sia,0,sizeof(sia));
   sia.sin.sin_family=AF_INET;
   sia.sin.sin_port=htons((unsigned)cnf_udp_port);
   if(bind(sock,&sia.sa,sizeof(sia.sin)))
      logfatal('N',"error %d binding UDP socket",errno);
   if(fcntl(sock,F_SETFL,O_NONBLOCK)==-1)
      logfatal('N',"error %d unblocking UDP socket",errno);
};
void udp_reset(void) {
   if(sock>=0) close(sock);
   sock=-1;
#ifdef UDP_STATISTICS
   if(pktcount)
      logprintf(LOG_INFO,'N',"udp: %d packets, %d bytes, avg pktlen %d",
		pktcount,bytecount,bytecount/pktcount);
#endif
};

const unsigned char *udp_recpkt(size_t *len,int *station) {
   union {
      struct sockaddr sa;
      struct sockaddr_in sin;
   } sia;
   int sialen=sizeof(sia),r,i;
   *len=0;*station=-1;
   r=recvfrom(sock,netbuf,NETBUF_LEN,0,&sia.sa,&sialen);
   if(r==-1) {
      if(errno==EWOULDBLOCK); /* nothing to recieve */
      else if(errno==ECONNREFUSED); /* one of the stations we're talking
				       to isn't listening.  Since there's
				       no way to work out which, we just
				       muddle on */
      else
	 logprintf(LOG_ERROR,'N',"error (%d) in recvfrom sock=%d",
		   errno,sock);
      return NULL;
   };
   /* now find the originating station */
   for(i=0;i<nstations;i++) {
      if(stations[i].addrlen==sizeof(struct sockaddr_in)&&
	 stations[i].addr) {
	 struct sockaddr_in *sin=stations[i].addr;
	 if(!memcmp(&sin->sin_addr,
		    &sia.sin.sin_addr,
		    sizeof(struct in_addr))) {
	    *len=r;
	    *station=i;
	    return netbuf;
	 };
      };
   };
   /* didn't find anything! */
   logprintf(LOG_ERROR,'N',"unclaimed packet (type=%c)",netbuf[0]);
   return NULL;
};

#ifndef NO_SELECT
int udp_waitpkt(int msec) {
   fd_set fds[1];
   struct timeval tv;
   FD_ZERO(fds);
   FD_SET(sock,fds);
   tv.tv_sec=msec/1000;
   tv.tv_usec=(msec%1000)*1000;
   return select(sock+1,fds,NULL,NULL,&tv);
};
#else
int udp_waitpkt(int msec) {
   union {
      struct sockaddr sa;
      struct sockaddr_in sin;
   } sia;
   int sialen=sizeof(sia),r;
   /* set blocking mode */
   if(fcntl(sock,F_SETFL,0)==-1)
      logfatal('N',"error %d blocking UDP socket",errno);
   /* now peek at the socket */
   r=recvfrom(sock,netbuf,NETBUF_LEN,MSG_PEEK,&sia.sa,&sialen);
   if(r==-1)
      logprintf(LOG_ERROR,'N',"error (%d) in peek recvfrom sock=%d",
		errno,sock);
   /* unset blocking mode */
   if(fcntl(sock,F_SETFL,O_NONBLOCK)==-1)
      logfatal('N',"error %d unblocking UDP socket",errno);
   return 0;
};
#endif

void udp_sendpkt(RemoteStation *rs,const void *pkt,size_t len) {
#ifdef UDP_STATISTICS
   pktcount++;
   bytecount+=len;
#endif
   sendto(sock,pkt,len,0,rs->addr,rs->addrlen);
};

NetDriver netdriver[2]={
   {"UDP",
    udp_init,udp_reset,
    udp_init_station,udp_reset_station,
    udp_recpkt,udp_waitpkt,
    udp_sendpkt,
    NULL,NULL},
   {NULL}
};



