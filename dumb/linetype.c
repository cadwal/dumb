/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/linetype.c: Linetypes.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "linetype.h"

struct LineTypeOffsets *ltofs;	/* declared in linetype.h */

static LumpNum linetype_ln = BAD_LUMPNUM;
static const LineType *lt = NULL;
static int nlts = 0;

static LumpNum sectortype_ln = BAD_LUMPNUM;
static const SectorType *st = NULL;
static int nsts = 0;

struct LineTypeScroller {
   int ltind;
   int xofs, yofs;
};

static struct LineTypeScroller *scrollers;
static int nscrollers;

static void init_ltscroll(void);

void
init_linetypes(void)
{
   /* load linetypes */
   linetype_ln = lookup_lump("LINETYPE", NULL, NULL);
   lt = NULL;
   nlts = 0;
   if (LUMPNUM_OK(linetype_ln)) {
      lt = (const LineType *) load_lump(linetype_ln);
      nlts = get_lump_len(linetype_ln) / sizeof(LineType);
      ltofs = (struct LineTypeOffsets *)
	 safe_calloc(sizeof(struct LineTypeOffsets), nlts);
      init_ltscroll();
      logprintf(LOG_INFO, 'M',
		_("Loaded %d linetypes, of which %d scrolling"),
		nlts, nscrollers);
   } else
      logprintf(LOG_ERROR, 'M', _("Failed to find LINETYPE in wad"));
   /* now sectortypes */
   sectortype_ln = lookup_lump("SECTTYPE", NULL, NULL);
   st = NULL;
   nsts = 0;
   if (LUMPNUM_OK(sectortype_ln)) {
      st = (const SectorType *) load_lump(sectortype_ln);
      nsts = get_lump_len(sectortype_ln) / sizeof(SectorType);
      logprintf(LOG_INFO, 'M', _("Loaded %d sectortypes"), nsts);
   } else
      logprintf(LOG_ERROR, 'M', _("Failed to find SECTTYPE in wad"));
}

static void
init_ltscroll(void)
{
   int i;
   struct LineTypeScroller *p;
   /* first count how many scrolling linetypes there are */
   nscrollers = 0;
   for (i = 0; i < nlts; i++) {
      if (lt[i].scrolldx || lt[i].scrolldy)
	 nscrollers++;
   }
   /* then allocate a table and put them in it */
   scrollers = (struct LineTypeScroller *)
      safe_malloc(nscrollers * sizeof(struct LineTypeScroller));
   for (i = 0, p = scrollers; i < nlts; i++) {
      if (lt[i].scrolldx || lt[i].scrolldy) {
	 p->ltind = i;
	 p->xofs = 0;
	 p->yofs = 0;
	 p++;
      }
   }
}

void
reset_linetypes(void)
{
   if (LUMPNUM_OK(linetype_ln) && lt)
      free_lump(linetype_ln);
   if (LUMPNUM_OK(sectortype_ln) && st)
      free_lump(sectortype_ln);
   sectortype_ln = linetype_ln = BAD_LUMPNUM;
   st = lt = NULL;
   nsts = nlts = 0;
}

#ifdef FAKE_LINETYPE_LUMP
static const LineType *
fake_linetype(int id)
{
   static LineType mylt;
   switch (id) {
   case 1:
   case 26:
   case 28:
   case 27:
   case 31:
   case 32:
   case 33:
   case 34:
   case 47:
   case 117:
   case 118:
      /* return a manual door-ish linetype structure */
      memset(&mylt, 0, sizeof(LineType));
      mylt.flags = LT_REPEATABLE |
	  LT_ALLOW_PLAYER | LT_ALLOW_NONPLAYER |
	  LT_ON_THUMPED | LT_ON_ACTIVATED;
      mylt.action[0].flags = LTA_MANUAL;
      mylt.action[0].lumptype = ML_SECTOR;
      mylt.action[0].eventtype = ME_CEILING;
      mylt.action[0].term_type[0] = LowestAdjacentCeiling;
      mylt.action[0].term_offset[0] = -(4 << 12);
      mylt.action[0].speed[0] = 1 << 11;
      mylt.action[1].flags = LTA_MANUAL;
      mylt.action[1].lumptype = ML_SECTOR;
      mylt.action[1].eventtype = ME_CEILING;
      mylt.action[1].delay = 128;
      mylt.action[1].term_type[0] = Floor;
      mylt.action[1].speed[0] = -(1 << 11);
      return &mylt;
   }				/* switch */
   return NULL;
}
#endif

const LineType *
lookup_linetype(int id)
{
#ifdef FAKE_LINETYPE_LUMP
   if (lt == NULL)
      return fake_linetype(id);
#endif
   if (id > 0 && id <= nlts)
      return lt + id;
   else
      return NULL;
}

const SectorType *
lookup_sectortype(int id)
{
   if (id > 0 && id <= nsts)
      return st + id;
   else
      return NULL;
}

void
linetype_ticks_passed(int tickspassed)
{
   /* scroll all scrolling linetypes */
   int i;
   for (i = 0; i < nscrollers; i++) {
      int ltind = scrollers[i].ltind;
      scrollers[i].xofs += lt[ltind].scrolldx * tickspassed;
      scrollers[i].yofs += lt[ltind].scrolldy * tickspassed;
      ltofs[ltind].xofs = scrollers[i].xofs >> 12;
      ltofs[ltind].yofs = scrollers[i].yofs >> 12;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
