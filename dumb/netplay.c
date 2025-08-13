#include <stdarg.h>
#include <strings.h>
#include <stdio.h>
#include <time.h>

#include "lib/log.h"
#include "lib/safem.h"
#include "lib/timer.h"
#include "dsound.h"
#include "game.h"
#include "things.h"
#include "netplay.h"


ConfItem netplay_conf[]={
   CONFI("init-timeout",NULL,0,"msec to wait for slaves to init",1000),
   CONFI("catchup-timeout",NULL,0,"msec to wait for slaves to catch up",2000),
   CONFI("master-timeout",NULL,0,"msec to wait for master",5000),
   CONFI("slave-timeout",NULL,0,"msec to allow slaves to get behind",1000),
   CONFB("async",NULL,0,"run network asychronously"),
   CONFI("ack-wait",NULL,0,"time between sending acks",400),
   {NULL}
};

#define cnf_init_to (netplay_conf[0].intval)
#define cnf_catchup_to (netplay_conf[1].intval)
#define cnf_master_to (netplay_conf[2].intval)
#define cnf_slave_to (netplay_conf[3].intval)
#define cnf_async (netplay_conf[4].intval)
#define cnf_ack_to (netplay_conf[5].intval)

/* slaveinit stuff */

SlaveInitPkt slave_info;
int got_slave_info=0;

void send_one_slaveinit(const LevData *ld, int station) {
   SlaveInitPkt sip;
   int pl=0;
   memset(&sip,0,sizeof(sip));
   sip.sig='1';
   sip.size=sizeof(SlaveInitPkt)-2;
   strncpy(sip.mapname,ld->name,8);
   sip.difficulty=ld->difficulty;
   sip.mplayer=ld->mplayer;

   if(!(stations[station].flags&RS_SLAVE)) return;
   if(stations[station].flags&RS_INIT) return;
   if(!ld->mplayer) stations[station].player=0;
   else if(stations[station].player<0) {
	   if(pl==ld->localplayer) pl++;
	   stations[station].player=pl++;
   };

   logprintf(LOG_DEBUG,'N',"sending slave-init to %s (station %d)",
	     stations[station].name,station);
   sip.plnum=stations[station].player;
   net_sendto(stations+station,&sip,sizeof(sip));
};

void send_slaveinit(const LevData *ld) {
   int i;

   logprintf(LOG_DEBUG,'N',"sending slave-init packets...");
   /* now loop through all the slaves, sending them the init packet */
   for(i=0;i<nstations;i++) {
	   send_one_slaveinit(ld, i);
   };
};

void wait_slaveinit(LevData *ld) {
   int i,n_uninit;
   time_t last_send=0;
   /* say what's going on */
   logprintf(LOG_INFO,'N',"waiting for slaves to initialise");
   /* clear all live bits so we start from scratch */
   for(i=0;i<nstations;i++) stations[i].flags&=~RS_INIT;
   do {
      n_uninit=0;
      /*logprintf(LOG_DEBUG,'N',"sending init packets to slaves");*/
      if(time(NULL)>last_send) {
	 send_slaveinit(ld);
	 last_send=time(NULL);
      };
      for(i=0;i<nstations;i++) {
	 if((stations[i].flags&RS_SLAVE)&&!(stations[i].flags&RS_INIT)) {
	    net_waitpkt(stations+i,cnf_init_to);
	    netplay_poll(ld);
	 };
	 if((stations[i].flags&RS_SLAVE)&&!(stations[i].flags&RS_INIT))
	    n_uninit++;
      };
   } while(n_uninit);
   logprintf(LOG_INFO,'N',"all slaves up");
};

int wait4master(void) {
   int i;
   for(i=0;i<nstations;i++)
      if(stations[i].flags&RS_MASTER)
	 if(net_waitpkt(stations+i,cnf_master_to))
	    return 0;
   return 1;
};

void send_initrequest(char *name) {
   unsigned char buf[100];
   int len = strlen(name);
   logprintf(LOG_DEBUG,'N',"sending init-request");
   buf[0]='0';
   if (len > sizeof(buf)-3) {
	   len = sizeof(buf)-3;
   }
   buf[1]=len+1;
   strncpy(buf+2,name,len);
   buf[2+len] = 0;
   net_sendmaster_nobuf(buf,2+len+1);
};

void wait_slaveinfo(LevData *ld) {
   time_t last_send=0;
   logprintf(LOG_INFO,'N',"waiting for slave init info from master");
   while(!got_slave_info) {
      char myname[100];
      /* yes, I haven't included the right headers for gethostname()
	 this is so I'll be reminded that this stuff shouldn't be
	 at this layer: gethostname() is inet-dependent and should be
	 called from unix_net(), not here */
      gethostname(myname,sizeof(myname)-1);
      if(time(NULL)>last_send) {
	 send_initrequest(myname);
	 last_send=time(NULL);
      };
      wait4master();
      netplay_poll(ld);
   };
};

/* network play synchronisation stuff */

static int syncsent=0;

void send_empty(char ch) {
   unsigned char buf[2];
   buf[0]=ch;
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
   if(ack) {
     static last_ack=0;
     if(cnf_async) return;
     if(ticks-last_ack<cnf_ack_to/MSEC_PER_TICK) return;
     last_ack=ticks;
   };
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

/* play_dsound() was originally in dsound.c but I moved it here
 * because it needs send_dsound() and XProtoThing isn't linked with
 * this network stuff.  Call this a hack if you want but it works.  
 */
void play_dsound(int i,fixed x,fixed y,fixed r) {
   play_dsound_local(i,x,y,r);
   send_dsound(i,x,y,r);
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
   /* save network traffic: only send first of a series of "no action" pkts */
   static int last_was_zero=0;
   if(allzero(in,sizeof(PlayerInput))) {
      if(last_was_zero) return;
      last_was_zero=1;
   }
   else last_was_zero=0;
   /* OK, now send the packet */
   *s++='I';
   *s++=sizeof(PlayerInput)+sizeof(int);
   memcpy(s,in,sizeof(PlayerInput));s+=sizeof(PlayerInput);
   (*(LE_int32*)s)=tickspassed;s+=sizeof(int);
   net_sendmaster(buf,sizeof(PlayerInput)+sizeof(int)+2);
};


int netplay_wait4slaves(void) {
   int i;
   for(i=0;i<nstations;i++)
      if((stations[i].flags&RS_SLAVE)&&(stations[i].flags&RS_LIVE)&&
	 stations[i].ackstate!=0&&
	 stations[i].ackstate<syncsent-cnf_slave_to/MSEC_PER_TICK) {
	 logprintf(LOG_DEBUG,'N',"waiting for slave '%s' (%d) to catch up",
		   stations[i].name,i);
	 if(!net_waitpkt(stations+i,cnf_catchup_to)) {
	    logprintf(LOG_ERROR,'N',"slave '%s' (%d) seems to be dead",
		      stations[i].name,i);
	    stations[i].flags&=~RS_LIVE;
	 };
	 return 1;
      };
   return 0;
};

/* join stuff */

void netplay_join(LevData *ld, int station, const char *name) {
	net_initstation(station, name, RS_SLAVE | RS_LIVE);
	send_one_slaveinit(ld, station);
}

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
      /* try to get a new packet */
      code=net_recpkt(&pktlen,&station);

      /* synchronisation */ 
      if(pktlen==0) {
	if(cnf_async) break;
	else if(netplay_wait4slaves()) continue;
	else break;
      };
      /* deal with packet */
      rs=stations+station;
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
	 case('M'):
	    /* reinject game message */
	    game_message(code[2],"%s",code+3);
	    break;
	 case('!'):
	    play_dspkt((const DSoundPkt*)code);
	    break;

	    /* initialisation sequence */
	 case('0'):
	    /* "I wanna be your slave" */
	    if(rs->flags&RS_LIVE)
	      logprintf(LOG_INFO,'N',"init request from %s ignored",rs->name);
	    else {
	      logprintf(LOG_INFO,'N',
			"got init request: setting up new station %d",station);
	      netplay_join(ld, station, code+2);
	    };
	    break;
	 case('1'):
	    /* slave_init packet */
	    memcpy(&slave_info,code,sizeof(slave_info));
	    got_slave_info++;
	    send_empty('2');
	    break;
	 case('2'):
	    /* "I got your slave init packet" */
	    rs->flags|=RS_INIT;
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
      if(!(rs->flags&RS_LIVE)) {
	logprintf(LOG_DEBUG,'N',"station %s (%d) lives!",rs->name,station);
	rs->flags|=RS_LIVE;
	if(rs->player>=0) touch_player(ld,rs->player);
      };
   };
};
