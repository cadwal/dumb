#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "animtex.h"

static LumpNum animtex_ln=BAD_LUMPNUM;
static int natt=0;
static const AnimTexTable *att=NULL;
static Texture **attd=NULL;

AnimTexNum get_animtex(const char *name) {
   int i;
   if(!att) init_animtex();
   for(i=0;i<natt;i++)  {
      if(!strncasecmp(name,att[i].name,8)) {
	 /* if my seqnum is bogus, return the next entry */
	 if(att[i].myseqnum>=att[i].seqlen) return i+1;
	 /* otherwise, I'll do */
	 else return i;
      }
      /* bogus seqnums indicate this is an alias for the next set
       * of anims, whose individual names should be ignored
       */
      if(att[i].myseqnum>=att[i].seqlen) i+=att[i].seqlen;
   }
   return -1;
}

int update_anim_state(AnimTexNum num,int snum,int tickspassed) {
   int tnum=snum&0xffff;
   if(!attd) init_animtex();
   if(num>=natt||num<0)
     logfatal('A',"Bad AnimTexNum (%d)",num);
   num-=att[num].myseqnum;
   if(att[num].flags&AT_SWITCH) return snum;
   snum>>=16;
   tnum+=tickspassed;
   if(tnum>=att[num+snum].duration) {tnum=0;snum++;}
   if(snum>=att[num].seqlen) snum=0;
   return (snum<<16)|(tnum&0xffff);
}

Texture *get_anim_texture(AnimTexNum num,int snum) {
   if(!attd) init_animtex();
   if(num>=natt||num<0)
     logfatal('A',"Bad AnimTexNum (%d)",num);
   num-=att[num].myseqnum;
   if(!(att[num].flags&AT_SWITCH)) snum>>=16;
   num+=snum;
   if(attd[num]==NULL)  {
      if(att[num].flags&AT_FLAT) attd[num]=get_flat_texture(att[num].name);
      else attd[num]=get_wall_texture(att[num].name);
   }
   return attd[num];
}

void init_animtex(void) {
   if(att) reset_animtex();
   animtex_ln=lookup_lump("ANIMTEX",NULL,NULL);
   if(LUMPNUM_OK(animtex_ln))  {
      natt=get_lump_len(animtex_ln)/sizeof(AnimTexTable);
      att=(const AnimTexTable *)load_lump(animtex_ln);
      attd=(Texture **)safe_calloc(sizeof(Texture *),natt);
   } else {
      att=NULL;
      natt=0;
   }
}

void reset_animtex(void) {
   if(LUMPNUM_OK(animtex_ln)) free_lump(animtex_ln);
   if(attd) safe_free(attd);
   att=NULL;
   attd=NULL;
   natt=0;
   animtex_ln=BAD_LUMPNUM;
}

