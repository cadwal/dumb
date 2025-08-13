#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/safem.h"
#include "libdumb/dsound.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "soundcomp.h"

typedef struct {
   SoundEnt s;
   char name[NAMELEN];
} SoundRec;

static int nsounds=0, maxsounds=0;
static SoundRec *sounds;

static int new_sound(const char *name);


void
init_soundcomp(void)
{
   maxsounds = ALLOC_BLK;
   sounds = (SoundRec *) safe_calloc(maxsounds, sizeof (SoundRec));
}

void
soundcomp(void)
{
   const char *s;
   SoundRec *sr;
   /* one parameter: name */
   s=next_token();
   if(s==NULL||*s=='\n') synerr("name expected after SoundType");
   sr = &sounds[new_sound(s)];
   /* start filling in data */
   while(1) {
      s=next_token();
      if(s==NULL) return;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Bend")) sr->s.bend_range=parm_num();
      else if(!strcasecmp(s,"BendConst")) sr->s.bend_const=parm_num();
      else if(!strcasecmp(s,"Chance")) sr->s.chance=parm_num();
      else if(!strcasecmp(s,"Random")||!strcasecmp(s,"Redir")) {
	 if(sr->s.nredir>=MAX_REDIR_SOUNDS) 
	    err("too many sound redirects");
	 sr->s.redir[sr->s.nredir]=parm_sound();
	 sr->s.nredir++;
      } else
	 break;
   }
   unget_token();
}

void
wrsounds(FILE *fout)
{
   int i;
   printf("%5d sounds\n",nsounds);
   for(i=0;i<nsounds;i++)
      fwrite(&sounds[i].s,sizeof(SoundEnt),1,fout);
}

int
parm_sound(void)
{
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


static int
new_sound(const char *name)
{
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

// Local Variables:
// c-basic-offset: 3
// End:
