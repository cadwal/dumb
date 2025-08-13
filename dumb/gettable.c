#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "libdumb/texture.h"
#include "libdumb/dsound.h"
#include "banner.h"
#include "draw.h"
#include "game.h"
#include "gettable.h"
#include "levdata.h"
#include "things.h"

static LumpNum ln=BAD_LUMPNUM;
static int ngetts=0;
static const Gettable *gett=NULL;
static Texture ***gettxt=NULL;

#define NUMICONS(gk) (gett[gk].iconanim? \
		      (1+gett[gk].iconanim-gett[gk].iconname[4]): \
		      1)
#define ANIMWAIT(gk) (gett[gk].timing+1)
#define GTEXTURE(gk) get_gktex(ld,gk)

#define PLEXTRA 2
#define PLINFOK(pl) (ld->plinfo[pl]!=NULL)
#define PLCOUNT(pl) (ld->plinfo[pl]+PLEXTRA)
#define PLSEL(pl) (ld->plinfo[pl])

static int *getcount(LevData *ld,int pl,int type) {
   switch((int)(gett[type].special)) {
   case(1): return &(ldthingd(ld)[ld->player[pl]].hits);
   case(2): return &(ldthingd(ld)[ld->player[pl]].armour);
   case(3): return &(ldthingd(ld)[ld->player[pl]].tmpinv);
   case(4): return &(ldthingd(ld)[ld->player[pl]].tmpgod);
   }
   return PLCOUNT(pl)+type;   
}
static Texture *get_gktex(LevData *ld,int gk) {
   int i,ni;
   if(!gettxt[gk]) return NULL;
   ni=NUMICONS(gk);
   i=ld->map_ticks/ANIMWAIT(gk);
   if(gett[gk].flags&GK_REVANIM) {
      i%=2*(ni-1);
      if(i<ni) return gettxt[gk][i];
      i-=ni;
      return gettxt[gk][(ni-2)-i];
   } else 
      return gettxt[gk][i%ni];
}

int get_plinfo_len(void) {
   if(ngetts==0) init_gettables();
   return ngetts+PLEXTRA;
}
void init_plinfo(int *pli) {
   int i;
   const Gettable *gt=gett;
   if(ngetts==0) init_gettables();
   for(i=0;i<PLEXTRA;i++)
      pli[i]=-1;
   for(i=0;i<ngetts;i++,gt++)
      pli[i+PLEXTRA]=gt->initial;
}

static void set_bogot(LevData *ld,int pl,const Gettable *gt) {
   ThingDyn *b;
   if(ld->plwep[pl]<0)
      ld->plwep[pl]=new_thing(ld,gt->bogotype,0,0,0);
   b=ldthingd(ld)+ld->plwep[pl];
   b->owner=ld->player[pl];
   if(b->proto==NULL||b->proto->id!=gt->bogotype) {
      b->proto=find_protothing(gt->bogotype);
      if(b->proto) {
	 b->phase_tbl=find_first_thingphase(b->proto->phase_id);
	 b->hits=b->proto->hits;
	 b->phase=b->proto->signals[TS_INIT];
	 b->phase_wait=b->phase_tbl[b->phase].wait;
      }
   }
}

void rotate_selection(LevData *ld,int pl,int type) {
   int i=PLSEL(pl)[type];
   int loops=-1;
   while(1) {
      i++;
      /* check bailout */
      if(loops++>ngetts) {
	 i=-1;
	 break;
      }
      /* wrap around if necessary */
      if(i>=ngetts||i<0) i=0;
      /* check selectable */
      if(type==0&&!(gett[i].flags&GK_WEPSELECT)) continue;
      if(type==1&&!(gett[i].flags&GK_SPESELECT)) continue;
      /* check we have one */
      if(PLCOUNT(pl)[i]<1) continue;
      if(gett[i].bulletkind>=0&&PLCOUNT(pl)[gett[i].bulletkind]<1) continue;
      /* a strange loop, but it works */
      break;
   }
   PLSEL(pl)[type]=i;
   set_bogot(ld,pl,gett+PLSEL(pl)[type]);
}

void use_selection(int type,LevData *ld,int pl) {
   const Gettable *gt=gett+PLSEL(pl)[type];
   int *cd=PLCOUNT(pl)+PLSEL(pl)[type];
   int *cb=PLCOUNT(pl)+PLSEL(pl)[type];
   if(PLSEL(pl)[type]<0) {
      rotate_selection(ld,pl,type);
      return;
   }
   if(gt->bulletkind>=0) cb=PLCOUNT(pl)+gt->bulletkind;
   if(cd[0]<1||cb[0]<gt->usenum) {
      rotate_selection(ld,pl,type);
      return;
   }
   set_bogot(ld,pl,gett+PLSEL(pl)[type]);
   cb[0]-=gt->usenum*use_item(ld,pl,gt);
   if(cb[0]<0) cb[0]=0;
   if(gt->sound>=0) play_dsound(gt->sound,0,0,0);
}

/* pickup */

void pickup_gettable(LevData *ld,int pl,int type,int num) {
   const Gettable *gt=gett+type;
   if(type<0||type>=ngetts) 
      logprintf(LOG_ERROR,'G',"Strange gettable type=%d num=%d",type,num);
   else {
      int *cnt=getcount(ld,pl,type);
      logprintf(LOG_DEBUG,'G',"pickup_gettable: pl=%d type=%d num=%d",
		pl,type,num);
      /* sound */
      if(gt->pickup_sound>=0)
	 play_dsound(gt->pickup_sound,0,0,0);
      /* send messages */
      if(*cnt==0&&(gt->flags&GK_GOT_THE))
	 game_message(pl,"YOU GOT THE %s!",gt->string);
      else if(*cnt>=gt->collect&&(gt->flags&GK_GOT_A)) {
	 game_message(pl,"THAT %s WOULD BE WASTED...",gt->string);
	 return;
      } else if(gt->flags&GK_GOT_A) 
	 game_message(pl,"GOT A %s.",gt->string);
      /* do the pickup */
      *cnt+=num;
      if(*cnt>gt->collect)
	 *cnt=gt->collect;
      if(gt->bulletkind>=0&&gt->bulletadd>0)
	 pickup_gettable(ld,pl,gt->bulletkind,gt->bulletadd);
   }
}


/* draw funcs */

static count_font=-1;

static void init_cfont(void) {
   if(count_font<0) {
      if(have_lump("IN0"))
	 count_font=init_font("SMALLIN%d",10,0);
      else 
	 count_font=init_font("STYSNUM%d",10,0);
   }
}

static void draw_count(void *fb,int c,int x,int y) {
   unsigned char buf[3]; 
   init_cfont();
   buf[0]=(c/100)%10;
   buf[1]=(c/10)%10;
   buf[2]=c%10;
   drawtext(fb,buf,3,count_font,x,y);
}

void draw_gettables(LevData *ld,int pl,
		    void *fb,int width,int height) {
   int i;
   const Gettable *gt=gett;
   for(i=0;i<ngetts;i++,gt++) {
      int xo=gt->xo,yo=gt->yo;
      int cnt=*getcount(ld,pl,i);
      Texture *gtx=GTEXTURE(i);
      if(gtx==NULL||cnt<1) continue;
      if(xo<0) xo+=width-gtx->width;
      if(yo<0) yo+=height-gtx->height-(gt->collect>1?16:0);
      /* center icon */
      if(gt->flags&GK_XCENTERICON) xo-=gtx->width/2;
      if(gt->flags&GK_YCENTERICON) yo-=gtx->height/2;
      /* now draw icon */
      if((gt->flags&GK_WEPSELECT)&&i==PLSEL(pl)[0])
	 draw_outline(fb,gtx,xo,yo);
      else if((gt->flags&GK_SPESELECT)&&i==PLSEL(pl)[1])
	 draw_outline(fb,gtx,xo,yo);
      else
	 draw(fb,gtx,xo,yo);
      if(gt->collect>1&&!gt->decay) 
	draw_count(fb,cnt,xo,yo+gtx->height+3);
   }
}

/* do decay stuff */
void update_gettables(LevData *ld,int ticks) {
   int pl;
   for(pl=0;pl<MAXPLAYERS;pl++) {
      int i;
      const Gettable *gt=gett;
      if(!PLINFOK(pl)) continue;
      for(i=0;i<ngetts;i++,gt++) {
	 int *count=getcount(ld,pl,i);
	 if(gt->decay&&(*count>0))
	    *count-=ticks*gt->decay;
	 if(*count<0) *count=0;
      }
   }
}

/* cheat */
void cheat_gettables(LevData *ld,int pl) {
   int i;
   const Gettable *gt=gett;
   int *count=PLCOUNT(pl);
   for(i=0;i<ngetts;i++,gt++,count++)
      *count=gt->collect;
}

/* reset locals */
void reset_local_gettables(LevData *ld) {
   int pl;
   for(pl=0;pl<MAXPLAYERS;pl++) {
      int i;
      const Gettable *gt=gett;
      int *count=PLCOUNT(pl);
      if(!PLINFOK(pl)) continue;
      for(i=0;i<ngetts;i++,gt++,count++) 
	 if(gt->flags&GK_LOCAL) *count=gt->initial;
   }
}

/* return non-zero if we have a key of type <keytype> */
int gettable_chk_key(const LevData *ld,int pl,int keytype) {
   int i;
   const Gettable *gt=gett;
   int *count=PLCOUNT(pl);
   for(i=0;i<ngetts;i++,gt++,count++)
      if(*count&&gt->key==keytype) return 1;
   return 0;
}


/* init and reset funcs */

void init_gettables(void) {
   int i;
   /* if already inited, throw away old info first */
   if(ngetts>0) reset_gettables();
   /* load lump and allocate GetDs */
   count_font=-1;
   ln=lookup_lump("GETTABLE",NULL,NULL);
   if(!LUMPNUM_OK(ln)) return;
   ngetts=get_lump_len(ln)/sizeof(Gettable);
   logprintf(LOG_INFO,'G',"init %d gettables",ngetts);
   gett=(Gettable *)load_lump(ln);
   gettxt=(Texture ***)safe_calloc(sizeof(Texture **),ngetts);
   /* initialise */
   for(i=0;i<ngetts;i++) {
      int j;
      char buf[10];
      if(gett[i].iconname[0]&&have_lump(gett[i].iconname)) {
	 gettxt[i]=(Texture **)safe_calloc(sizeof(Texture *),NUMICONS(i));
	 strcpy(buf,gett[i].iconname);
	 for(j=NUMICONS(i)-1;j>=0;j--) {
	    buf[4]=gett[i].iconname[4]+j;
	    gettxt[i][j]=get_misc_texture(buf);
	 }
      }
   }
}

void reset_gettables(void) {
   if(LUMPNUM_OK(ln)) free_lump(ln);
   if(gettxt) {
      int i;
      for(i=0;i<ngetts;i++)
	 if(gettxt[i]) safe_free(gettxt[i]);
      safe_free(gettxt);
   }
   ngetts=0;
   gett=NULL;
   gettxt=NULL;
}

