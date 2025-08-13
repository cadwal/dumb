/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/phasecomp.c: PhaseTable compiler.
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

#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "phasecomp.h"
#include "soundcomp.h"

static int nphasetbls = 0, maxphasetbls = 0;
static ThingPhaseRec *phases;


void
init_phasecomp(void)
{
   maxphasetbls = ALLOC_BLK;
   phases = (ThingPhaseRec *)
      safe_malloc(maxphasetbls * sizeof(ThingPhaseRec));
}

void
phasecomp(void)
{
   const char *s;
   ThingPhaseRec *p;
   ThingPhase def;
   ThingPhase *tp = &def;
   /* make new phasetable */
   memset(&def, 0, sizeof(ThingPhase));
   def.id = nphasetbls + 1;
   def.next = -1;
   def.sound = -1;
   def.wait = 1;
   if (nphasetbls >= maxphasetbls - 1) {
      maxphasetbls += ALLOC_BLK;
      phases = (ThingPhaseRec *)
	 safe_realloc(phases, sizeof(ThingPhaseRec) * maxphasetbls);
   }
   p = phases + (nphasetbls++);
   memset(p, 0, sizeof(ThingPhaseRec));
   memset(p->signals, -1, sizeof(short) * NUM_THINGSIGS);
   p->signals[TS_INIT] = 0;
   p->maxphases = ALLOC_BLK;
   p->tp = (ThingPhase *) safe_malloc(p->maxphases * sizeof(ThingPhase));
   p->tpname = (char *) safe_malloc(p->maxphases * NAMELEN);
   /* one parameter, name */
   s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("name expected after PhaseTable"));
   strncpy(p->name, s, NAMELEN - 1);
   /* now the phase table */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n');
      else if (!strcasecmp(s, "Default"))
	 tp = &def;
      else if (!strcasecmp(s, "Phase")) {
	 /* add a new phase to the table */
	 if (p->nphases >= p->maxphases - 1) {
	    p->maxphases += ALLOC_BLK;
	    p->tp = (ThingPhase *)
	       safe_realloc(p->tp, p->maxphases * sizeof(ThingPhase));
	    p->tpname = (char *)
	       safe_realloc(p->tpname, p->maxphases * NAMELEN);
	 }
	 tp = p->tp + p->nphases;
	 memcpy(tp, &def, sizeof(ThingPhase));
	 def.id = 0;
	 parm_str(p->tpname + p->nphases * NAMELEN, NAMELEN);
	 p->nphases++;
	 s = next_token();
	 if (s != NULL)
	    tp->spr_phase = *s;
      } else if (!strcasecmp(s, "SigDetect"))
	 p->signals[TS_DETECT] = p->nphases - 1;
      else if (!strcasecmp(s, "SigFight"))
	 p->signals[TS_FIGHT] = p->nphases - 1;
      else if (!strcasecmp(s, "SigShoot"))
	 p->signals[TS_SHOOT] = p->nphases - 1;
      else if (!strcasecmp(s, "SigSpecial"))
	 p->signals[TS_SPECIAL] = p->nphases - 1;
      else if (!strcasecmp(s, "SigOuch"))
	 p->signals[TS_OUCH] = p->nphases - 1;
      else if (!strcasecmp(s, "SigDie"))
	 p->signals[TS_DIE] = p->nphases - 1;
      else if (!strcasecmp(s, "SigExplode"))
	 p->signals[TS_EXPLODE] = p->nphases - 1;
      else if (!strcasecmp(s, "SigReanimate"))
	 p->signals[TS_ANIMATE] = p->nphases - 1;
      else if (!strcasecmp(s, "SigInit"))
	 p->signals[TS_INIT] = p->nphases - 1;
      else if (!strcasecmp(s, "Glow"))
	 tp->flags |= TPH_GLOW;
      else if (!strcasecmp(s, "NoGlow"))
	 tp->flags &= ~TPH_GLOW;
      else if (!strcasecmp(s, "Destroy"))
	 tp->flags |= TPH_DESTROY;
      else if (!strcasecmp(s, "NoDestroy"))
	 tp->flags &= ~TPH_DESTROY;
      else if (!strcasecmp(s, "NoSigs"))
	 tp->flags |= TPH_NOSIGS;
      else if (!strcasecmp(s, "Sigs"))
	 tp->flags &= ~TPH_NOSIGS;
      else if (!strcasecmp(s, "Strategy"))
	 tp->flags |= TPH_STRATEGY;
      else if (!strcasecmp(s, "NoStrategy"))
	 tp->flags &= ~TPH_STRATEGY;
      else if (!strcasecmp(s, "HeatSeek"))
	 tp->flags |= TPH_HEATSEEK;
      else if (!strcasecmp(s, "NoHeatSeek"))
	 tp->flags &= ~TPH_HEATSEEK;
      else if (!strcasecmp(s, "Shoot"))
	 tp->flags |= TPH_SHOOT;
      else if (!strcasecmp(s, "NoShoot"))
	 tp->flags &= ~TPH_SHOOT;
      else if (!strcasecmp(s, "Explode"))
	 tp->flags |= TPH_EXPLODE;
      else if (!strcasecmp(s, "NoExplode"))
	 tp->flags &= ~TPH_EXPLODE;
      else if (!strcasecmp(s, "Melee"))
	 tp->flags |= TPH_MELEE;
      else if (!strcasecmp(s, "NoMelee"))
	 tp->flags &= ~TPH_MELEE;
      else if (!strcasecmp(s, "Spawn2"))
	 tp->flags |= TPH_SPAWN2;
      else if (!strcasecmp(s, "NoSpawn2"))
	 tp->flags &= ~TPH_SPAWN2;
      else if (!strcasecmp(s, "RSpawn2"))
	 tp->flags |= TPH_RSPAWN2;
      else if (!strcasecmp(s, "NoRSpawn2"))
	 tp->flags &= ~TPH_RSPAWN2;
      else if (!strcasecmp(s, "Idle"))
	 tp->flags |= TPH_IDLE;
      else if (!strcasecmp(s, "NoIdle"))
	 tp->flags &= ~TPH_IDLE;
      /*else if(!strcasecmp(s,"Noisy")) tp->flags|=TPH_NOISY; */
      else if (!strcasecmp(s, "BFGEffect"))
	 tp->flags |= TPH_BFGEFFECT;
      else if (!strcasecmp(s, "NoBFGEffect"))
	 tp->flags &= ~TPH_BFGEFFECT;
      else if (!strcasecmp(s, "Spawn"))
	 tp->flags |= TPH_SHOOT;
      else if (!strcasecmp(s, "NoSpawn"))
	 tp->flags &= ~TPH_SHOOT;
      else if (!strcasecmp(s, "TInvis"))
	 tp->flags |= TPH_TINVIS;
      else if (!strcasecmp(s, "NoTInvis"))
	 tp->flags &= ~TPH_TINVIS;
      else if (!strcasecmp(s, "Whirly"))
	 tp->flags |= TPH_WHIRLY;
      else if (!strcasecmp(s, "NoWhirly"))
	 tp->flags &= ~TPH_WHIRLY;
      else if (!strcasecmp(s, "Charge"))
	 tp->flags |= TPH_CHARGE;
      else if (!strcasecmp(s, "NoCharge"))
	 tp->flags &= ~TPH_CHARGE;
      else if (!strcasecmp(s, "RSkip"))
	 tp->flags |= TPH_RSKIP;
      else if (!strcasecmp(s, "NoRSkip"))
	 tp->flags &= ~TPH_RSKIP;
      else if (!strcasecmp(s, "Become")) {
	 tp->flags |= TPH_BECOME;
	 tp->next = 0;
      } else if (!strcasecmp(s, "Become2")) {
	 tp->flags |= TPH_BECOME2;
	 tp->next = 0;
      } else if (!strcasecmp(s, "Duration"))
	 tp->wait = parm_time();
      else if (!strcasecmp(s, "RndDuration"))
	 tp->rwait = parm_time();
      else if (!strcasecmp(s, "Goto"))
	 tp->next = parm_phase(p);
      else if (!strcasecmp(s, "Sound"))
	 tp->sound = parm_sound();
      else if (!strcasecmp(s, "SpritePh"))
	 tp->spr_phase = parm_ch();
      else if (!strcasecmp(s, "Sprite"))
	 parm_str(tp->sprite, 5);
      else
	 break;
   }
   p->tp = (ThingPhase *) safe_realloc(p->tp, p->nphases * sizeof(ThingPhase));
   p->tpname = (char *) safe_realloc(p->tpname, p->nphases * NAMELEN);
   p->maxphases = p->nphases;
   unget_token();
}

void
wrphases(FILE *fout)
{
   int i;
   printf(_("%5d phase tables\n"), nphasetbls);
   for (i = 0; i < nphasetbls; i++)
      fwrite(phases[i].tp, sizeof(ThingPhase), phases[i].nphases, fout);
}


ThingPhaseRec *
find_ph_tbl(const char *s)
{
   int i;
   for (i = 0; i < nphasetbls; i++)
      if (!strcasecmp(phases[i].name, s))
	 return phases + i;
   return NULL;
}

ThingPhaseRec *
parm_ph_tbl(void)
{
   const char *s = next_token();
   ThingPhaseRec *tpr;
   if (s == NULL || *s == '\n')
      synerr(_("phase table name parameter expected"));
   tpr = find_ph_tbl(s);
   if (!tpr)
      synerr(_("phase table name unrecognised"));
   return tpr;
}

int
parm_phase(ThingPhaseRec *p)
{
   int i;
   const char *s = next_token();
   if (p == NULL)
      synerr(_("you need to specify a phasetable first"));
   if (s == NULL || *s == '\n')
      synerr(_("phase name parameter expected"));
   for (i = 0; i < p->nphases; i++)
      if (!strcasecmp(p->tpname + i * NAMELEN, s))
	 return i;
   synerr(_("phase name unrecognised"));
   return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
