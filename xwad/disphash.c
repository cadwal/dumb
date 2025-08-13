#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "disphash.h"

typedef struct _DHEnt {
   struct _DHEnt *chain;
   Window w;
   DispatchFunc func;
   AppInst *inst;
   void *info;
} DHEnt;

#define HASHSIZE 29

#define HASH(wn) ((wn)%HASHSIZE)

static DHEnt tbl[HASHSIZE];

void init_dh(void) {
   int i;
   for(i=0;i<HASHSIZE;i++) {
      tbl[i].w=None;
      tbl[i].chain=NULL;
   };
};

void add_dh(Window w,DispatchFunc func,AppInst *inst,void *info) {
   DHEnt *dh=tbl+HASH(w);
   while(dh->w!=None&&dh->w!=w&&dh->chain!=NULL) dh=dh->chain;
   if(dh->w!=None&&dh->w!=w) {
      dh->chain=calloc(1,sizeof(DHEnt));
      dh=dh->chain;
   };
   dh->w=w;
   dh->func=func;
   dh->inst=inst;
   dh->info=info;
};

void remove_dh(Window w) {
   DHEnt *dh=tbl+HASH(w);
   while(dh->w!=w&&dh->chain!=NULL) dh=dh->chain;
   if(dh->w==w) dh->w=None;
};

int dispatch(XEvent *ev) {
   Window w=ev->xany.window;
   DHEnt *dh=tbl+HASH(w);
   while(dh->w!=w&&dh->chain!=NULL) dh=dh->chain;
   if(dh->w==w) dh->func(ev,dh->inst,dh->info);
   else return -1;
   return 0;
};

