#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "levdyn.h"
#include "levinfo.h"

static int ninfo=0;
static const LevInfo *info=NULL;
static LumpNum info_ln=BAD_LUMPNUM;

void init_levinfo(void) {
   if(info) return;
   info_ln=lookup_lump("LEVINFO",NULL,NULL);
   if(!LUMPNUM_OK(info_ln)) {
      logprintf(LOG_ERROR,'M',"warning: no LEVINFO lump for this game");
      ninfo=-1;
      return;
   }
   ninfo=get_lump_len(info_ln)/sizeof(LevInfo);
   info=load_lump(info_ln);
   logprintf(LOG_INFO,'M',"Init %d levels",ninfo);
}

void reset_levinfo(void) {
   if(LUMPNUM_OK(info_ln)) free_lump(info_ln);
   info_ln=BAD_LUMPNUM;
   info=NULL;
   ninfo=0;
}

const LevInfo *find_levinfo(LevData *ld) {
   int i;
   const LevInfo *li;
   if(ninfo==0) init_levinfo();
   if(ld->levinfo_id>=0) return info+ld->levinfo_id;
   if(ninfo<0) return NULL;
   for(i=0,li=info;i<ninfo;i++,li++)
      if(!strcasecmp(ld->name,li->name)) break;
   if(i>=ninfo) return NULL;
   ld->levinfo_id=i;
   return li;
}

static void load(LevData *ld,const LevInfo *li) {
   int d=ld->difficulty,m=ld->mplayer,i;
   int plhits[MAXPLAYERS],plarm[MAXPLAYERS];
   /* sanity check */
   if(li==NULL||li>info+ninfo) 
      logfatal('M',"bogus levinfo pointer in levinfo_load");
   /* save old level's player hit points */
   for(i=0;i<MAXPLAYERS;i++) {
      if(ld->player[i]>=0) {
	 plhits[i]=ldthingd(ld)[ld->player[i]].hits;
	 plarm[i]=ldthingd(ld)[ld->player[i]].armour;
      } else {
	 plhits[i]=-1;
	 plarm[i]=-1;
      }
   }
   /* load new level */
   reset_level(ld);
   load_level(ld,li->name,d,m);
   ld->levinfo_id=li-info;
   /* restore player hps */
   for(i=0;i<MAXPLAYERS;i++) {
      if(ld->player[i]>=0&&plhits[i]>=0) {
	 ldthingd(ld)[ld->player[i]].hits=plhits[i];
	 ldthingd(ld)[ld->player[i]].armour=plarm[i];
      }
   }
}

void levinfo_startgame(LevData *ld,int episode,int difficulty,int mplayer);

void levinfo_next(LevData *ld,int secret) {
   const LevInfo *li=find_levinfo(ld);
   int id;
   if(!li) logfatal('M',"no levinfo for %s, so can't find next level",
		    ld->name);
   if(secret) id=li->secret;
   else id=li->next;
   if(id>=0) load(ld,info+id);
   else load(ld,++li);
}

const char *get_skyname(LevData *ld) {
   const LevInfo *li=find_levinfo(ld);
   if(!li) return "SKY1";
   return li->sky;
}

