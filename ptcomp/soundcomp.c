/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/soundcomp.c: SoundType compiler.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#include <config.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

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

static int nsounds = 0, maxsounds = 0;
static SoundRec *sounds;

static int new_sound(const char *name);


void
init_soundcomp(void)
{
   maxsounds = ALLOC_BLK;
   sounds = (SoundRec *) safe_calloc(maxsounds, sizeof(SoundRec));
}

void
soundcomp(void)
{
   const char *s;
   SoundRec *sr;
   /* one parameter: name */
   s = parm_name(_("Name expected after SoundType"));
   sr = &sounds[new_sound(s)];
   sr->s.nredir=0;
   /* start filling in data */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "Bend"))
	 sr->s.bend_range = parm_int();
      else if (!strcasecmp(s, "BendConst"))
	 sr->s.bend_const = parm_int();
      else if (!strcasecmp(s, "Chance"))
	 sr->s.chance = parm_uint();
      else if (!strcasecmp(s, "Random") || !strcasecmp(s, "Redir")) {
	 if (sr->s.nredir >= MAX_REDIR_SOUNDS)
	    err(_("Too many sound redirects (max %d)"), (int) MAX_REDIR_SOUNDS);
	 sr->s.redir[sr->s.nredir] = parm_sound();
	 sr->s.nredir++;
      } else
	 break;
   }
   unget_token();
}

void
wrsounds(WADWR *wout)
{
   int i;
   printf(_("%5d sounds\n"), nsounds);
   wadwr_lump(wout, "SOUNDS");
   for (i = 0; i < nsounds; i++)
      wadwr_write(wout, &sounds[i].s, sizeof(SoundEnt));
}

int
parm_sound(void)
{
   int i;
   const char *s = parm_name(_("Sound name parameter expected"));
   if (isdigit(*s))
      return atoi(s);
   for (i = 0; i < nsounds; i++)
      if (!strcasecmp(sounds[i].name, s))
	 return i;
   /* sound name unrecognised */
   return new_sound(s);
}


static int
new_sound(const char *name)
{
   int i;
   for (i = 0; i < nsounds; i++)
      if (!strcasecmp(sounds[i].name, name))
	 err(_("Soundtype `%s' redefined"), name);
   nsounds++;
   if (nsounds >= maxsounds) {
      maxsounds += ALLOC_BLK;
      sounds = (SoundRec *) safe_realloc(sounds, maxsounds * sizeof(SoundRec));
   }
   memset(sounds+i,0,sizeof(SoundRec));
   strcpy(sounds[i].name, name);
   strcpy(sounds[i].s.lumpname, name);
   return i;
}

// Local Variables:
// c-basic-offset: 3
// End:
