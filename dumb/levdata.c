#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "levdata.h"
#include "levdyn.h"
#include "dyncode.h"
#include "netplay.h"

/*#define LEVDATA_DEBUG*/

static MapLumpTypeInfo mlti[ML_NTYPES]=ML_INFO;

char *get_next_level_name(char *buf,const char *n) {
   if(n==NULL||*n==0) {
      if(LUMPNUM_OK(lookup_lump("MAP01",NULL,NULL)))
	 strcpy(buf,"MAP01");
      else
	 strcpy(buf,"E1M1");
   } else if(*n=='E'||*n=='e') {
      strcpy(buf,n);
      buf[3]++;
      if(buf[3]>'9') {
	 buf[3]='1';
	 buf[1]++;
      }
   } else {
      strcpy(buf,n);
      buf[4]++;
      if(buf[4]>'9') {
	 buf[4]='0';
	 buf[3]++;
      }
   }
   return buf;
}

void init_player(LevData *ld,int pl,int th) {
   ld->player[pl]=th;
   if(ld->plinfo[pl]==NULL) {
      logprintf(LOG_DEBUG,'M',"init_plinfo: player=%d thing=%d",pl,th);
      ld->plinfo[pl]=(int *)safe_calloc(sizeof(int),ld->plinfo_len);
      ld->plbkup[pl]=(int *)safe_calloc(sizeof(int),ld->plinfo_len);
      init_plinfo(ld->plinfo[pl]);
      memcpy(ld->plbkup[pl],ld->plinfo[pl],sizeof(int)*ld->plinfo_len);
   }
}

void load_level(LevData *ld,const char *name,int d,int mp) {
   int i;
   logprintf(LOG_INFO,'M',"Loading %s: difficulty=%d",name,d);
   strncpy(ld->name,name,8);
   ld->difficulty=d;
   ld->mplayer=mp;
   if(mp) ld->localplayer=mp-1;
   else ld->localplayer=0;
   ld->levinfo_id=-1;
   ld->plinfo_len=get_plinfo_len();
   for(i=0;i<MAXPLAYERS;i++) ld->plwep[i]=ld->player[i]=-1;
   for(i=0;i<ML_NTYPES;i++) {
      LumpNum ln;
      int count;
      /* find and load the lump */
#ifdef LEVDATA_DEBUG
      logprintf(LOG_DEBUG,'M',"Loading %s grain=%d align=%d",
		mlti[i].lumpname,mlti[i].lumpgrain,mlti[i].lumpalign);
#endif
      if(mlti[i].lumpalign&&(mlti[i].lumpgrain%mlti[i].lumpalign))
	 logfatal('M',"levdata: %s will not be aligned correctly",
		  mlti[i].lumpname); 
      ld->lumpnum[i]=ln=safe_lookup_lump(mlti[i].lumpname,name,NULL,
					 LOG_FATAL);
      ld->count[i]=count=get_lump_len(ln)/mlti[i].lumpgrain;
      ld->lump[i]=load_lump(ln);
      /* if there's dynamic data for these entries, allocate and init it */
      if(mlti[i].dyngrain>0) {
	ld->dyn[i]=safe_calloc(count+mlti[i].dynextra,mlti[i].dyngrain);
	ld->dyn_bkup[i]=safe_calloc(count+mlti[i].dynextra,mlti[i].dyngrain);
      }
      if(mlti[i].dyninit) {
	 int j;
	 char *dyn=(char *)(ld->dyn[i]);
	 const char *lump=(const char *)(ld->lump[i]);
#ifdef LEVDATA_DEBUG
	 logprintf(LOG_DEBUG,'M',"Init %d %s",count,mlti[i].lumpname);
#endif
	 for(j=0;j<count;j++) {
	    mlti[i].dyninit(dyn,lump,ld);
	    dyn+=mlti[i].dyngrain;
	    lump+=mlti[i].lumpgrain;
	 }
      }
      /* if we allocated extras, adjust the count and throw away lump */
      if(mlti[i].dynextra) {
	 ld->count[i]+=mlti[i].dynextra;
	 free_levlumptype(ld,i);
      }
      /* OK, onto next lumptype */
   }
   if(ld->player[0]<0) logprintf(LOG_ERROR,'M',"Uh-oh: no player!");
   /*logprintf(LOG_DEBUG,'M',"Done: %s",name);*/
}

void reset_level(LevData *ld) {
   int i;
   for(i=0;i<ML_NTYPES;i++) {
      free_lump(ld->lumpnum[i]);
      if(ld->dyn[i]) safe_free(ld->dyn[i]);
      if(ld->dyn_bkup[i]) safe_free(ld->dyn_bkup[i]);
   }
}
void free_level(LevData *ld) {
   int i;
#ifdef LEVDATA_DEBUG
   logprintf(LOG_INFO,'M',"Freeing level data for %s",ld->name);
#endif
   for(i=0;i<MAXPLAYERS;i++) { 
      if(ld->plinfo[i]) safe_free(ld->plinfo[i]);
      if(ld->plbkup[i]) safe_free(ld->plbkup[i]);
   }
   reset_level(ld);
}

void free_levlumptype(LevData *ld,MapLumpType mlt) {
#ifdef LEVDATA_DEBUG
   logprintf(LOG_DEBUG,'M',"Pre-freeing %s for %s",
	     mlti[mlt].lumpname,ld->name);
#endif
   free_lump(ld->lumpnum[mlt]);
   ld->lump[mlt]=NULL;
}

/* compare dyn tables with their backups and use appropriate encode
   func to send update packets */
void generate_updates(LevData *ld) {
   MapLumpType mlt;
   int pl;
   char buf[DYN_CODE_BUF_LEN];
   /* go through each lumptype */
   for(mlt=0;mlt<ML_NTYPES;mlt++) {
      int i,grain=mlti[mlt].dyngrain;
      const char *nu=(const char *)(ld->dyn[mlt]);
      char *old=(char *)(ld->dyn_bkup[mlt]);
      /* check that this lumptype is worth encoding */
      if(old==NULL||mlti[mlt].encode==NULL) continue;
      /* go through each item of this type */ 
      for(i=0;i<ld->count[mlt];i++) {
	 /* if there's been a change, encode and send it */
	 if(memcmp(nu,old,grain)) {
	    int n=mlti[mlt].encode(buf,nu,old,ld);
	    if(n>0) send_update(mlt,i,n,buf);
	 }
	 /* update pointers */
	 nu+=grain;
	 old+=grain;
      }
      /* update backup */
      memcpy(ld->dyn_bkup[mlt],ld->dyn[mlt],
	     ld->count[mlt]*mlti[mlt].dyngrain);
   }
   /* check plinfos */
   for(pl=0;pl<MAXPLAYERS;pl++) {
      int i;
      if(ld->plinfo[pl]==NULL) break;
      for(i=0;i<ld->plinfo_len;i++) {
	 if(ld->plinfo[pl][i]!=ld->plbkup[pl][i]) {
	    send_plinfo_update(pl,i,ld->plinfo[pl][i]);
	    ld->plbkup[pl][i]=ld->plinfo[pl][i];
	 }
      }
   }
   /* send sync packet */
   send_sync(0,ld->map_ticks);
}

/* lookup appropriate decode func for this update and apply it */
void apply_update(LevData *ld,MapLumpType mlt,int ofs,
		  const void *code,int codelen) {
   char *p;
   if(!got_slave_info) return;
   if(mlt<0||mlt>=ML_NTYPES)
      logfatal('M',"bad maplumptype (%d) in apply_update()",(int)mlt);
   if(mlti[mlt].decode==NULL)
      logfatal('M',"no decode func for maplumptype %d",(int)mlt);
   if(ofs<0||ofs>=ld->count[mlt])
      logfatal('M',"out of range item (%d) in apply_update()",ofs);
   p=ld->dyn[mlt];
   p+=mlti[mlt].dyngrain*ofs;
   mlti[mlt].decode(p,code,codelen,ld);
}

void apply_plinfo_update(LevData *ld,int player,int offset,int value) {
   if(!got_slave_info) return;
   if(ld->plinfo[player])
      ld->plinfo[player][offset]=value;
   else 
      logprintf(LOG_ERROR,'M',"got plinfo_update for player with no plinfo?");
}
