#include <stdarg.h>
#include <stdio.h>

#include "levdyn.h"
#include "things.h"
#include "dyncode.h"

static int first_mismatch(const char *p1,const char *p2,int len) {
   int i=0;
   while(len>0) {
      if(*p1!=*p2) return i;
      i++;
      p1++;
      p2++;
      len--;
   };
   return 32767;
};

DECL_ENCODE(Vertex) {
  return 0;
};
DECL_DECODE(Vertex) {
};

DECL_ENCODE(Side) {
   const SideDyn *d=dyn;
   SideCode *c=code;
   c->las=d->lanimstate;
   c->uas=d->uanimstate;
   c->mas=d->manimstate;
   return sizeof(SideCode);
};
DECL_DECODE(Side) {
   SideDyn *d=dyn;
   const SideCode *c=code;
   d->lanimstate=c->las;
   d->uanimstate=c->uas;
   d->manimstate=c->mas;
};

DECL_ENCODE(Line) {
  return 0;
};
DECL_DECODE(Line) {
};

DECL_ENCODE(Sector) {
   const SectorDyn *d=dyn;
   SectorCode *c=code;
   c->floor=d->floor;
   c->ceiling=d->ceiling;
   c->fas=d->fanimstate;
   c->cas=d->canimstate;
   return sizeof(SectorCode);
};
DECL_DECODE(Sector) {
   SectorDyn *d=dyn;
   const SectorCode *c=code;
   d->floor=c->floor;
   d->ceiling=c->ceiling;
   d->fanimstate=c->fas;
   d->canimstate=c->cas;
};

DECL_ENCODE(Thing) {
   const ThingDyn *d=dyn,*b=backup;
   ThingCode *c=code;
   LE_int16 *c2=code;
   int fm;
   /* don't update dead things */
   if(d->proto==NULL&&b->proto==NULL) return 0;
   /* KLUDGE: we find the first mismatched byte in the thing
              to avoid updating for trivial changes */
   fm=first_mismatch(dyn,backup,sizeof(ThingDyn));
   if(fm>=THING_MAGIC_JELLYBEAN) return 0;
   /* other KLUDGE: phase change shortcut */
   if(fm>=THING_MAGIC_JELLYBEAN-sizeof(int)) {
      *c2=d->phase;
      return sizeof(LE_int16);
   };
   /* now we're ready to encode */
   c->x=d->x;
   c->y=d->y;
   c->z=d->z;
   c->angle=d->angle;
   c->elev=d->elev;
   c->owner=d->owner;
   c->sector=d->sector;
   c->armour=d->armour;
   c->hits=d->hits;
   if(d->proto) c->proto=d->proto->id;
   else c->proto=-1;
   c->phase=d->phase;
   return sizeof(ThingCode);
};
DECL_DECODE(Thing) {
   ThingDyn *d=dyn;
   /* is it a full update? */
   if(len==sizeof(ThingCode)) {
      const ThingCode *c=(const ThingCode *)code;
      d->x=c->x;
      d->y=c->y;
      d->z=c->z;
      d->angle=c->angle;
      d->elev=c->elev;
      d->owner=c->owner;
      d->sector=c->sector;
      d->hits=c->hits;
      d->armour=c->armour;
      if(c->proto==-1) d->proto=NULL;
      else {
	 if(d->proto==NULL||c->proto!=d->proto->id) 
	    thing_become(ld,d-ldthingd(ld),c->proto);
	 d->phase=c->phase;
      };
   }
   /* or just a new phase? */
   else {
      const LE_int16 *c=(const LE_int16 *)code;
      d->phase=*c;
   };
};










