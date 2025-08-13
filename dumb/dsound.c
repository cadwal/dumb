#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "wad/wadstruct.h"
#include "wad/wadio.h"
#include "lib/log.h"
#include "lib/safem.h"
#include "plat/sound.h"
#include "netplay.h"
#include "dsound.h"


typedef struct {
   LumpNum ln;
   const SoundHdr *sh;
} SoundDyn;

static LumpNum se_ln=BAD_LUMPNUM;
static int nses=0;
static const SoundEnt *se=NULL;
static SoundDyn *sd=NULL;

static View view;

void init_dsound(void) {
   if(se) reset_dsound();
   se_ln=lookup_lump("SOUNDS",NULL,NULL);
   if(!LUMPNUM_OK(se_ln)) {
      logprintf(LOG_ERROR,'S',"couldn't find SOUNDS lump");
      return;
   };
   se=(const SoundEnt *)load_lump(se_ln);
   nses=get_lump_len(se_ln)/sizeof(SoundEnt);
   logprintf(LOG_INFO,'S',"Loaded %d sounds",nses);
   sd=(SoundDyn *)safe_calloc(nses,sizeof(SoundDyn));
};

void reset_dsound(void) {
   int i;
   for(i=0;i<nses;i++)
      if(sd[i].sh) free_lump(sd[i].ln);
   safe_free(sd);
   sd=NULL;
   se=NULL;
   nses=0;
   se_ln=BAD_LUMPNUM;
};

void dsound_setview(View *v) {
   memcpy(&view,v,sizeof(View));
};

static int rnd(int n) {
   return (int)(((float)n)*rand()/(RAND_MAX+1.0));
};

static void get_bend(SoundBalance *bal,const SoundEnt *se) {
   if(se->bend_range&&se->bend_range<16) {
      do bal->bend=rnd(32)-16;
      while(abs(bal->bend)<se->bend_range);
      /*logprintf(LOG_DEBUG,'S',"bend: %d",bal.bend);*/
   };
   if(se->bend_const)
      bal->bend+=se->bend_const;
};

void play_dsound(int i,fixed x,fixed y,fixed r) {
   play_dsound_local(i,x,y,r);
   send_dsound(i,x,y,r);
};

void play_dsound_local(int i,fixed x,fixed y,fixed r) {
   SoundDyn *s=sd+i;
   const SoundEnt *ent=se+i;
   SoundBalance bal=FLAT_BALANCE;
   if(!se) return;
   if(i<0||i>=nses) logfatal('S',"play_dsound got bad sound id (%d)",i);
   /* deal with chance */
   if(ent->chance) {
      int n=rnd(abs(ent->chance));
      if(ent->chance<0&&n) return;
      if(ent->chance>0&&!n) return;
   };
   /* deal with redirection */
   if(ent->nredir>0) {
      int n=rnd(ent->nredir);
      /*logprintf(LOG_DEBUG,'S',"play_dsound: random %d/%d",
		n,(int)(ent->nredir));*/
      get_bend(&bal,ent);
      i=ent->redir[n];
   };
   /* special case: if r==0, sound is directionless */
   if(r!=0) {
      fixed dist;
      dist=fix_pythagoras(view.x-x,view.y-y);
      if(dist<r) {
	 /* we're very close to what's making the noise: it's all around us */
	 /*bal.lbvol=bal.lfvol=bal.rbvol=bal.rfvol=(12*FIXED_ONE)/10;*/
      }
      else {
	 /* pan the noise off to one side or the other */
	 fixed angle=fix_vec2angle(x-view.x,y-view.y)-view.angle;
	 fixed sine=fixsin(angle);
	 fixed fade=fixdiv(20*FIXED_ONE,FIXED_ONE+dist-r);
	 if(fade>FIXED_ONE) fade=FIXED_ONE;
	 if(fade<FIXED_ONE/20) return;
	 /*logprintf(LOG_DEBUG,'S',
		   "pan: angle=%f sine=%f dist=%f r=%f fade=%f",
		   FIXED_TO_FLOAT(angle),FIXED_TO_FLOAT(sine),
		   FIXED_TO_FLOAT(dist),
		   FIXED_TO_FLOAT(r),FIXED_TO_FLOAT(fade));*/
	 bal.lbvol=bal.lfvol=fixmul(fade,sine+FIXED_ONE);
	 bal.rbvol=bal.rfvol=fixmul(fade,FIXED_ONE-sine);
      };
   };
   /* check that sound is loaded */
   if(s->sh==NULL) {
      s->ln=getlump(se[i].lumpname);
      s->sh=(const SoundHdr*)load_lump(s->ln);
   };
   /* work out bend */
   get_bend(&bal,se+i);
   /* play it */
   play_sound(s->sh->data,s->sh->len,bal,s->sh->hz);
};






