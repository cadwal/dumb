/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/prothing.c: ProtoThings.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "prothing.h"
#include "protoinwad.h"

static const ThingPhase *phases = NULL;
static ProtoThing *protos = NULL;
static int nphases = 0, nprotos = 0;
static LumpNum phase_ln;

static const ThingPhase **phaseidx = NULL;
static int nphaseidx = 0;

static int
count_phases(int id)
{
   const ThingPhase *ph = find_first_thingphase(id), *end = phases + nphases;
   int i = 0;
   if (ph == NULL)
      return 0;
   do {
      i++;
      ph++;
   } while (ph < end && (ph->id == 0 || ph->id == id));
   return i;
}

static Texture **
get_sprite_table(ProtoThing *pr)
{
   if (pr->sprites == NULL)
      pr->sprites = (Texture **) safe_calloc(8 * count_phases(pr->phase_id),
					     sizeof(Texture *));
   return pr->sprites;
}

Texture *
find_phase_sprite(ProtoThing *proto, int phase, char rot)
{
   const int idx = phase * 8 + (rot - '1');
   Texture **sp = get_sprite_table(proto);
   if (rot < '1' || rot > '8')
      logprintf(LOG_FATAL, 'O', _("wanted rot %c for sprite %s phase %d?\?\?"),
		rot, proto->sprite, phase);
   if (sp[idx] == NULL) {
      const ThingPhase *tp = find_thingphase(proto->phase_id, phase);
      const char *sprite_name;
      if (tp->sprite[0] != '\0')
	 sprite_name = tp->sprite;
      else if (proto->sprite[0] != '\0')
	 sprite_name = proto->sprite;
      else {
	 /* There is no sprite for this proto!
	  *
	  * Perhaps there should be a statically allocated empty
	  * texture so this could return &empty_texture and save it in
	  * sp[idx] to speed up the next call.  */
	 return NULL;
      }
      sp[idx] = get_sprite(sprite_name, tp->spr_phase, rot, NULL);
      if (sp[idx] == NULL)
	 logprintf(LOG_ERROR, 'O',
		   _("Can't find sprite %s, ph=%d (%c) rot=%c"),
		   proto->sprite, phase, tp->spr_phase, rot);
   }
   return sp[idx];
}

const ThingPhase *
find_first_thingphase(int id)
{
   int start, stop, t;
   if (phases == NULL)
      init_prothings();
   start = 0;
   stop = nphaseidx - 1;
   do {
      t = (start + stop) / 2;
      if (!phaseidx[t])
	 return NULL;		/* should this ever happen??? */
      if (phaseidx[t]->id == id)
	 break;
      if (start >= stop)
	 logprintf(LOG_FATAL, 'O', _("Can't find ThingPhase id=%d"), id);
      if (phaseidx[t]->id < id)
	 start = t + 1;
      else
	 stop = t - 1;
   } while (1);
   return phaseidx[t];
}

ProtoThing *
find_protothing(int id)
{
   int start, stop, t;
   if (protos == NULL)
      init_prothings();
   start = 0;
   stop = nprotos - 1;
   do {
      t = (start + stop) / 2;
      if (protos[t].id == id)
	 break;
      if (start >= stop)
	 return NULL;
      if (protos[t].id < id)
	 start = t + 1;
      else
	 stop = t - 1;
   } while (1);
   return &protos[t];
}

void
reset_prothings(void)
{
   if (phases)
      free_lump(phase_ln);
   if (protos) {
      int i;
      for (i = 0; i < nprotos; i++) {
	 if (protos[i].sprites)
	    safe_free(protos[i].sprites);
	 if (protos[i].gets)
	    safe_free(protos[i].gets);
	 if (protos[i].firstpickupmsg)
	    safe_free(protos[i].firstpickupmsg);
	 if (protos[i].pickupmsg)
	    safe_free(protos[i].pickupmsg);
	 if (protos[i].ignoremsg)
	    safe_free(protos[i].ignoremsg);
      }
      safe_free(protos);
   }
   if (phaseidx)
      safe_free(phaseidx);
   phaseidx = NULL; nphaseidx = 0;
   phases   = NULL; nphases   = 0;
   protos   = NULL; nprotos   = 0;
}

void
init_prothings(void)
{
   int i;
   LumpNum proto_ln;
   if (protos)
      reset_prothings();
   phase_ln = safe_lookup_lump("PHASES", NULL, NULL, LOG_FATAL);
   proto_ln = safe_lookup_lump("PROTOS", NULL, NULL, LOG_FATAL);
   /* load phases from wad */
   if (LUMPNUM_OK(phase_ln)) {
      nphases = get_lump_len(phase_ln) / sizeof(ThingPhase);
      phases = (const ThingPhase *) load_lump(phase_ln);
   }
   /* build phase index */
   phaseidx = (const ThingPhase **)
      safe_malloc(nphases * sizeof(const ThingPhase *));
   for (nphaseidx = i = 0; i < nphases; i++)
      if (phases[i].id > 0)
	 phaseidx[nphaseidx++] = phases + i;
   phaseidx = (const ThingPhase **)
      safe_realloc(phaseidx, nphaseidx * sizeof(const ThingPhase *));
   /* load protos from wad */
   if (LUMPNUM_OK(proto_ln)) {
      nprotos = decode_protoinwad_array(&protos,
					((const ProtoThing_inwad *)
					 load_lump(proto_ln)),
					get_lump_len(proto_ln));
      logprintf(LOG_INFO, 'O', _("init %d protothings"), nprotos);
      free_lump(proto_ln);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
