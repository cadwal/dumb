#include <stdarg.h>
#include <strings.h>
#include <stdio.h>

#include "lib/log.h"
#include "lib/safem.h"
#include "netio.h"
#include "plat/net.h"

/* netio stuff (move to srcfile of its own???) */

#define MAX_STATIONS 16
int nstations=0;
RemoteStation *stations=NULL;

void net_init(void) {
   int i;
   stations=(RemoteStation *)safe_calloc(MAX_STATIONS,sizeof(RemoteStation));
   for(i=0;netdriver[i].name;i++)
      netdriver[i].init();
};
void net_reset(void) {
   int i;
   net_bufflush();
   for(i=0;i<nstations;i++) 
      stations[i].driver->reset_station(stations+i);
   for(i=0;netdriver[i].name;i++)
      netdriver[i].reset();
};

int net_initstation(int station, const char *name, int flags) {
   RemoteStation *rs;
   int i;
   if(station<0) {
     if(nstations>=MAX_STATIONS) {
       logprintf(LOG_ERROR,'N',"too many stations");
       return -1;
     };
     station=nstations;
   };
   rs=stations+station;
   strncpy(rs->name,name,RS_NAME_LEN);
   rs->name[RS_NAME_LEN-1]=0;
   rs->flags=flags;
   rs->ackstate=0;
   rs->player=-1;
   for(i=0;netdriver[i].name;i++)
      if(!netdriver[i].init_station(rs)) {
	 rs->driver=netdriver+i;
	 if(station>=nstations) nstations=station+1;
	 return station;
      };
   return -1;
};

const unsigned char *net_recpkt(size_t *pktlen,int *station) {
   int i;
   *pktlen=0;
   *station=-1;
   for(i=0;netdriver[i].name;i++) {
      const unsigned char *pkt=netdriver[i].recpkt(pktlen,station);
      if(pkt) return pkt;
   };
   return NULL;
};

int net_waitpkt(RemoteStation *rs,int msec) {
   return rs->driver->waitpkt(msec);
};

void net_sendto(RemoteStation *rs,const void *pkt,size_t len) {
   rs->driver->sendpkt(rs,pkt,len);
};

void net_sendmaster_nobuf(const void *pkt,size_t len) {
   int i;
   for(i=0;i<nstations;i++)
      if(stations[i].flags&RS_MASTER)
	 net_sendto(stations+i,pkt,len);
};
void net_slavecast_nobuf(const void *pkt,size_t len) {
   /* TODO: take advantage of driver slavecast() func */
   int i;
   for(i=0;i<nstations;i++)
      if((stations[i].flags&RS_SLAVE)&&(stations[i].flags&RS_LIVE))
	 net_sendto(stations+i,pkt,len);
};

/* buffered send functions */
static unsigned char scbuf[NETBUF_LEN];
static int sclen=0;
static NetSendFunc scsend=NULL;

void net_bufsend(NetSendFunc func,const void *pkt,size_t len) {
   if(scsend!=func) {
      net_bufflush();
      scsend=func;
   };
   if(len+sclen>NETBUF_LEN) net_bufflush();
   memcpy(scbuf+sclen,pkt,len);
   sclen+=len;
};
void net_bufflush(void) {
   if(scsend&&sclen>0) scsend(scbuf,sclen);
   sclen=0;
};

