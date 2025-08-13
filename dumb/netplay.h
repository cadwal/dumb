
#ifndef NETPLAY_H
#define NETPLAY_H

#include "levdata.h"
#include "plat/input.h"
#include "plat/net.h"
#include "dumb/netio.h"
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

#define PROTOCOL_VERSION 1

typedef struct {
   char sig,size;
   char mapname[10];
   char plnum,mplayer;
   char difficulty,protocol_version;
} SlaveInitPkt;
#define SLAVEINIT_REQ_SIG '0'
#define SLAVEINIT_SIG '1'
#define SLAVEINIT_ACK_SIG '2'

typedef struct {
   char sig,size;
   LE_int16 sound;
   LE_int32 x,y,radius;
} DSoundPkt;
#define DSOUND_SIG '!'

typedef struct {
   char sig,size;
   LE_int16 mltype,offset,_spare;
   char code[0];
} UpdatePktHdr;
#define UPDATE_SIG 'U'

typedef struct {
   char sig,size;
   LE_int16 player,offset,value;
} PlayerInfoPkt;
#define PLINFO_SIG 'P'

typedef struct {
   char sig,size;
   LE_int16 _spare;
   LE_int32 tickspassed;
   PlayerInput inp;
} InputPkt;
#define INPUT_SIG 'I'

typedef struct {
   char sig,size;
   LE_int16 _spare;
   LE_int32 tickspassed;
} SyncPkt,AckPkt;
#define SYNC_SIG 'S'
#define ACK_SIG 'A'

#define QUIT_SIG 'Q'
#define MESSAGE_SIG 'M'

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
