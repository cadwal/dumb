/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/animcomp.c: AnimTexture/SwitchTexture compiler.
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
#include "libdumbutil/timer.h"
#include "libdumb/animtexstruct.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "animcomp.h"

typedef struct {
   AnimTexTable *tbl;
   int ntbl, maxtbl;
   char name[NAMELEN];
} AnimRec;

static int nanims = 0, maxanims = 0;
static AnimRec *anims;


void
init_animcomp(void)
{
   maxanims = ALLOC_BLK;
   anims = (AnimRec *) safe_calloc(maxanims, sizeof(AnimRec));
}

void
animcomp(int is_sw)
{
   const char *s;
   int is_flat = 0, parm = 1;
   AnimRec *p;
   AnimTexTable *at = NULL;
   int defdur = 150 / MSEC_PER_TICK;
   /* make new animrec */
   if (nanims >= maxanims - 1) {
      maxanims += ALLOC_BLK;
      anims = (AnimRec *) safe_realloc(anims, sizeof(AnimRec) * maxanims);
   }
   p = anims + (nanims++);
   memset(p, 0, sizeof(AnimRec));
   p->maxtbl = ALLOC_BLK;
   p->tbl = (AnimTexTable *) safe_malloc(p->maxtbl * sizeof(AnimRec));
   /* one parameter, name */
   s = parm_name(_("Name expected after AnimTex"));
   strncpy(p->name, s, NAMELEN - 1);
   /*logprintf(LOG_DEBUG, 'P', "anim: <%s> %d %d",
      p->name, anims-p, ALLOC_BLK); */
   /* now the info */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "Flat"))
	 is_flat = 1;
      else if (!strcasecmp(s, "Parm"))
	 parm = parm_num();
      else if (!strcasecmp(s, "Duration")) {
	 if (at)
	    at->duration = parm_time();
	 else
	    defdur = parm_time();
      } else if (!strcasecmp(s, "To")) {
	 int i, to = parm_num(), n = 1 + to - parm;
	 if (n <= 0)
	    err(_("To (%d) must exceed Parm (%d)"), to, parm);
	 free(p->tbl);
	 p->tbl = (AnimTexTable *) safe_calloc(n, sizeof(AnimTexTable));
	 p->maxtbl = p->ntbl = n;
	 for (i = 0; i < n; i++) {
	    at = p->tbl + i;
	    sprintf(at->name, p->name, i + parm);
	    if (is_sw)
	       at->flags |= AT_SWITCH;
	    if (is_flat)
	       at->flags |= AT_FLAT;
	    at->myseqnum = i;
	    at->duration = defdur;
	 }
      } else if (!strcasecmp(s, "Tag")) {
	 if (p->ntbl)
	    synerr(_("Tag must come before any Texture"));
	 p->ntbl++;
	 memset(p->tbl, 0, sizeof(AnimTexTable));
	 strncpy(p->tbl->name, p->name, 8);
	 p->tbl->myseqnum = 0xff;
      } else if (!strcasecmp(s, "Texture")) {
	 at = p->tbl + p->ntbl;
	 p->ntbl++;
	 if (p->ntbl >= p->maxtbl) {
	    p->maxtbl += ALLOC_BLK;
	    p->tbl = (AnimTexTable *) safe_realloc(p->tbl,
					    p->maxtbl * sizeof(AnimRec));
	 }
	 memset(at, 0, sizeof(AnimTexTable));
	 parm_str(at->name, 9);
	 if (is_sw)
	    at->flags |= AT_SWITCH;
	 if (is_flat)
	    at->flags |= AT_FLAT;
	 at->myseqnum = p->ntbl - 1;
	 at->duration = defdur;
      } else
	 break;
   }
   unget_token();
}

void
wranims(WADWR *wout)
{
   int i;
   printf(_("%5d animtexes\n"), nanims);
   wadwr_lump(wout, "ANIMTEX");
   for (i = 0; i < nanims; i++) {
      int j;
      /*printf(_("         %s: %d textures\n"),anims[i].name,anims[i].ntbl); */
      for (j = 0; j < anims[i].ntbl; j++)
	 anims[i].tbl[j].seqlen = anims[i].ntbl;
      wadwr_write(wout, anims[i].tbl, anims[i].ntbl * sizeof(AnimTexTable));
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
