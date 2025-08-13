#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "lib/log.h"
#include "lib/safem.h"
#include "wad/wadio.h"
#include "prothing.h"

#ifdef BUILT_IN_XLUMPS
static const ThingPhase default_tp_tbl[];
static ProtoThing default_pt_tbl[];
static int dpt_sorted=0;
#endif /*BUILT_IN_XLUMPS*/

static const ThingPhase *phases=NULL;
static const ProtoThing *protos=NULL;
static int nphases=0,nprotos=0;
static LumpNum phase_ln,proto_ln;

static const ThingPhase **phaseidx=NULL;
static int nphaseidx=0;

/* Ugh! A triple pointer. */
static Texture ***sprites=NULL;
/*static const ThingPhase **proto_phases=NULL;*/

static int count_phases(int id)  {
   const ThingPhase *ph=find_first_thingphase(id),*end=phases+nphases;
   int i=0;
   if(ph==NULL) return 0;
   do {
      i++;
      ph++;
   } while(ph<end&&(ph->id==0||ph->id==id));
   return i;
};

static Texture **get_sprite_table(const ProtoThing *pr)  {
   const int idx=(pr-protos);
   if(sprites[idx]==NULL)
     sprites[idx]=safe_calloc(8*count_phases(pr->phase_id),sizeof(Texture *));
   return sprites[idx];
};

Texture *find_phase_sprite(const ProtoThing *proto,
			   int phase,char rot) {
   const int idx=phase*8+(rot-'1');
   Texture **sp=get_sprite_table(proto);
   if(rot<'1'||rot>'8')
     logprintf(LOG_FATAL,'O',"wanted rot %c for sprite %s phase %d???",
	       rot,proto->sprite,phase);
   if(sp[idx]==NULL) {
      const ThingPhase *tp=find_thingphase(proto->phase_id,phase);
      sp[idx]=get_sprite((tp->sprite[0]==0)?proto->sprite:tp->sprite,
			 tp->spr_phase,
			 rot,NULL);
      if(sp[idx]==NULL) 
      	logprintf(LOG_ERROR,'O',"Can't find sprite %s, ph=%d (%c) rot=%c",
		  proto->sprite,phase,tp->spr_phase,rot);
   };
   return sp[idx];
};

const ThingPhase *find_first_thingphase(int id)  {
   int start,stop,t;
   if(phases==NULL) init_prothings();
   start=0;
   stop=nphaseidx-1;
   do {
      t=(start+stop)/2;
      if(phaseidx[t]->id==id) break;
      if(start>=stop) 
      	logprintf(LOG_FATAL,'O',"Can't find ThingPhase id=%d",id);
      if(phaseidx[t]->id<id) start=t+1;
      else stop=t-1;
   } while(1);
   return phaseidx[t];
};

const ProtoThing *find_protothing(int id)  {
   int start,stop,t;
   if(protos==NULL) init_prothings();
   start=0;
   stop=nprotos-1;
   do {
      t=(start+stop)/2;
      if(protos[t].id==id) break;
      if(start>=stop) return NULL;
      if(protos[t].id<id) start=t+1;
      else stop=t-1;
   } while(1);
   return protos+t;
};

void reset_prothings(void) {
#ifdef BUILT_IN_XLUMPS
   if(phases&&phases!=default_tp_tbl) free_lump(phase_ln);
   if(protos&&protos!=default_pt_tbl) free_lump(proto_ln);
#else
   if(phases) free_lump(phase_ln);
   if(protos) free_lump(proto_ln);
#endif
   //if(proto2phase) safe_free(proto2phase);
   if(phaseidx) safe_free(phaseidx);
   if(sprites) {
      int i;
      for(i=0;i<nprotos;i++) if(sprites[i]) safe_free(sprites[i]);
      safe_free(sprites);
   };
   sprites=NULL;
   //proto2phase=NULL;
   phases=NULL;
   protos=NULL;
   phaseidx=NULL;
   nphaseidx=nphases=nprotos=0;
};

#ifdef BUILT_IN_XLUMPS
static int ptcmp(const void *p1,const void *p2) {
   if(((const ProtoThing *)p1)->id>((const ProtoThing *)p2)->id) return 1;
   if(((const ProtoThing *)p1)->id<((const ProtoThing *)p2)->id) return -1;
   return 0;
};
#endif /*BUILT_IN_XLUMPS*/

void init_prothings(void)  {
   int i;
   if(protos) reset_prothings();
#ifdef BUILT_IN_XLUMPS
   phase_ln=lookup_lump("PHASES",NULL,NULL);
   proto_ln=lookup_lump("PROTOS",NULL,NULL);
#else
   phase_ln=safe_lookup_lump("PHASES",NULL,NULL,LOG_FATAL);
   proto_ln=safe_lookup_lump("PROTOS",NULL,NULL,LOG_FATAL);
#endif /*BUILT_IN_XLUMPS*/
   /* load phases from wad */
   if(LUMPNUM_OK(phase_ln)) {
      nphases=get_lump_len(phase_ln)/sizeof(ThingPhase);
      phases=load_lump(phase_ln);
   }
#ifdef BUILT_IN_XLUMPS
   /* load phases from internal table */
   else {
      phases=default_tp_tbl;
      nphases=0;
      while(phases[nphases].id!=-1) nphases++;
   };
#endif /*BUILT_IN_XLUMPS*/
   /* build phase index */
   phaseidx=safe_malloc(nphases*sizeof(ThingPhase *));
   for(nphaseidx=i=0;i<nphases;i++) 
      if(phases[i].id>0) 
	 phaseidx[nphaseidx++]=phases+i;
   phaseidx=safe_realloc(phaseidx,nphaseidx*sizeof(ThingPhase *));
   /* load protos from wad */
   if(LUMPNUM_OK(proto_ln)) {
      nprotos=get_lump_len(proto_ln)/sizeof(ProtoThing);
      protos=load_lump(proto_ln);
   }
#ifdef BUILT_IN_XLUMPS
   /* load protos from internal table */
   else {
      protos=default_pt_tbl;
      nprotos=0;
      while(protos[nprotos].id!=-1) nprotos++;
      /* sort the prothings, so we can do binary searches on them */
      if(!dpt_sorted)  {
	 dpt_sorted=1;
	 qsort(default_pt_tbl,nprotos,sizeof(ProtoThing),ptcmp);
      };
   };
#endif /*BUILT_IN_XLUMPS*/
   sprites=safe_calloc(nprotos,sizeof(Texture **));
   /*proto2phase=safe_calloc(nprotos,sizeof(ThingPhase *));
   for(i=0;i<nprotos;i++) 
     proto2phase[i]=find_first_thingphase(protos[i].phase_id);*/
};

#ifdef BUILT_IN_XLUMPS
#include "pt_data.c"
#endif /*BUILT_IN_XLUMPS*/

