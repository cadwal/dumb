
#ifndef NETPLAY_H
#define NETPLAY_H

#include "levdata.h"
#include "plat/input.h"
#include "netio.h"
#include "lib/conf.h"

extern ConfItem netplay_conf[];

#define RS_NAME_LEN 64

typedef struct RemoteStation_struct {
   NetDriver *driver;
   void *addr;
   int addrlen;
   int flags;
   int ackstate;
   int player;
   char name[RS_NAME_LEN];
} RemoteStation;

#define RS_SLAVE 0x0001          /* this station is enslaved to me */
#define RS_MASTER 0x0002         /* I'm enslaved to this station */
#define RS_WADSERVER 0x0004      /* this station can serve lumps to me */
#define RS_LIVE 0x0008           /* this station seems to be live */
#define RS_INIT 0x0010           /* station has ackked an init packet */

extern RemoteStation *stations;
extern int nstations;

typedef struct {
   char sig,size;
   char mapname[10];
   char plnum,mplayer;
   char difficulty;
} SlaveInitPkt;

typedef struct {
   char sig,size;
   LE_int32 sound,x,y,radius;
} DSoundPkt;

extern SlaveInitPkt slave_info;
extern int got_slave_info;

void send_slaveinit(const LevData *ld);

void send_input(LevData *ld,const PlayerInput *in,int tickspassed);
void send_update(MapLumpType mltype,int offset,int codelen,const void *code);
void send_plinfo_update(int player,int offset,int value);
void send_sync(int ack,int ticks);
void send_slavequit(void);
void send_message(int player,const char *msg);
void send_dsound(int sound,fixed x,fixed y,fixed radius);

void wait_slaveinfo(LevData *ld); /* slave call to wait for info */
int wait4master(void);
void wait_slaveinit(LevData *ld); /* master call to wait for slaves to wake */

void netplay_poll(LevData *ld);
void netplay_dispatch(LevData *ld,const char *pkts,size_t len);

void netplay_init(const char *name,int flags);
void netplay_reset(void);

#endif
