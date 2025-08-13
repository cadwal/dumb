#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "lib/log.h"
#include "levdyn.h"
#include "animtex.h"
#include "dsound.h"
#include "updmap.h"
#include "things.h"
#include "linetype.h"
#include "game.h"

/*#define UPDMAP_DEBUG*/

/* change sector types */
void change_sector_type(LevData *ld,int sector,int type) {
   SectorDyn *sd=ldsectord(ld)+sector;
   /* shut down old sector type */
   /* change type */
   sd->type=type;
   /* start up new type */
};

/* deal with ML_SECTOR events */
static void do_sector_event(LevData *ld,MapEvent *me,int tickspassed) {
   SectorDyn *sd=ldsectord(ld)+me->entity,*mod=NULL;
   const fixed delta=me->curdelta*tickspassed;
   const fixed term=me->curterm;
   fixed *q=NULL;

   /* deal with texture and type changes */
   if(me->model>=0) mod=ldsectord(ld)+me->model;
   switch(me->type) {
   case(ME_CEILING_TYPE):
      if(mod) change_sector_type(ld,me->entity,mod->type);
      /* fall through */
   case(ME_CEILING_TEX):
      if(mod) {
#ifdef UPDMAP_DEBUG
	 logprintf(LOG_DEBUG,'M',"copying ceiling texture from %d to %d",
		   me->model,me->entity);
#endif
	 sd->ctex=mod->ctex;
	 sd->canim=mod->canim;
	 sd->canimstate=mod->canimstate;
      };
      me->type=ME_NONE;
      return;
   case(ME_FLOOR_TYPE):
      if(mod) change_sector_type(ld,me->entity,mod->type);
      /* fall through */
   case(ME_FLOOR_TEX):
      if(mod) {
#ifdef UPDMAP_DEBUG
	 logprintf(LOG_DEBUG,'M',"copying floor texture from %d to %d",
		   me->model,me->entity);
#endif
	 sd->ftex=mod->ftex;
	 sd->fanim=mod->fanim;
	 sd->fanimstate=mod->fanimstate;
      };
      me->type=ME_NONE;
      return;
   };

   /* check that it's not time to stop */
   if(me->curdelta==0) {
#ifdef UPDMAP_DEBUG
      logprintf(LOG_DEBUG,'M',"sector event terminated OK s=%d type=%d",
		me->entity,me->type);
#endif
      me->type=ME_NONE;
   };
   /* work out what we're acting on */
   switch(me->type)  {
   case(ME_NONE): break;
   case(ME_CEILING): q=&(sd->ceiling); break;
   case(ME_FLOOR): q=&(sd->floor); break;
   case(ME_LIGHT): q=&(sd->dark); break;
   default:
      logprintf(LOG_ERROR,'M',"do_sector_event: strange eventtype (%d)",
		me->type);
      me->type=ME_NONE;
   };
   /* act on it */
   if(q) {
      *q+=delta;
      if((delta<0&&*q<=term)||
	 (delta>0&&*q>=term)) {
	 /* go to next stage */
	 *q=term;
	 NEXT_STAGE(me);
      };
   };
   /* do start sound (only if the event didn't terminate) */
   if(me->type!=ME_NONE&&me->stage>=0&&me->sound>=0) {
      if(sd->cent_r==0) 
	 logprintf(LOG_DEBUG,'M',"sector %d has no center?",me->entity);
      else 
	play_dsound(me->sound,sd->cent_x,sd->cent_y,sd->cent_r);
      me->sound=-1;
   };
   /* do stop sound (only if start sound got to play) */
   if(me->type==ME_NONE&&me->sound<0&&me->stopsound>=0) {
      if(sd->cent_r==0) 
	 logprintf(LOG_DEBUG,'M',"sector %d has no center?",me->entity);
      else 
	play_dsound(me->stopsound,sd->cent_x,sd->cent_y,sd->cent_r);
      me->stopsound=-1;
   };
};

/* deal with ML_THING events */
static void do_thing_event(LevData *ld,MapEvent *me) {
   ThingDyn *td=ldthingd(ld)+me->entity;
   switch(me->type)  {
   case(ME_NEWLEVEL):
      game_want_newlvl(0);
      me->type=ME_NONE;
      break;
   case(ME_SECRETLEVEL):
      game_want_newlvl(1);
      me->type=ME_NONE;
      break;
   case(ME_TELEPORT):
      logprintf(LOG_DEBUG,'M',"do_thing_event: teleporting %d",me->entity);
      td->x=me->x;
      td->y=me->y;
      td->z=me->z;
      td->angle=me->angle;
      thingd_findsector(ld,td);
      me->type=ME_NONE;
      break;
   default:
      logprintf(LOG_ERROR,'M',"do_thing_event: strange eventtype (%d)",
		me->type); 
      me->type=ME_NONE;
   };
};

/* deal with ML_SIDE events */
static void do_side_event(LevData *ld,MapEvent *me) {
   SideDyn *sd=ldsided(ld)+me->entity;
   switch(me->type)  {
   case(ME_SWITCHON):
      me->type=ME_NONE;
      if(sd->manimstate==1) return;
      logprintf(LOG_DEBUG,'M',"do_side_event: switching %d on",me->entity);
      sd->uanimstate=1;
      sd->manimstate=1;
      sd->lanimstate=1;
      break;
   case(ME_SWITCHOFF):
      me->type=ME_NONE;
      if(sd->manimstate==0) return;
      logprintf(LOG_DEBUG,'M',"do_side_event: switching %d off",me->entity);
      sd->uanimstate=0;
      sd->manimstate=0;
      sd->lanimstate=0;
      break;
   default:
      logprintf(LOG_ERROR,'M',"do_side_event: strange eventtype (%d)",
		me->type); 
      me->type=ME_NONE;
   };
   /* deal with sounds */
   if(me->sound>=0) {
      play_dsound(me->sound,
		  sd->lined->cent_x,sd->lined->cent_y,
		  sd->lined->length/2);
      me->sound=-1;
   };
};

static MapEvent bogus_event;

MapEvent *insert_event(LevData *ld,
		       MapLumpType lumptype,
		       MapEventType etype,
		       int entity,const void *key) {
   int i;
   MapEvent *me;
   for(i=0;i<MAXEVENTS;i++)
      if(!EVENTVALID(ld->event[i])) break;
   if(i>=MAXEVENTS) {
      logprintf(LOG_ERROR,'M',"No free event in insert_event(%d,%d,%d)",
		lumptype,etype,entity);
      return &bogus_event;
   };
   me=ld->event+i;
   /* init event */
   /*logprintf(LOG_DEBUG,'M',"insert_event(%d,%d,%d)",lumptype,etype,entity);*/
   memset(me,0,sizeof(MapEvent));
   me->ltype=lumptype;
   me->type=etype;
   me->entity=entity;
   me->start=ld->map_ticks+1;
   me->stop=me->start+65536;
   me->stopsound=me->sound=-1;
   me->stage=-2;
   me->key=key;
   return me;
};

int unqueue_event(LevData *ld,
		  MapLumpType lumptype,
		  MapEventType etype,
		  int entity,const void *key) {
   int i,j=0;
   for(i=0;i<MAXEVENTS;i++) {
      if(!EVENTVALID(ld->event[i])) continue;
      if(ld->event[i].ltype==lumptype&&
	 ld->event[i].type==etype&&
	 ld->event[i].entity==entity&&
	 (key==NULL||ld->event[i].key==key)) {
	 /*logprintf(LOG_DEBUG,'M',"event %d unqueued",i);*/
	 j++;
	 memset(ld->event+i,0,sizeof(MapEvent));
      };
   };
   return j;
};

MapEvent *find_active_event(LevData *ld,
			    MapLumpType lumptype,
			    MapEventType etype,
			    int entity) {
   int i;
   for(i=0;i<MAXEVENTS;i++) {
      if(!EVENTVALID(ld->event[i])) continue;
      if(ld->event[i].start>ld->map_ticks) continue;
      if(ld->event[i].ltype==lumptype&&
	 ld->event[i].type==etype&&
	 ld->event[i].entity==entity) return ld->event+i;
   };
   return NULL;
};

int find_event(LevData *ld,
		     MapLumpType lumptype,
		     MapEventType etype,
		     int entity) {
   int i,j=0;
   for(i=0;i<MAXEVENTS;i++) {
      if(!EVENTVALID(ld->event[i])) continue;
      if(ld->event[i].ltype==lumptype&&
	 ld->event[i].type==etype&&
	 ld->event[i].entity==entity) j++;
   };
   return j;
};


#define ANIM_UPD(ch) if(sd->ch##anim>=0) {\
sd->ch##animstate=update_anim_state(sd->ch##anim,sd->ch##animstate,tickspassed);\
sd->ch##tex=get_anim_texture(sd->ch##anim,sd->ch##animstate);\
}

static void um_sides(const LevData *ld,int tickspassed) {
   int i;
   SideDyn *sd=ldsided(ld);   
   for(i=0;i<ldnsides(ld);i++,sd++)  {
      /* update animated textures (if any) */
      ANIM_UPD(u);
      ANIM_UPD(m);
      ANIM_UPD(l);
   };
};

static void um_sectors(const LevData *ld,int tickspassed) {
   int i;
   SectorDyn *sd=ldsectord(ld);
   for(i=0;i<ldnsectors(ld);i++,sd++)  {
      /* update animated floors & ceilings (if any) */
      ANIM_UPD(f);
      ANIM_UPD(c);
   };
};

static void um_events(LevData *ld,int tickspassed) {
   int i;
   MapEvent *me;
   for(i=0,me=ld->event;i<MAXEVENTS;i++,me++) {
      if(!EVENTVALID(*me)||me->start>ld->map_ticks) continue;
      /* has event passed its best before date? */
      if(me->stop<ld->map_ticks) {
	 logprintf(LOG_DEBUG,'M',"event %d passed its stop date",i);
	 memset(me,0,sizeof(MapEvent));
	 continue;
      };
      /* should event run now? */
      if(me->start<=ld->map_ticks) {
	 /* check wait */
	 if(me->wait!=ME_NONE&&
	    find_active_event(ld,me->ltype,me->wait,me->entity))
	    continue;
	 /* stage stuff */
	 if(me->stage<0) {
	    me->stage+=2;
#ifdef UPDMAP_DEBUG
	    logprintf(LOG_DEBUG,'M',"event %d entering stage %d entity %d",
		      i,me->stage,me->entity);
#endif
	    me->curterm=me->term[me->stage];
	    me->curdelta=me->delta[me->stage];
	    if(me->ltype==ML_SECTOR) 
	       ldsectord(ld)[me->entity].crush_effect=me->crush_effect;
	 };
	 /* run entity type specific event code */
	 switch(me->ltype) {
	 case(ML_SECTOR): do_sector_event(ld,me,tickspassed); break;
	 case(ML_SIDE): do_side_event(ld,me); break;
	 case(ML_THING): do_thing_event(ld,me); break;
	 default:
	    logprintf(LOG_ERROR,'M',"Strange lumptype %d in um_events",
		      me->ltype);
	 };
      };
   };
};

void update_map(LevData *ld,int tickspassed) {
   ld->map_ticks+=tickspassed;
   um_events(ld,tickspassed);
   um_sides(ld,tickspassed);
   um_sectors(ld,tickspassed);
};

/* need a better place to put this function */
void sector_crush_thing(LevData *ld,int sector,int thing) {
   SectorDyn *sd=ldsectord(ld)+sector;
   ThingDyn *td=ldthingd(ld)+thing;
   MapEvent *me;
   /* don't bother if they're already dead */
   if(td->hits<=0||td->proto==NULL) return;
   /* find the event that's causing it */
   me=find_active_event(ld,ML_SECTOR,ME_CEILING,sector);
   if(me==NULL) {
#ifdef UPDMAP_DEBUG
      logprintf(LOG_DEBUG,'M',"%d is being crushed, but not by sector %d ???",
		thing,sector);
#endif
      return;
   };
   /* what to do? */
   switch(sd->crush_effect) {

   /* bounce back up */
   case(MEC_REVERSE):
      unqueue_event(ld,ML_SECTOR,ME_CEILING,sector,NULL);
      /* this is a bit of a kludge */
      me=insert_event(ld,ML_SECTOR,ME_CEILING,sector,NULL);
      me->term[0]=get_term_type(ld,LowestAdjacentCeiling,sector)-(4<<12);
      me->delta[0]=1<<11;
      break;

   /* do damage, slow down crusher, and pin unfortunate beastie*/
   case(MEC_FASTHURT):
      if(me->curdelta<0) me->curdelta=-(1<<10);
      td->dangle=FIXED_PI/16;
      thing_take_damage(ld,thing,10);
      td->dx=td->dy=0;
      if(td->hits<=0) NEXT_STAGE(me);
      break;
   case(MEC_SLOWHURT):
      if(me->curdelta<0) me->curdelta=-(1<<8);
      thing_take_damage(ld,thing,2);
      td->dx=td->dy=0;
      if(td->hits<=0) NEXT_STAGE(me);
      break;
   
   /* stop */
   case(MEC_STOP):
      unqueue_event(ld,ML_SECTOR,ME_CEILING,sector,NULL);
      break;

   };
};






