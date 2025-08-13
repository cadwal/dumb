#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/safem.h"
#include "libdumb/levinfostruct.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "licomp.h"

typedef struct {
   LevInfo l;
   char next[10];
   char secret[10];
} LevInfoRec;

static int nlinfos=0, maxlinfos=0;
static LevInfoRec *linfos;

static int lookup_linfo(const char *n);


void
init_licomp(void)
{
   maxlinfos = ALLOC_BLK;
   linfos = (LevInfoRec *) safe_malloc(maxlinfos * sizeof (LevInfoRec));
}

void
licomp(void)
{
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

void
wrlinfos(FILE *fout)
{
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


static int
lookup_linfo(const char *n)
{
   int i;
   LevInfoRec *li;
   for(i=0,li=linfos;i<nlinfos;i++,li++)
      if(!strcasecmp(li->l.name,n)) return i;
   printf("unrecognised levelname: %s\n",n);
   return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
