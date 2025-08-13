#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/safem.h"
#include "libdumb/gettablestruct.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "gettcomp.h"
#include "protocomp.h"
#include "soundcomp.h"

typedef struct {
   Gettable g;
   char name[NAMELEN];
} GettableRec;

static int ngetts=0, maxgetts=0;
static GettableRec *getts;


void
init_gettcomp(void)
{
   maxgetts = ALLOC_BLK;
   getts = (GettableRec *) safe_malloc(maxgetts * sizeof (GettableRec));
}

void
gettcomp(void)
{
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
      else if(!strcasecmp(s,"IconPos")) {
	 p->g.xo=parm_num();
	 p->g.yo=parm_num();
      }
      else if (!strcasecmp(s, "WeaponNumber")) p->g.weaponnumber=parm_num();
      else if (!strcasecmp(s, "ReplaceWeapon")) p->g.replaceweapon=parm_gett();
      else break;
   }
   unget_token();
}

void
wrgetts(FILE *fout)
{
   int i;
   printf("%5d gettables\n",ngetts);
   for(i=0;i<ngetts;i++)
      fwrite(&getts[i].g,sizeof(Gettable),1,fout);
}

int
parm_gett(void)
{
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

// Local Variables:
// c-basic-offset: 3
// End:
