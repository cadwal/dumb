
#ifndef PLAT_NET_H
#define PLAT_NET_H

#include "libdumbutil/confdef.h"

struct RemoteStation_struct;

typedef struct {
   const char *name;
   void (*init)(void);
   void (*reset)(void);
   int (*init_station)(struct RemoteStation_struct *rs);
   void (*reset_station)(struct RemoteStation_struct *rs);
   const unsigned char *(*recpkt)(size_t *size,int *station);
   int (*waitpkt)(int msec);
   void (*sendpkt)(struct RemoteStation_struct *rs,
		   const void *pkt,size_t size);
   void (*slavecast)(const void *pkt,size_t size);
   void (*broadcast)(const void *pkt,size_t size);
   void (*getmyhost)(char *buf,size_t len);
} NetDriver;

extern NetDriver netdriver[];

extern ConfItem net_conf[];

#endif
