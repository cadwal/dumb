#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "libdumbutil/timer.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumb/dsound.h"
#include "libdumb/animtexstruct.h"
#include "libdumb/gettablestruct.h"
#include "libdumb/levinfostruct.h"
#include "libdumb/linetypestruct.h"
#include "libdumb/prothingstruct.h"
#include "libdumb/levdatanums.h"

#define NAMELEN 32
#define ALLOC_BLK 64

FILE *fin;

int line=0,tok_ungot=0;
char tokbuf[256]="";
char cppfile[256]="stdin";

typedef struct {
   ProtoThing pt;
   char name[NAMELEN];
} ProtoThingRec;

typedef struct {
   Gettable g;
   char name[NAMELEN];
} GettableRec;

typedef struct {
   LineType l;
   char name[NAMELEN];
} LineTypeRec;

typedef struct {
   SoundEnt s;
   char name[NAMELEN];
} SoundRec;

typedef struct {
   AnimTexTable *tbl;
   int ntbl,maxtbl;
   char name[NAMELEN];
} AnimRec;

typedef struct {
   ThingPhase *tp;
   char *tpname;
   char name[NAMELEN];
   int nphases,maxphases;
   LE_int16 signals[NUM_THINGSIGS];
} ThingPhaseRec;

typedef struct {
   LevInfo l;
   char next[10];
   char secret[10];
} LevInfoRec;

int default_time_units=1;
fixed default_speed=1<<11;

int nprotos=0,nphasetbls=0,ngetts=0,nlts=0,nsts=0;
int nsounds=0,nanims=0,nlinfos=0;
int maxprotos=0,maxphasetbls=0,maxgetts=0,maxlts=0,maxsts=0;
int maxsounds=0,maxanims=0,maxlinfos=0;
ProtoThingRec *protos;
ThingPhaseRec *phases;
GettableRec *getts;
LineTypeRec *lts,*sts;
SoundRec *sounds;
AnimRec *anims;
LevInfoRec *linfos;

int top_assigned_proto_id=16385;

void chkuniqueid(int id) {
   int i;
   for(i=nprotos-2;i>=0;i--)
      if(protos[i].pt.id==id) {
	 printf("ptcomp: line %d (%s): ID %d not unique (clashes with %s)\n",
		line,cppfile,id,protos[i].name);
	 exit(2);
      }
}

#define unget_token() tok_ungot++;

const char *next_token(void) {
   char *s=tokbuf;
   int i;
   if(line==0) line=1;
   if(tok_ungot) {
      tok_ungot=0;
      return tokbuf;
   }
   memset(tokbuf,0,256);
   while(1) {
      if(feof(fin)) return NULL;
      i=getc(fin);
      *s = (char)i;
      if(*s!=' '&&*s!='\t'&&*s!='\r'&&i!=-1) break;
   }
   if(*s=='#') {
      while((*s++=getc(fin))!='\n') if(feof(fin)) return NULL;
      line=atoi(tokbuf+1);
      s=strchr(tokbuf,'"');
      if(s) {
	 char *t=cppfile;
	 s++;
	 while(*s!='"') *t++=*s++;
	 *t=0;
	 if(*cppfile==0) strcpy(cppfile,"stdin");
      }
      /*printf("line=%d file=(%s)\n",line,cppfile);*/
      s=tokbuf;
      *s=0;
      return "\n";
   }
   if(*s=='\n') {
      line++;
      *s=0;
      return "\n";
   }
   do {
      if(feof(fin)) return tokbuf;
      s++;
      i=getc(fin);
      *s=(char)i;
   } while(*s!=' '&&*s!='\t'&&*s!='\r'&&i!=-1&&*s!='\n'&&*s!='#');
   if(*s=='\n'||*s=='#') ungetc(*s,fin);
   *s=0;
   return tokbuf;
}

void synerr(const char *detail) {
   printf("ptcomp: syntax error near line %d (%s), token '%s'\n",
	  line,cppfile,tokbuf);
   if(detail) printf("        %s\n",detail);
   exit(3);
}
void err(const char *detail) {
   printf("ptcomp: fatal error near line %d (%s)\n",line,cppfile);
   if(detail) printf("        %s\n",detail);
   exit(3);
}

ThingPhaseRec *find_ph_tbl(const char *s) {
   int i;
   for(i=0;i<nphasetbls;i++) 
      if(!strcasecmp(phases[i].name,s))
	 return phases+i;
   return NULL;
}
ThingPhaseRec *parm_ph_tbl(void) {
   const char *s=next_token();
   ThingPhaseRec *tpr;
   if(s==NULL||*s=='\n') synerr("phase table name parameter expected");
   tpr=find_ph_tbl(s);
   if(!tpr) synerr("phase table name unrecognised");
   return tpr;
}
int parm_phase(ThingPhaseRec *p) {
   int i;
   const char *s=next_token();
   if(p==NULL) synerr("you need to specify a phasetable first");
   if(s==NULL||*s=='\n') synerr("phase name parameter expected");
   for(i=0;i<p->nphases;i++) 
      if(!strcasecmp(p->tpname+i*NAMELEN,s))
	 return i;
   synerr("phase name unrecognised");
   return -1;
}
int parm_proto(void) {
   int i;
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("proto name parameter expected");
   if(isdigit(*s)) return atoi(s);
   for(i=0;i<nprotos;i++) 
      if(!strcasecmp(protos[i].name,s))
	 return protos[i].pt.id;
   synerr("proto name unrecognised");
   return -1;
}
int parm_gett(void) {
   int i;
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("gettable name parameter expected");
   if(isdigit(*s)) return atoi(s);
   for(i=0;i<ngetts;i++) 
      if(!strcasecmp(getts[i].name,s))
	 return i;
   synerr("gettable name unrecognised");
   return -1;
}

static int new_sound(const char *name) {
   int i;
   for(i=0;i<nsounds;i++) 
      if(!strcasecmp(sounds[i].name,name))
	 err("soundtype redefined");
   nsounds++;
   if(nsounds>=maxsounds) {
      maxsounds+=ALLOC_BLK;
      sounds=(SoundRec *)safe_realloc(sounds,maxsounds*sizeof(SoundRec));
   }
   strcpy(sounds[i].name,name);
   strcpy(sounds[i].s.lumpname,name);
   return i;
}

int parm_sound(void) {
   int i;
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("sound name parameter expected");
   if(isdigit(*s)) return atoi(s);
   for(i=0;i<nsounds;i++) 
      if(!strcasecmp(sounds[i].name,s))
	 return i;
   /* sound name unrecognised */
   return new_sound(s);
}

void parm_str(char *buf,int n) {
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("string parameter expected");
   strncpy(buf,s,n-1);
   buf[n-1]=0;
}
char parm_ch(void) {
   const char *s=next_token();
   if(s==NULL||*s=='\n'||s[1]) synerr("character parameter expected");
   return *s;
}
int parm_num(void) {
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("integer parameter expected");
   return atoi(s);
}

double
parm_dbl(void)
{
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("floating-point parameter expected");
   return atof(s);
}

int time_units(const char *s,int n) {
   while(*s&&isdigit(*s)) s++;
   if(!*s) return n*default_time_units;
   if(!strcasecmp(s,"sec")) return n*1000/MSEC_PER_TICK;
   if(!strcasecmp(s,"msec")) return n/MSEC_PER_TICK;
   if(!strcasecmp(s,"hsec")) return n*10/MSEC_PER_TICK;
   if(!strcasecmp(s,"ticks")) return n;
   synerr("strange timing unit");
   return n;
}

int parm_time(void) {
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("time parameter expected");
   return time_units(s,atoi(s));
}

int speed_units(const char *s,int n) {
   while(*s&&isdigit(*s)) s++;
   if(!*s) return n;
   /* we must be talking pixels */
   n<<=12;
   if(!strcasecmp(s,"/sec")) return n*MSEC_PER_TICK/1000;
   if(!strcasecmp(s,"/msec")) return n*MSEC_PER_TICK;
   if(!strcasecmp(s,"/hsec")) return n*MSEC_PER_TICK/10;
   if(!strcasecmp(s,"/tick")) return n;
   synerr("strange timing unit");
   return n;
}

int parm_speed(void) {
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("speed parameter expected");
   return speed_units(s,atoi(s));
}

fixed arc_units(const char *s,int n) {
   while(*s&&isdigit(*s)) s++;
   if(!*s) {
      /* backwards compatibility */
      if(n==0) return 0;
      return FIXED_PI/n; 
   }
   if(!strcasecmp(s,"deg")) return (n*FIXED_PI)/180;
   if(!strcasecmp(s,"pi")) {
      if(n==0) n=1;
      return FIXED_PI*n;
   }
   if(!strncasecmp(s,"pi/",3)) {
      int m;
      if(n==0) n=1;
      m=atoi(s+3);
      if(m) return (FIXED_PI*n)/m;
   }
   synerr("strange arc unit");
   return n;
}

fixed parm_arc(void) {
   const char *s=next_token();
   if(s==NULL||*s=='\n') synerr("arc parameter expected");
   return arc_units(s,atoi(s));
}

LT_TermType parm_termtype(void) {
   const char *s=next_token();
   if(s==NULL||*s=='\n');
   else if(!strcasecmp(s,"Floor")) return Floor;
   else if(!strcasecmp(s,"LowestFloor")) return LowestFloor;
   else if(!strcasecmp(s,"LowestAdjacentFloor")) return LowestAdjacentFloor;
   else if(!strcasecmp(s,"HighestFloor")) return HighestFloor;
   else if(!strcasecmp(s,"HighestAdjacentFloor")) return HighestAdjacentFloor;
   else if(!strcasecmp(s,"NextHighestFloor")) return NextHighestFloor;
   else if(!strcasecmp(s,"NextLowestFloor")) return NextLowestFloor;
   else if(!strcasecmp(s,"Ceiling")) return Ceiling;
   else if(!strcasecmp(s,"LowestCeiling")) return LowestCeiling;
   else if(!strcasecmp(s,"LowestAdjacentCeiling")) 
      return LowestAdjacentCeiling;
   else if(!strcasecmp(s,"HighestCeiling")) return HighestCeiling;
   else if(!strcasecmp(s,"HighestAdjacentCeiling")) 
      return HighestAdjacentCeiling;
   else if(!strcasecmp(s,"NextHighestCeiling")) return NextHighestCeiling;
   else if(!strcasecmp(s,"NextLowestCeiling")) return NextLowestCeiling;
   else if(!strcasecmp(s,"FloorPlusSLT")) return FloorPlusSLT;
   else if(!strcasecmp(s,"FloorMinusSLT")) return FloorMinusSLT;
   else if(!strcasecmp(s,"OrigFloor")) return OrigFloor;
   else if(!strcasecmp(s,"OrigCeiling")) return OrigCeiling;
   else if(!strcasecmp(s,"One")) return TTOne;
   else if(!strcasecmp(s,"Zero")) return TTZero;
   else if(!strcasecmp(s,"DarkestSector")) return DarkestSector;
   else if(!strcasecmp(s,"DarkestAdjacentSector")) 
      return DarkestAdjacentSector;
   else if(!strcasecmp(s,"LightestSector")) return LightestSector;
   else if(!strcasecmp(s,"LightestAdjacentSector")) 
      return LightestAdjacentSector;
   /* dynamic lightlevels are actually darklevels */
   else if(!strcasecmp(s,"VeryDark")) return TTOne;
   else if(!strcasecmp(s,"VeryLight")) return TTZero;
   /* abbreviations */
   else if(!strcasecmp(s,"LIF")) return LowestFloor;
   else if(!strcasecmp(s,"LEF")) return LowestAdjacentFloor;
   else if(!strcasecmp(s,"HIF")) return HighestFloor;
   else if(!strcasecmp(s,"HEF")) return HighestAdjacentFloor;
   else if(!strcasecmp(s,"LIC")) return LowestCeiling;
   else if(!strcasecmp(s,"LEC")) return LowestAdjacentCeiling;
   else if(!strcasecmp(s,"HIC")) return HighestCeiling;
   else if(!strcasecmp(s,"HEC")) return HighestAdjacentCeiling;
   else if(!strcasecmp(s,"nhEF")) return NextHighestFloor;
   else if(!strcasecmp(s,"nlEF")) return NextLowestFloor;
   else if(!strcasecmp(s,"nhEC")) return NextHighestCeiling;
   else if(!strcasecmp(s,"nlEC")) return NextLowestCeiling;
   /* none of the above */
   else synerr("termtype parameter expected");
   return 0;
}

void parm_msg(char *buf,int n) {
   char *s;
   parm_str(buf,n);
   for(s=buf;*s;s++) if(*s=='_') *s=' ';
}

int atolumptype(const char *s) {
   if(!strcasecmp(s,"Sector")) return ML_SECTOR;
   else if(!strcasecmp(s,"Side")) return ML_SIDE;
   else if(!strcasecmp(s,"Thing")) return ML_THING;
   else synerr("strange lumptype");
   return 0;
}
int atoeventtype(const char *s) {
   if(!strcasecmp(s,"Ceiling")) return ME_CEILING;
   else if(!strcasecmp(s,"CeilingTexture")) return ME_CEILING_TEX;
   else if(!strcasecmp(s,"CeilingType")) return ME_CEILING_TYPE;
   else if(!strcasecmp(s,"Floor")) return ME_FLOOR;
   else if(!strcasecmp(s,"FloorTexture")) return ME_FLOOR_TEX;
   else if(!strcasecmp(s,"FloorType")) return ME_FLOOR_TYPE;
   else if(!strcasecmp(s,"Darkness")) return ME_LIGHT;
   else if(!strcasecmp(s,"Light")) return ME_LIGHT;
   else if(!strcasecmp(s,"SwitchOn")) return ME_SWITCHON;
   else if(!strcasecmp(s,"SwitchOff")) return ME_SWITCHOFF;
   else if(!strcasecmp(s,"Teleport")) return ME_TELEPORT;
   else if(!strcasecmp(s,"SecretLevel")) return ME_SECRETLEVEL;
   else if(!strcasecmp(s,"NewLevel")) return ME_NEWLEVEL;
   else synerr("strange eventtype");
   return 0;
}

void secomp(void) {
   const char *s;
   SoundRec *sr;
   /* one parameter: name */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after SoundEnt");
   sr=sounds+new_sound(s);
   /* start filling in data */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Bend")) sr->s.bend_range=parm_num();
      else if(!strcasecmp(s,"BendConst")) sr->s.bend_const=parm_num();
      else if(!strcasecmp(s,"Chance")) sr->s.chance=parm_num();
      else if(!strcasecmp(s,"Random")||!strcasecmp(s,"Redir")) {
	 if(sr->s.nredir>=MAX_REDIR_SOUNDS) err("too many sound redirects");
	 sr->s.redir[sr->s.nredir]=parm_sound();
	 sr->s.nredir++;
      }
      else break;
   }
   unget_token();
}

int ltacomp(const char *s,LT_Action *lta,int *stage) {
      if(!strcasecmp(s,"WaitFor")) {
	 s=next_token();
	 if(s==NULL||*s=='\n') synerr("eventtype expected after WaitFor");
	 lta->waittype=atoeventtype(s);
      }
      else if(!strcasecmp(s,"Manual")) lta->flags|=LTA_MANUAL;
      else if(!strcasecmp(s,"ManualF")) lta->flags|=LTA_MANUAL_FRONT;
      else if(!strcasecmp(s,"DonutOuter")) lta->flags|=LTA_DONUT_OUTER;
      else if(!strcasecmp(s,"DonutInner")) lta->flags|=LTA_DONUT_INNER;
      else if(!strcasecmp(s,"Stair")) lta->flags|=LTA_STAIR;
      else if(!strcasecmp(s,"UnqueueAll")) lta->flags|=LTA_UNQUEUE_ALL;
      else if(!strcasecmp(s,"FastCrush")) lta->flags|=LTA_FASTCRUSH;
      else if(!strcasecmp(s,"SlowCrush")) lta->flags|=LTA_SLOWCRUSH;
      else if(!strcasecmp(s,"NoCrush")) lta->flags|=LTA_NOCRUSH;
      else if(!strcasecmp(s,"TriggerModel")) /*lta->flags|=LTA_TRIG_MODEL*/;
      else if(!strcasecmp(s,"NumericModel")) lta->flags|=LTA_NUM_MODEL;
      /*else if(!strcasecmp(s,"Lockout")) lta->flags|=LTA_LOCKOUT;*/
      else if(!strcasecmp(s,"Delay")) lta->delay=parm_time();
      else if(!strcasecmp(s,"Plus")) lta->term_offset[*stage]=parm_num();
      else if(!strcasecmp(s,"Minus")) lta->term_offset[*stage]=-parm_num();
      else if(!strcasecmp(s,"To")) lta->term_type[*stage=0]=parm_termtype();
      else if(!strcasecmp(s,"BackTo")) {
	 lta->term_type[*stage=1]=parm_termtype();
	 lta->speed[*stage]=-lta->speed[0];
      }
      else if(!strcasecmp(s,"Speed")) {
	 int i=parm_speed();
	 if(lta->speed[*stage]<0) lta->speed[*stage]=-i;
	 else lta->speed[*stage]=i;
      }
      else if(!strcasecmp(s,"Up")) {
	 if(lta->speed[*stage]<0) lta->speed[*stage]=-lta->speed[*stage];
      }
      else if(!strcasecmp(s,"Down")) {
	 if(lta->speed[*stage]>0) lta->speed[*stage]=-lta->speed[*stage];
      }
      else if(!strcasecmp(s,"SpawnType")) lta->spawntype=parm_proto();
      else if(!strcasecmp(s,"Sound")) lta->sound=parm_sound();
      else if(!strcasecmp(s,"ContSound")) lta->contsound=parm_sound();
      else if(!strcasecmp(s,"StartSound")) lta->sound=parm_sound();
      else if(!strcasecmp(s,"StopSound")) lta->stopsound=parm_sound();
      else return 0;
      return -1;
}

void ltcomp(int issect) {
   const char *s;
   LineTypeRec *p;
   LT_Action *lta=NULL;
   int id,nactions=0;
   char myname[NAMELEN];
   int stage=0;
   /* two parameters, name and id */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after Line/SectorType");
   strncpy(myname,s,NAMELEN-1);
   myname[NAMELEN-1]=0;
   s=next_token();
   if(s==NULL||*s=='\n') synerr("ID expected after Line/SectorType");
   id=atoi(s);
   /* allocate a sectortype */
   if(issect) {
      /* check allocation */
      if(id>=maxsts) {
	 maxsts=id+ALLOC_BLK;
	 sts=(LineTypeRec*)safe_realloc(sts,maxsts*sizeof(LineTypeRec));
	 memset(sts+nsts,0,(maxsts-nsts)*sizeof(LineTypeRec));
      }
      /* deal with id */
      if(id>=nsts) nsts=id+1;
      p=sts+id;
   }
   /* allocate a linetype */
   else {
      /* check allocation */
      if(id>=maxlts) {
	 maxlts=id+ALLOC_BLK;
	 lts=(LineTypeRec*)safe_realloc(lts,maxlts*sizeof(LineTypeRec));
	 memset(lts+nlts,0,(maxlts-nlts)*sizeof(LineTypeRec));
      }
      /* deal with id */
      if(id>=nlts) nlts=id+1;
      p=lts+id;
   }
   /* check uniqueness */
   if(p->name[0]) {
      printf("ptcomp: line %d (%s): ID %d (%s) not unique (clashes with %s)\n",
	     line,cppfile,id,myname,p->name);
      exit(2);
   }
   strcpy(p->name,myname);
   /* start filling in data */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Action")) {
	 lta=p->l.action+nactions++;
	 if(nactions>MAX_LT_ACTIONS) err("too many actions");
	 s=next_token();
	 if(s==NULL||*s=='\n') synerr("lumptype expected after Action");
	 lta->lumptype=atolumptype(s);
	 s=next_token();
	 if(s==NULL||*s=='\n') synerr("eventtype expected after Action");
	 lta->eventtype=atoeventtype(s);
	 lta->sound=lta->stopsound=-1;
	 lta->speed[0]=default_speed;
      }
      else if(!strcasecmp(s,"KeyType")) p->l.keytype=parm_num();
      else if(!strcasecmp(s,"Damage")) p->l.damage=parm_num();
      else if(!strcasecmp(s,"SpotType")) p->l.spottype=parm_proto();
      else if(!strcasecmp(s,"Repeatable")) p->l.flags|=LT_REPEATABLE;
      else if(!strcasecmp(s,"AllowPlayer")) p->l.flags|=LT_ALLOW_PLAYER;
      else if(!strcasecmp(s,"AllowMonster")) p->l.flags|=LT_ALLOW_NONPLAYER;
      else if(!strcasecmp(s,"OnThumped")) p->l.flags|=LT_ON_THUMPED;
      else if(!strcasecmp(s,"OnCrossed")) p->l.flags|=LT_ON_CROSSED;
      else if(!strcasecmp(s,"OnActivated")) p->l.flags|=LT_ON_ACTIVATED;
      else if(!strcasecmp(s,"OnDamaged")) p->l.flags|=LT_ON_DAMAGED;
      else if(!strcasecmp(s,"FrontOnly")) p->l.flags|=LT_FRONT_ONLY;
      else if(lta==NULL) break;
      else if(ltacomp(s,lta,&stage));
      else break;
   }
   unget_token();
}

void gettcomp(void) {
   const char *s;
   GettableRec *p;
   /* make new gettable */
   if(ngetts>=maxgetts-1) {
      maxgetts+=ALLOC_BLK;
      getts=(GettableRec*)safe_realloc(getts,sizeof(GettableRec)*maxgetts);
   }
   p=getts+(ngetts++);
   memset(p,0,sizeof(GettableRec));
   p->g.bulletkind=-1;
   p->g.bogotype=-1;
   p->g.usenum=1;
   p->g.sound=-1;
   p->g.pickup_sound=-1;
   /* one parameter, name */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after Gettable");
   strncpy(p->name,s,NAMELEN-1);
   /* now the info */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"XCenter")) p->g.flags|=GK_XCENTERICON;
      else if(!strcasecmp(s,"YCenter")) p->g.flags|=GK_YCENTERICON;
      else if(!strcasecmp(s,"WepSel")) p->g.flags|=GK_WEPSELECT;
      else if(!strcasecmp(s,"SpeSel")) p->g.flags|=GK_SPESELECT;
      else if(!strcasecmp(s,"GotThe")) p->g.flags|=GK_GOT_THE;
      else if(!strcasecmp(s,"GotA")) p->g.flags|=GK_GOT_A;
      else if(!strcasecmp(s,"Local")) p->g.flags|=GK_LOCAL;
      else if(!strcasecmp(s,"KeyType")) p->g.key=parm_num();
      else if(!strcasecmp(s,"Decay")) p->g.decay=parm_num();
      else if(!strcasecmp(s,"Timing")) p->g.timing=parm_time();
      else if(!strcasecmp(s,"Sound")) p->g.sound=parm_sound();
      else if(!strcasecmp(s,"PickupSound")) p->g.pickup_sound=parm_sound();
      else if(!strcasecmp(s,"Special")) p->g.special=parm_num();
      else if(!strcasecmp(s,"Collect")) p->g.collect=parm_num();
      else if(!strcasecmp(s,"Initial")) p->g.initial=parm_num();
      else if(!strcasecmp(s,"Usenum")) p->g.usenum=parm_num();
      else if(!strcasecmp(s,"AddAmmo")) p->g.bulletadd=parm_num();
      else if(!strcasecmp(s,"Ammo")) p->g.bulletkind=parm_gett();
      /*else if(!strcasecmp(s,"Shoots")) p->g.bullettype=parm_proto();*/
      else if(!strcasecmp(s,"Bogotype")) p->g.bogotype=parm_proto();
      else if(!strcasecmp(s,"Icon")) parm_str(p->g.iconname,10);
      else if(!strcasecmp(s,"Anim")) p->g.iconanim=parm_ch();
      else if(!strcasecmp(s,"RevAnim")) {
	 p->g.iconanim=parm_ch();
	 p->g.flags|=GK_REVANIM;
      }
      else if(!strcasecmp(s,"Message")) parm_msg(p->g.string,GK_MSG_LEN);
      /*else if(!strcasecmp(s,"ShootMany")) {
	 p->g.shootnum=parm_num();
	 p->g.shootarc=FIXED_PI/parm_num();
      }*/
      else if(!strcasecmp(s,"IconPos")) {
	 p->g.xo=parm_num();
	 p->g.yo=parm_num();
      }
      else break;
   }
   unget_token();
}

void animcomp(int is_sw) {
   const char *s;
   int is_flat=0,parm=1;
   AnimRec *p;
   AnimTexTable *at=NULL;
   int defdur=150/MSEC_PER_TICK;
   /* make new animrec */
   if(nanims>=maxanims-1) {
      maxanims+=ALLOC_BLK;
      anims=(AnimRec*)safe_realloc(anims,sizeof(AnimRec)*maxanims);
   }
   p=anims+(nanims++);
   memset(p,0,sizeof(AnimRec));
   p->maxtbl=ALLOC_BLK;
   p->tbl=(AnimTexTable*)safe_malloc(p->maxtbl*sizeof(AnimRec));
   /* one parameter, name */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after AnimTex");
   strncpy(p->name,s,NAMELEN-1);
   /*logprintf(LOG_DEBUG,'P',"anim: <%s> %d %d",p->name,anims-p,ALLOC_BLK);*/
   /* now the info */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Flat")) is_flat=1;
      else if(!strcasecmp(s,"Parm")) parm=parm_num();
      else if(!strcasecmp(s,"Duration")) {
	 if(at) at->duration=parm_time();
	 else defdur=parm_time();
      }
      else if(!strcasecmp(s,"To")) {
	 int i,n=1+parm_num()-parm;
	 if(n<=0) synerr("To must exceed Parm");
	 free(p->tbl);
	 p->tbl=(AnimTexTable*)safe_calloc(n,sizeof(AnimTexTable));
	 p->maxtbl=p->ntbl=n;
	 for(i=0;i<n;i++) {
	    at=p->tbl+i;
	    sprintf(at->name,p->name,i+parm);
	    if(is_sw) at->flags|=AT_SWITCH;
	    if(is_flat) at->flags|=AT_FLAT;
	    at->myseqnum=i;
	    at->duration=defdur;
	 }
      } else if(!strcasecmp(s,"Tag")) {
	 if(p->ntbl) synerr("Tag must come before any Texture");
	 p->ntbl++;
	 memset(p->tbl,0,sizeof(AnimTexTable));
	 strncpy(p->tbl->name,p->name,8);
	 p->tbl->myseqnum=0xff;
      }
      else if(!strcasecmp(s,"Texture")) {
	 at=p->tbl+p->ntbl;
	 p->ntbl++;
	 if(p->ntbl>=p->maxtbl) {
	    p->maxtbl+=ALLOC_BLK;
	    p->tbl=(AnimTexTable*)safe_realloc(p->tbl,
					       p->maxtbl*sizeof(AnimRec));
	 }
	 memset(at,0,sizeof(AnimTexTable));
	 parm_str(at->name,9);
	 if(is_sw) at->flags|=AT_SWITCH;
	 if(is_flat) at->flags|=AT_FLAT;
	 at->myseqnum=p->ntbl-1;
	 at->duration=defdur;
      }
      else break;
   }
   unget_token();
}

void
phasecomp(void)
{
   const char *s;
   ThingPhaseRec *p;
   ThingPhase def;
   ThingPhase *tp=&def;
   /* make new phasetable */
   memset(&def,0,sizeof(ThingPhase));
   def.id=nphasetbls+1;
   def.next=-1;
   def.sound=-1;
   def.wait=1;
   if(nphasetbls>=maxphasetbls-1) {
      maxphasetbls+=ALLOC_BLK;
      phases=(ThingPhaseRec*)safe_realloc(phases,
					  sizeof(ThingPhaseRec)*maxphasetbls);
   }
   p=phases+(nphasetbls++);
   memset(p,0,sizeof(ThingPhaseRec));
   memset(p->signals,-1,sizeof(short)*NUM_THINGSIGS);
   p->signals[TS_INIT]=0;
   p->maxphases=ALLOC_BLK;
   p->tp=(ThingPhase*)safe_malloc(p->maxphases*sizeof(ThingPhase));
   p->tpname=(char*)safe_malloc(p->maxphases*NAMELEN);
   /* one parameter, name */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after PhaseTable");
   strncpy(p->name,s,NAMELEN-1);
   /* now the phase table */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Default")) tp=&def;
      else if(!strcasecmp(s,"Phase")) {
	 /* add a new phase to the table */
	 if(p->nphases>=p->maxphases-1) {
	    p->maxphases+=ALLOC_BLK;
	    p->tp=(ThingPhase*)safe_realloc(p->tp,p->maxphases*sizeof(ThingPhase));
	    p->tpname=(char*)safe_realloc(p->tpname,p->maxphases*NAMELEN);
	 }
	 tp=p->tp+p->nphases;
	 memcpy(tp,&def,sizeof(ThingPhase));
	 def.id=0;
	 parm_str(p->tpname+p->nphases*NAMELEN,NAMELEN);
	 p->nphases++;
	 s=next_token();
	 if(s!=NULL) tp->spr_phase=*s;
      }
      else if(!strcasecmp(s,"SigDetect")) 
	 p->signals[TS_DETECT]=p->nphases-1;
      else if(!strcasecmp(s,"SigFight"))
	 p->signals[TS_FIGHT]=p->nphases-1;
      else if(!strcasecmp(s,"SigShoot")) 
	 p->signals[TS_SHOOT]=p->nphases-1;
      else if(!strcasecmp(s,"SigSpecial")) 
	 p->signals[TS_SPECIAL]=p->nphases-1;
      else if(!strcasecmp(s,"SigOuch")) 
	 p->signals[TS_OUCH]=p->nphases-1;
      else if(!strcasecmp(s,"SigDie")) 
	 p->signals[TS_DIE]=p->nphases-1;
      else if(!strcasecmp(s,"SigExplode")) 
	 p->signals[TS_EXPLODE]=p->nphases-1;
      else if(!strcasecmp(s,"SigReanimate")) 
	 p->signals[TS_ANIMATE]=p->nphases-1;
      else if(!strcasecmp(s,"SigInit")) 
	 p->signals[TS_INIT]=p->nphases-1;
      else if(!strcasecmp(s,"Glow")) tp->flags|=TPH_GLOW;
      else if(!strcasecmp(s,"NoGlow")) tp->flags &= ~TPH_GLOW;
      else if(!strcasecmp(s,"Destroy")) tp->flags|=TPH_DESTROY;
      else if(!strcasecmp(s,"NoDestroy")) tp->flags &= ~TPH_DESTROY;
      else if(!strcasecmp(s,"NoSigs")) tp->flags|=TPH_NOSIGS;
      else if(!strcasecmp(s,"Sigs")) tp->flags &= ~TPH_NOSIGS;
      else if(!strcasecmp(s,"Strategy")) tp->flags|=TPH_STRATEGY;
      else if(!strcasecmp(s,"NoStrategy")) tp->flags &= ~TPH_STRATEGY;
      else if(!strcasecmp(s,"HeatSeek")) tp->flags|=TPH_HEATSEEK;
      else if(!strcasecmp(s,"NoHeatSeek")) tp->flags &= ~TPH_HEATSEEK;
      else if(!strcasecmp(s,"Shoot")) tp->flags|=TPH_SHOOT;
      else if(!strcasecmp(s,"NoShoot")) tp->flags &= ~TPH_SHOOT;
      else if(!strcasecmp(s,"Explode")) tp->flags|=TPH_EXPLODE;
      else if(!strcasecmp(s,"NoExplode")) tp->flags &= ~TPH_EXPLODE;
      else if(!strcasecmp(s,"Melee")) tp->flags|=TPH_MELEE;
      else if(!strcasecmp(s,"NoMelee")) tp->flags &= ~TPH_MELEE;
      else if(!strcasecmp(s,"Spawn2")) tp->flags|=TPH_SPAWN2;
      else if(!strcasecmp(s,"NoSpawn2")) tp->flags &= ~TPH_SPAWN2;
      else if(!strcasecmp(s,"RSpawn2")) tp->flags|=TPH_RSPAWN2;
      else if(!strcasecmp(s,"NoRSpawn2")) tp->flags &= ~TPH_RSPAWN2;
      else if(!strcasecmp(s,"Idle")) tp->flags|=TPH_IDLE;
      else if(!strcasecmp(s,"NoIdle")) tp->flags &= ~TPH_IDLE;
      /*else if(!strcasecmp(s,"Noisy")) tp->flags|=TPH_NOISY;*/
      else if(!strcasecmp(s,"BFGEffect")) tp->flags|=TPH_BFGEFFECT;
      else if(!strcasecmp(s,"NoBFGEffect")) tp->flags &= ~TPH_BFGEFFECT;
      else if(!strcasecmp(s,"Spawn")) tp->flags|=TPH_SHOOT;
      else if(!strcasecmp(s,"NoSpawn")) tp->flags &= ~TPH_SHOOT;
      else if(!strcasecmp(s,"Become")) {
	 tp->flags|=TPH_BECOME;
	 tp->next=0;
      }
      else if(!strcasecmp(s,"Become2")) {
	 tp->flags|=TPH_BECOME2;
	 tp->next=0;
      }
      else if(!strcasecmp(s,"Duration")) tp->wait=parm_time();
      else if(!strcasecmp(s,"RndDuration")) tp->rwait=parm_time();
      else if(!strcasecmp(s,"Goto")) tp->next=parm_phase(p);
      else if(!strcasecmp(s,"Sound")) tp->sound=parm_sound();
      else if(!strcasecmp(s,"SpritePh")) tp->spr_phase=parm_ch();
      else if(!strcasecmp(s,"Sprite")) parm_str(tp->sprite,5);
      else break;
   }
   p->tp=(ThingPhase*)safe_realloc(p->tp,p->nphases*sizeof(ThingPhase));
   p->tpname=(char *)safe_realloc(p->tpname,p->nphases*NAMELEN);
   p->maxphases=p->nphases;
   unget_token();
}

static void set_phid(ProtoThingRec *p,ThingPhaseRec *tpr) {
   p->pt.phase_id=tpr->tp->id;
   memcpy(p->pt.signals,tpr->signals,sizeof(short)*NUM_THINGSIGS);
}

void protocomp(void) {
   const char *s;
   ProtoThingRec *p;
   ThingPhaseRec *tpr;
   /* make new proto */
   if(nprotos>=maxprotos-1) {
      maxprotos+=ALLOC_BLK;
      protos=(ProtoThingRec*)safe_realloc(protos,
					  sizeof(ProtoThingRec)*maxprotos);
   }
   p=protos+(nprotos++);
   memset(p,0,sizeof(ProtoThingRec));
   p->pt.hits=-1;
   p->pt.artitype=-1;
   p->pt.shootnum=1;
   p->pt.flags|=PT_INF_MASS;
   p->pt.speed=FIXED_ONE;
   p->pt.see_arc=(FIXED_PI-FIXED_PI/4);
   p->pt.aim_arc=(FIXED_PI/4);
   p->pt.become1=-1;
   p->pt.become2=-1;
   /* two parameters, name and id */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after Proto");
   strncpy(p->name,s,NAMELEN-1);
   tpr=find_ph_tbl(p->name);
   if(tpr) set_phid(p,tpr);
   s=next_token();
   if(s==NULL||*s=='\n') 
      p->pt.id=top_assigned_proto_id++;
   else chkuniqueid(p->pt.id=atoi(s));
   /* now info for this proto */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Phases")) set_phid(p,tpr=parm_ph_tbl());
      else if(!strcasecmp(s,"SigDetect")) 
	 p->pt.signals[TS_DETECT]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigFight")) 
	 p->pt.signals[TS_FIGHT]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigShoot")) 
	 p->pt.signals[TS_SHOOT]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigSpecial")) 
	 p->pt.signals[TS_SPECIAL]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigOuch")) 
	 p->pt.signals[TS_OUCH]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigDie")) 
	 p->pt.signals[TS_DIE]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigExplode")) 
	 p->pt.signals[TS_EXPLODE]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigReanimate")) 
	 p->pt.signals[TS_ANIMATE]=parm_phase(tpr);
      else if(!strcasecmp(s,"SigInit")) 
	 p->pt.signals[TS_INIT]=parm_phase(tpr);
      else if(!strcasecmp(s,"Splat")) p->pt.bloodtype=parm_proto();
      else if(!strcasecmp(s,"Shoots")) p->pt.spawn1=parm_proto();
      else if(!strcasecmp(s,"Spawns")) p->pt.spawn1=parm_proto();
      else if(!strcasecmp(s,"Spawn2")) p->pt.spawn2=parm_proto();
      else if(!strcasecmp(s,"Becomes")) p->pt.become1=parm_proto();
      else if(!strcasecmp(s,"Become2")) p->pt.become2=parm_proto();
      else if(!strcasecmp(s,"Sprite")) parm_str(p->pt.sprite,6);
      else if(!strcasecmp(s,"Damage")) p->pt.damage=parm_num();
      else if(!strcasecmp(s,"Speed")) p->pt.speed=parm_num()<<12;
      else if(!strcasecmp(s,"Mass")) {
	 int i=parm_num();
	 p->pt.flags&=~PT_INF_MASS;
	 if(i) {
	    p->pt.realmass=(FIXED_ONE*i)/100;
	    p->pt.friction=(61*FIXED_ONE)/64;
	 }
	 else {
	    p->pt.realmass=p->pt.friction=FIXED_ONE;
	    p->pt.flags|=PT_CAN_FLY;
	 }
      }
      else if(!strcasecmp(s,"Hits")) p->pt.hits=parm_num();
      else if(!strcasecmp(s,"Shooter")) p->pt.flags|=PT_SHOOTER;
      /*else if(!strcasecmp(s,"Noisy")) p->pt.flags|=PT_NOISY;*/
      else if(!strcasecmp(s,"Beastie")) p->pt.flags|=PT_BEASTIE|PT_TARGET;
      else if(!strcasecmp(s,"Explosive")) p->pt.flags|=PT_EXPLOSIVE;
      else if(!strcasecmp(s,"ZCenter")) p->pt.flags|=PT_ZCENTER;
      else if(!strcasecmp(s,"PHANTOM")) p->pt.flags|=PT_PHANTOM;
      else if(!strcasecmp(s,"SKIRTCLIFFS")) p->pt.flags|=PT_SKIRT_CLIFFS;
      else if(!strcasecmp(s,"CANFLY")) p->pt.flags|=PT_CAN_FLY;
      else if(!strcasecmp(s,"Flying")) p->pt.flags|=PT_CAN_FLY;
      else if(!strcasecmp(s,"PINVIS")) p->pt.flags|=PT_PINVIS;
      else if(!strcasecmp(s,"BULLETKLUDGE")) p->pt.flags|=PT_BULLET_KLUDGE;
      else if(!strcasecmp(s,"FASTSHOOT")) p->pt.flags|=PT_FAST_SHOOTER;
      else if(!strcasecmp(s,"Hanging")) p->pt.flags|=PT_HANGING;
      else if(!strcasecmp(s,"YMoveOnly")) p->pt.flags|=PT_YMOVE_ONLY;
      else if(!strcasecmp(s,"Player")) p->pt.flags|=PT_PLAYER;
      else if(!strcasecmp(s,"Target")) p->pt.flags|=PT_TARGET;
      else if(!strcasecmp(s,"BulletProof")) p->pt.flags|=PT_BULLETPROOF;
      else if(!strcasecmp(s,"FloatsUp")) p->pt.flags|=PT_FLOATSUP;
      else if(!strcasecmp(s,"ZPegged")) p->pt.flags|=PT_ZPEG;
      else if(!strcasecmp(s,"Bouncy")) p->pt.flags|=PT_BOUNCY;
      else if(!strcasecmp(s,"Blocking")) p->pt.flags|=PT_BLOCKING;
      else if(!strcasecmp(s,"Bogus")) p->pt.flags|=PT_BOGUS|PT_PHANTOM;
      else if(!strcasecmp(s,"SpawnSpot")) p->pt.flags|=PT_SPAWNSPOT;
      else if(!strcasecmp(s,"NoHurtOwner")) p->pt.flags|=PT_NOHURTO;
      else if(!strcasecmp(s,"TurnWhenHitting")) p->pt.flags|=PT_TURNWHENHITTING;
      else if(!strcasecmp(s,"BulletAttack")) p->pt.flags|=PT_BULLET;
      else if(!strcasecmp(s,"SeeArc")) p->pt.see_arc=parm_arc();
      else if(!strcasecmp(s,"AimArc")) p->pt.aim_arc=parm_arc();
      else if(!strcasecmp(s,"ShootMany")) {
	 p->pt.shootnum=parm_num();
	 p->pt.shootarc=parm_arc();
      }
      else if(!strcasecmp(s,"ShootPara")) {
	 p->pt.flags|=PT_PARA_SHOOT;
	 p->pt.shootnum=parm_num();
	 p->pt.shootarc=parm_num()<<12;
      }
      else if(!strcasecmp(s,"Size")) {
	 p->pt.radius=parm_num()<<11;
	 p->pt.height=parm_num()<<12;
      }
      else if(!strcasecmp(s,"Gets")) {
	 p->pt.artitype=parm_gett();
	 p->pt.artinum=parm_num();
      }
      else break;
   }
   unget_token();
}

static int lookup_linfo(const char *n) {
   int i;
   LevInfoRec *li;
   for(i=0,li=linfos;i<nlinfos;i++,li++)
      if(!strcasecmp(li->l.name,n)) return i;
   printf("unrecognised levelname: %s\n",n);
   return -1;
}
void licomp(void) {
   const char *s;
   LevInfoRec *li;
   /* make new li */
   if(nlinfos>=maxlinfos-1) {
      maxlinfos+=ALLOC_BLK;
      linfos=(LevInfoRec*)safe_realloc(linfos,
				    sizeof(LevInfoRec)*maxlinfos);
   }
   li=linfos+(nlinfos++);
   memset(li,0,sizeof(LevInfoRec));
   li->l.secret=-1;
   li->l.next=-1;
   /* one parameter, name */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after Level");
   strncpy(li->l.name,s,8);
   /* now parse other keywords */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Music")) parm_str(li->l.music,9);
      else if(!strcasecmp(s,"Sky")) parm_str(li->l.sky,9);
      else if(!strcasecmp(s,"StartGame")) li->l.flags|=LI_START;
      else if(!strcasecmp(s,"EndGame")) li->l.flags|=LI_END;
      else if(!strcasecmp(s,"Secret")) parm_str(li->secret,9);
      else if(!strcasecmp(s,"Next")) parm_str(li->next,9);
      else break;
   }
   unget_token();
}

void ptcomp(void) {
   while(1) {
      const char *s=next_token();
      if(s==NULL) break;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Proto")) protocomp();
      else if(!strcasecmp(s,"PhaseTable")) phasecomp();
      else if(!strcasecmp(s,"Gettable")) gettcomp();
      else if(!strcasecmp(s,"LineType")) ltcomp(0);
      else if(!strcasecmp(s,"SectorType")) ltcomp(1);
      else if(!strcasecmp(s,"SoundType")) secomp();
      else if(!strcasecmp(s,"AnimTexture")) animcomp(0);
      else if(!strcasecmp(s,"SwitchTexture")) animcomp(1);
      else if(!strcasecmp(s,"SwTexture")) animcomp(1);
      else if(!strcasecmp(s,"Level")) licomp();
      else if(!strcasecmp(s,"TimeUnits")) default_time_units=parm_time();
      else if(!strcasecmp(s,"DefaultSpeed")) default_speed=parm_speed();
      else synerr(NULL);
   }
}

void wrphases(FILE *fout) {
   int i;
   printf("%5d phase tables\n",nphasetbls);
   for(i=0;i<nphasetbls;i++)
      fwrite(phases[i].tp,sizeof(ThingPhase),phases[i].nphases,fout);
}
void wrprotos(FILE *fout) {
   int i;
   printf("%5d protos\n",nprotos);
   for(i=0;i<nprotos;i++) {
      if(protos[i].pt.phase_id<1) 
	 printf("warning: proto %s (%d) has no phasetable\n",
		protos[i].name,(int)(protos[i].pt.id));
      fwrite(&protos[i].pt,sizeof(ProtoThing),1,fout);
   }
}
void wrgetts(FILE *fout) {
   int i;
   printf("%5d gettables\n",ngetts);
   for(i=0;i<ngetts;i++) 
      fwrite(&getts[i].g,sizeof(Gettable),1,fout);
}
void wrlts(FILE *fout) {
   int i;
   printf("%5d linetypes\n",nlts);
   for(i=0;i<nlts;i++) 
      fwrite(&lts[i].l,sizeof(LineType),1,fout);
}
void wrsts(FILE *fout) {
   int i;
   printf("%5d sectortypes\n",nsts);
   for(i=0;i<nsts;i++) 
      fwrite(&sts[i].l,sizeof(LineType),1,fout);
}
void wrsounds(FILE *fout) {
   int i;
   printf("%5d sounds\n",nsounds);
   for(i=0;i<nsounds;i++) 
      fwrite(&sounds[i].s,sizeof(SoundEnt),1,fout);
}
void wrlinfos(FILE *fout) {
   int i;
   printf("%5d levinfos\n",nlinfos);
   for(i=0;i<nlinfos;i++) {
      if(linfos[i].secret[0])
	 linfos[i].l.secret=lookup_linfo(linfos[i].secret);
      if(linfos[i].next[0])
	 linfos[i].l.next=lookup_linfo(linfos[i].next);
      fwrite(&linfos[i].l,sizeof(LevInfo),1,fout);
   }
}
void wranims(FILE *fout) {
   int i;
   printf("%5d animtexes\n",nanims);
   for(i=0;i<nanims;i++) {
      int j;
      /*printf("         %s: %d textures\n",anims[i].name,anims[i].ntbl);*/
      for(j=0;j<anims[i].ntbl;j++)
	 anims[i].tbl[j].seqlen=anims[i].ntbl;
      fwrite(anims[i].tbl,sizeof(AnimTexTable),anims[i].ntbl,fout);
   }
}

static int ptcmp(const void *p1,const void *p2) {
   if(((const ProtoThingRec *)p1)->pt.id>((const ProtoThingRec *)p2)->pt.id) 
      return 1;
   if(((const ProtoThingRec *)p1)->pt.id<((const ProtoThingRec *)p2)->pt.id) 
      return -1;
   return 0;
}

int main(int argc,char **argv) {
   FILE *fout;
   char foutname[256];
   if(argc!=2) {
      printf("Usage: ptcomp <dir>\n\n"
	     "ptcomp is used to compile .pt files into the PHASES,\n"
	     "PROTOS, and GETTABLE lumps used by DUMB.  It reads .pt\n"
	     "source from the standard input, and writes its output\n"
	     "into files of the form <dir>/PROTOS.lump\n"
	     "\n");
      return 1;
   }
   log_stdout(); /* for error messages from safem */
   maxlinfos=maxprotos=maxphasetbls=maxgetts=
      maxlts=maxsts=maxsounds=maxanims=ALLOC_BLK;
   protos=(ProtoThingRec*)safe_malloc(maxprotos*sizeof(ProtoThingRec));
   phases=(ThingPhaseRec*)safe_malloc(maxphasetbls*sizeof(ThingPhaseRec));
   getts=(GettableRec*)safe_malloc(maxgetts*sizeof(GettableRec));
   lts=(LineTypeRec*)safe_calloc(maxlts,sizeof(LineTypeRec));
   sts=(LineTypeRec*)safe_calloc(maxsts,sizeof(LineTypeRec));
   sounds=(SoundRec*)safe_calloc(maxsounds,sizeof(SoundRec));
   anims=(AnimRec*)safe_calloc(maxanims,sizeof(AnimRec));
   linfos=(LevInfoRec*)safe_malloc(maxlinfos*sizeof(LevInfoRec));
   fin=stdin;
   /*if(fin==NULL) {
      printf("Error %d opening source file\n",errno);
      return 2;
   }*/

   printf("ptcomp: compiling...\n");
   ptcomp();
   /*fclose(fin);*/

   printf("ptcomp: sorting...\n");
   qsort(protos,nprotos,sizeof(ProtoThingRec),ptcmp);

   printf("ptcomp: writing...\n");
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/PHASES.lump"),"wb");
   if(fout) {
      wrphases(fout);
      fclose(fout);
   }
   else printf("Error %d opening phase file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/PROTOS.lump"),"wb");
   if(fout) {
      wrprotos(fout);
      fclose(fout);
   }
   else printf("Error %d opening proto file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/GETTABLE.lump"),"wb");
   if(fout) {
      wrgetts(fout);
      fclose(fout);
   }
   else printf("Error %d opening gettable file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/LINETYPE.lump"),"wb");
   if(fout) {
      wrlts(fout);
      fclose(fout);
   }
   else printf("Error %d opening linetype file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/SECTTYPE.lump"),"wb");
   if(fout) {
      wrsts(fout);
      fclose(fout);
   }
   else printf("Error %d opening sectortype file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/SOUNDS.lump"),"wb");
   if(fout) {
      wrsounds(fout);
      fclose(fout);
   }
   else printf("Error %d opening sounds file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/ANIMTEX.lump"),"wb");
   if(fout) {
      wranims(fout);
      fclose(fout);
   }
   else printf("Error %d opening animtex file %s\n",errno,foutname);
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/LEVINFO.lump"),"wb");
   if(fout) {
      wrlinfos(fout);
      fclose(fout);
   }
   else printf("Error %d opening levinfo file %s\n",errno,foutname);
   return 0;
}
