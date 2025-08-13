#include <stdarg.h>
#include <strings.h>
#include <stdio.h>

#include "lib/log.h"
#include "lib/safem.h"
#include "lib/timer.h"
#include "dsound.h"
#include "game.h"
#include "netplay.h"


ConfItem netplay_conf[]={
   CONFI("init-timeout",NULL,0,"msec to wait for slaves to init",1000),
   CONFI("catchup-timeout",NULL,0,"msec to wait for slaves to catch up",1500),
   CONFI("master-timeout",NULL,0,"msec to wait for master",5000),
   {NULL}
};

#define cnf_init_to (netplay_conf[0].intval)
#define cnf_catchup_to (netplay_conf[1].intval)
#define cnf_master_to (netplay_conf[2].intval)


/* slaveinit stuff */

SlaveInitPkt slave_info;
int got_slave_info=0;

void send_slaveinit(const LevData *ld) {
   SlaveInitPkt sip;
   int i,pl=0;
   memset(&sip,0,sizeof(sip));
   sip.sig='1';
   sip.size=sizeof(SlaveInitPkt)-2;
   strncpy(sip.mapname,ld->name,8);
   sip.difficulty=ld->difficulty;
   sip.mplayer=ld->mplayer;
   /* now loop through all the slaves, sending them this packet */
   for(i=0;i<nstations;i++) {
      if(!(stations[i].flags&RS_SLAVE)) continue;
      if(stations[i].flags&RS_LIVE) continue;
      if(stations[i].player<0) {
	 if(pl==ld->localplayer) pl++;
	 stations[i].player=pl++;
      };
      sip.plnum=stations[i].player;
      net_sendto(stations+i,&sip,sizeof(sip));
   };
};

void wait_slaveinit(LevData *ld) {
   int i,n_uninit;
   /* say what's going on */
   logprintf(LOG_INFO,'N',"waiting for slaves to initialise");
   /* clear all live bits so we start from scratch */
   for(i=0;i<nstations;i++) stations[i].flags&=~RS_LIVE;
   do {
      n_uninit=0;
      /*logprintf(LOG_DEBUG,'N',"sending init packets to slaves");*/
      send_slaveinit(ld);
      for(i=0;i<nstations;i++) {
	 if((stations[i].flags&RS_SLAVE)&&!(stations[i].flags&RS_LIVE)) {
	    net_waitpkt(stations+i,cnf_init_to);
	    netplay_poll(ld);
	 };
	 if((stations[i].flags&RS_SLAVE)&&!(stations[i].flags&RS_LIVE))
	    n_uninit++;
      };
   } while(n_uninit);
   logprintf(LOG_INFO,'N',"all slaves up");
};

int wait4master(void) {
   int i;
   for(i=0;i<nstations;i++)
      if(stations[i].flags&RS_MASTER)
	 if(!net_waitpkt(stations+i,cnf_master_to))
	    return 0;
   return 1;
};

void wait_slaveinfo(LevData *ld) {
   logprintf(LOG_INFO,'N',"waiting for slave init info from master");
   while(!got_slave_info) {
      wait4master();
      netplay_poll(ld);
   };
};

/* network play synchronisation stuff */

static int syncsent=0;

void send_nop(void) {
   unsigned char buf[2];
   buf[0]='N';
   buf[1]=0;
   net_sendmaster_nobuf(buf,2);
};

void send_slavequit(void) {
   unsigned char buf[2];
   buf[0]='Q';
   buf[1]=0;
   net_slavecast(buf,2);
   net_slavecast_flush();
};

void send_update(MapLumpType mltype,int offset,int codelen,const void *code) {
   unsigned char buf[DYN_CODE_BUF_LEN+4];
   /*logprintf(LOG_DEBUG,'N',"send_update: mltype=%d which=%d len=%d",
	     mltype,offset,codelen);*/
   buf[0]='U';
   buf[1]=codelen+3;
   buf[2]=mltype;
   buf[3]=offset&0xff;
   buf[4]=(offset>>8)&0xff;
   memcpy(buf+5,code,codelen);
   net_slavecast(buf,codelen+5);
};
void send_plinfo_update(int player,int offset,int value) { 
   unsigned char buf[7];
   buf[0]='P';
   buf[1]=5;
   buf[2]=player;
   buf[3]=offset&0xff;
   buf[4]=(offset>>8)&0xff;
   buf[5]=value&0xff;
   buf[6]=(value>>8)&0xff;
   net_slavecast(buf,7);
};

void send_sync(int ack,int ticks) {
   unsigned char buf[6];
   /*logprintf(LOG_DEBUG,'N',"send_%s: ticks=%d",ack?"ack":"sync",ticks);*/
   buf[0]=(ack?'A':'S'); 
   buf[1]=4;
   buf[2]=ticks&0xff;
   buf[3]=(ticks>>8)&0xff;
   buf[4]=(ticks>>16)&0xff;
   buf[5]=(ticks>>24)&0xff;
   if(ack) net_sendmaster(buf,6);
   else {
      syncsent=ticks;
      net_slavecast(buf,6);
   };
   net_bufflush();
};

void send_dsound(int sound,fixed x,fixed y,fixed radius) {
   DSoundPkt pkt;
   pkt.sig='!';
   pkt.size=sizeof(pkt)-2;
   pkt.sound=sound;
   pkt.x=x;
   pkt.y=y;
   pkt.radius=radius;
   net_slavecast(&pkt,sizeof(pkt));
};

void send_message(int pl,const char *msg) {
   unsigned char buf[256];
   int l=strlen(msg),i;
   if(l>250) l=250;
   buf[0]='M';
   buf[1]=l+2;
   /*buf[2] gets player number */
   strncpy(buf+3,msg,250);
   buf[253]=0;
   for(i=0;i<nstations;i++)
      if(pl==-1||stations[i].player==pl) { 
	 buf[2]=stations[i].player;
	 net_sendto(stations+i,buf,l+4);
      };
};

static int allzero(const void *_s,int l) {
   const char *s=(const char *)_s;
   while(l--) if(*(s++)) return 0;
   return 1;
};
   
void send_input(LevData *ld,const PlayerInput *in,int tickspassed) {
   unsigned char buf[sizeof(PlayerInput)+sizeof(int)+2];
   unsigned char *s=buf;
   if(allzero(in,sizeof(PlayerInput))) return;
   *s++='I';
   *s++=sizeof(PlayerInput)+sizeof(int);
   memcpy(s,in,sizeof(PlayerInput));s+=sizeof(PlayerInput);
   (*(LE_int32*)s)=tickspassed;s+=sizeof(int);
   net_sendmaster(buf,sizeof(PlayerInput)+sizeof(int)+2);
};


int netplay_wait4slaves(void) {
   int i;
   for(i=0;i<nstations;i++)
      if((stations[i].flags&RS_SLAVE)&&
	 stations[i].ackstate!=0&&
	 stations[i].ackstate<syncsent-1000/MSEC_PER_TICK) {
	 logprintf(LOG_DEBUG,'N',"waiting for slave '%s' to catch up",
		   stations[i].name);
	 if(!net_waitpkt(stations+i,cnf_catchup_to)) {
	    logprintf(LOG_ERROR,'N',"slave '%s' seems to be dead",
		      stations[i].name);
	    stations[i].flags&=~(RS_SLAVE|RS_LIVE);
	 };
	 return 1;
      };
   return 0;
};


/* receive stuff */

static void play_dspkt(const DSoundPkt *pkt) {
   play_dsound_local(pkt->sound,pkt->x,pkt->y,pkt->radius);
};

void netplay_poll(LevData *ld) {
   const unsigned char *code;
   size_t pktlen;
   int station;
   RemoteStation *rs;
   while(1) {
      code=net_recpkt(&pktlen,&station);
      if(pktlen==0&&!netplay_wait4slaves()) break;
      rs=stations+station;
      rs->flags|=RS_LIVE;
      while(pktlen>0) {
	 int codelen=code[1];
	 if(codelen>pktlen)
	    logprintf(LOG_ERROR,'N',"inconsistent packet length %d > %d",
		      codelen,pktlen);
	 switch(code[0]) {
	 case('U'):
	    /* this will probably cause unaligned accesses on the alpha
	       get around the problem by memcpying code into an aligned
	       buffer? */
	    apply_update(ld,
			 code[2],
			 code[3]+((unsigned int)(code[4])<<8),
			 code+5,codelen-3);
	    break;
	 case('P'):
	    apply_plinfo_update(ld,code[2],
				code[3]+(((unsigned int)code[4])<<8),
				code[5]+(((unsigned int)code[6])<<8));
	    break;
	 case('Q'):
	    game_want_quit(1);
	    rs->flags&=~RS_LIVE;
	    break;
	 case('S'):
	    ld->map_ticks=*((LE_int32*)(code+2));
	    break;
	 case('A'):
	    rs->ackstate=*((LE_int32*)(code+2));
	    /*logprintf(LOG_DEBUG,'N',"ack %d from %s",
	                rs->ackstate,rs->name);*/
	    break;
	 case('I'):
	    /* deal with input packets here */
	    process_input(ld,
			  (const void *)(code+2),
			  *(const LE_int32 *)(code+2+sizeof(PlayerInput)),
			  rs->player);
	    break;
	 case('1'):
	    /* slave init */
	    memcpy(&slave_info,code,sizeof(slave_info));
	    got_slave_info++;
	    send_nop();
	    break;
	 case('N'):
	    /* nop, send to make sure other end knows we're live */
	    break;
	 case('M'):
	    /* reinject game message */
	    game_message(code[2],"%s",code+3);
	    break;
	 case('!'):
	    play_dspkt((const DSoundPkt*)code);
	    break;

	 default:
	    logprintf(LOG_ERROR,'N',
		      "strange packet type (%c) in netplay_poll()",
		      code[0]);
	    break;
	 };
	 code+=codelen+2;
	 pktlen-=codelen+2;
      };
   };
};
