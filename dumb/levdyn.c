#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "lib/log.h"
#include "lib/fixed.h"
#include "things.h"
#include "levdyn.h"
#include "levinfo.h"
#include "animtex.h"

#define MAPORD_TO_RENDERORD(x) (fixed)(x<<12)

LD_DYNDECL(Vertex) {
   LD_DYNINIT(Vertex,dyn,lump);
   dyn->x=MAPORD_TO_RENDERORD(lump->x);
   dyn->y=MAPORD_TO_RENDERORD(lump->y);
};

LD_DYNDECL(Sector) {
   const char *tname;
   int i,j;
   LD_DYNINIT(Sector,dyn,lump);
   dyn->ceiling=MAPORD_TO_RENDERORD(lump->ceiling);
   dyn->floor=MAPORD_TO_RENDERORD(lump->floor);
   
   /* get floor texture */
   tname=lump->ftexture;
   if(!strncmp(tname,"F_SKY",5)) {
      tname=get_skyname(ld);
      dyn->sky|=2;
   };
   if((dyn->fanim=get_animtex(tname))>=0)
     dyn->ftex=get_anim_texture(dyn->fanim,0);
   else if(dyn->sky&2)
     dyn->ftex=get_wall_texture(tname);
   else 
     dyn->ftex=get_flat_texture(tname);
   
   /* get ceiling texture */
   tname=lump->ctexture;
   if(!strncmp(tname,"F_SKY",5)) {
      tname=get_skyname(ld);
      dyn->sky|=1;
   };
   if((dyn->canim=get_animtex(tname))>=0)
     dyn->ctex=get_anim_texture(dyn->canim,0);
   else if(dyn->sky&1)
     dyn->ctex=get_wall_texture(tname);
   else 
     dyn->ctex=get_flat_texture(tname);
   
   /* calculate "dark" value */
   dyn->dark=INT_TO_FIXED(255-lump->light)/288;

   /* calculate "center" (kludgily) */
   dyn->cent_r=0;
   j=0;
   for(i=0;i<ldnsides(ld);i++) 
      if(ldside(ld)[i].sector==(lump-ldsector(ld))) {
	 LineDyn *l=ldsided(ld)[i].lined;
	 if(l==NULL) continue;
	 j++;
	 dyn->cent_x+=l->cent_x;
	 dyn->cent_y+=l->cent_y;
	 if(dyn->cent_r<l->length) dyn->cent_r=l->length;
      };
   if(j>0) {
      dyn->cent_x/=j;
      dyn->cent_y/=j;
   };
};

static Texture *get_wtex(const char *n) {
   Texture *t;
   if(*n=='-'||*n==0) return NULL;
   t=get_wall_texture(n);
   return t;
};
LD_DYNDECL(Side) {
   LD_DYNINIT(Side,dyn,lump);
   /* upper texture */
   if((dyn->uanim=get_animtex(lump->utexture))>=0)
     dyn->utex=get_anim_texture(dyn->uanim,0);
   else dyn->utex=get_wtex(lump->utexture);
   /* middle texture */
   if((dyn->manim=get_animtex(lump->texture))>=0)
     dyn->mtex=get_anim_texture(dyn->manim,0);
   else dyn->mtex=get_wtex(lump->texture);
   /* lower texture */
   if((dyn->lanim=get_animtex(lump->ltexture))>=0)
     dyn->ltex=get_anim_texture(dyn->lanim,0);
   else dyn->ltex=get_wtex(lump->ltexture);
   /* pointer to my lined */
   dyn->lined=NULL;
};

LD_DYNDECL(Line) {
   LD_DYNINIT(Line,dyn,lump);
   VertexDyn *v1=ldvertexd(ld)+(lump->ver1);
   VertexDyn *v2=ldvertexd(ld)+(lump->ver2);
   dyn->length=fix_pythagoras(v1->x-v2->x,v1->y-v2->y);
   dyn->cent_x=(v1->x+v2->x)/2;
   dyn->cent_y=(v1->y+v2->y)/2;
   if(lump->side[0]>=0) ldsided(ld)[lump->side[0]].lined=dyn;
   /*logprintf(LOG_DEBUG,'M',"initline: line %d length=%f (%f,%f)-(%f,%f)",lump-ldline(ld),
	     FIXED_TO_FLOAT(dyn->length),
	     FIXED_TO_FLOAT(v1->x),
	     FIXED_TO_FLOAT(v1->y),
	     FIXED_TO_FLOAT(v2->x),
	     FIXED_TO_FLOAT(v2->y));*/
};

#define BOUNCEMAX (4*FIXED_ONE)

int find_empty_thing(const LevData *ld)  {
   static int i=0;
   int last=i;
   do {
      i++;
      if(i>=ldnthings(ld)) i=0;
      if(ldthingd(ld)[i].proto==NULL) return i;
   } while(i!=last);
   return -1;
};
int safe_find_empty_thing(const LevData *ld)  {
   int i=find_empty_thing(ld);
   if(i<0)
     logprintf(LOG_FATAL,'O',"Ran out of thing slots");
   return i;
};

int new_thing(const LevData *ld,int prid,fixed x, fixed y, fixed z)  {
   int th=safe_find_empty_thing(ld);
   ThingDyn *td=ldthingd(ld)+th;
   memset(td,0,sizeof(ThingDyn));
   td->x=x;
   td->y=y;
   td->z=z;
   td->bouncemax=BOUNCEMAX;
   thingd_findsector(ld,td);
   td->proto=find_protothing(prid);
   if(prid>0&&td->proto==NULL) 
      logfatal('O',"Couldn't find protothing ID=%d",prid);
   td->owner=td->target=-1;
   if(td->proto)  {
      td->hits=td->proto->hits;
      td->phase_tbl=find_thingphase(td->proto->phase_id,0);
      td->phase=td->proto->signals[TS_INIT];
   };
   return th;
};

/* return one if difficulty level OK for this thing */
static int diffchk(const ThingData *t,const LevData *ld) {
   if((t->flags&THING_MULTI)&&!ld->mplayer) return 0;
   switch(ld->difficulty) {
   case(1):
   case(2):
      if(t->flags&THING_12) return 1;
      else return 0;
   case(3):
      if(t->flags&THING_3) return 1;
      else return 0;
   case(4):
   case(5):
      if(t->flags&THING_45) return 1;
      else return 0;
   };
   logprintf(LOG_ERROR,'O',"diffchk: strange difficulty??? (%d)",
	     ld->difficulty);
   return 0;
};

#define D2R(a) fixmul(INT_TO_FIXED(a)/360,FIXED_2PI)
LD_DYNDECL(Thing) {
   LD_DYNINIT(Thing,dyn,lump);
   /*logprintf(LOG_DEBUG,'O',"init thing type=%d at (%d,%d)",lump->type,lump->x,lump->y);*/
   dyn->x=MAPORD_TO_RENDERORD(lump->x);
   dyn->y=MAPORD_TO_RENDERORD(lump->y);
   dyn->z=0;
   dyn->angle=D2R(lump->angle);
   thingd_findsector(ld,dyn);
   /* check if thing should exist on this skill level */
   if(diffchk(lump,ld))
      dyn->proto=find_protothing(lump->type);
   else 
      dyn->proto=NULL;
   /* if it's the player, fill in ld->player */
   if(dyn->proto&&(dyn->proto->flags&PT_PLAYER)) {
      int mp=ld->mplayer;
      if(mp==0) mp++;
      if(lump->type>mp) dyn->proto=NULL;
      else 
	 init_player(ld,
		     lump->type-1,
		     dyn-ldthingd(ld));
   };
   /* finish off generic initialisation */
   dyn->owner=dyn->target=-1;
   if(dyn->proto) {
      if(dyn->sector>=0) {
	 const SectorDyn *sd=ldsectord(ld)+dyn->sector;
	 if(dyn->proto->flags&PT_HANGING)
	    dyn->z=sd->ceiling-dyn->proto->height;
	 else if(dyn->proto->flags&PT_CAN_FLY)
	    dyn->z=(sd->ceiling+sd->floor)/2;
	 else
	    dyn->z=sd->floor;
      };
      dyn->hits=dyn->proto->hits;
      dyn->phase_tbl=find_thingphase(dyn->proto->phase_id,0);
      dyn->phase=dyn->proto->signals[TS_INIT];
   };
};
