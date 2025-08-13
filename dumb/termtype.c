/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/termtype.c: Computing goals for numeric sector parameters (light etc.)
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
#include <limits.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "levdyn.h"
#include "linetype.h"

static fixed
lc(const LevData *ld, int sector, int adj, int next)
{
   int wall;
   fixed f = FIXED_MAX;
   if (!adj)
      f = ldsectord(ld)[sector].ceiling;
   for (wall = 0; wall < ldnlines(ld); wall++) {
      fixed g = FIXED_MAX;
      int front = ldline(ld)[wall].side[0];
      int back = ldline(ld)[wall].side[1];
      if (back < 0)
	 continue;
      front = ldside(ld)[front].sector;
      back = ldside(ld)[back].sector;
      if (front == sector)
	 g = ldsectord(ld)[back].ceiling;
      else if (back == sector)
	 g = ldsectord(ld)[front].ceiling;
      if (next && g <= ldsectord(ld)[sector].ceiling)
	 continue;
      if (g < f)
	 f = g;
   }
   return f;
}
static fixed
lf(const LevData *ld, int sector, int adj, int next)
{
   int wall;
   fixed f = FIXED_MAX;
   if (!adj)
      f = ldsectord(ld)[sector].floor;
   for (wall = 0; wall < ldnlines(ld); wall++) {
      fixed g = FIXED_MAX;
      int front = ldline(ld)[wall].side[0];
      int back = ldline(ld)[wall].side[1];
      if (back < 0)
	 continue;
      front = ldside(ld)[front].sector;
      back = ldside(ld)[back].sector;
      if (front == sector)
	 g = ldsectord(ld)[back].floor;
      else if (back == sector)
	 g = ldsectord(ld)[front].floor;
      if (next && g <= ldsectord(ld)[sector].floor)
	 continue;
      if (g < f)
	 f = g;
   }
   return f;
}

static fixed
hc(const LevData *ld, int sector, int adj, int next)
{
   int wall;
   fixed f = FIXED_MIN;
   if (!adj)
      f = ldsectord(ld)[sector].ceiling;
   for (wall = 0; wall < ldnlines(ld); wall++) {
      fixed g = FIXED_MIN;
      int front = ldline(ld)[wall].side[0];
      int back = ldline(ld)[wall].side[1];
      if (back < 0)
	 continue;
      front = ldside(ld)[front].sector;
      back = ldside(ld)[back].sector;
      if (front == sector)
	 g = ldsectord(ld)[back].ceiling;
      else if (back == sector)
	 g = ldsectord(ld)[front].ceiling;
      if (next && g >= ldsectord(ld)[sector].ceiling)
	 continue;
      if (g > f)
	 f = g;
   }
   return f;
}
static fixed
hf(const LevData *ld, int sector, int adj, int next)
{
   int wall;
   fixed f = FIXED_MIN;
   if (!adj)
      f = ldsectord(ld)[sector].floor;
   for (wall = 0; wall < ldnlines(ld); wall++) {
      fixed g = FIXED_MIN;
      int front = ldline(ld)[wall].side[0];
      int back = ldline(ld)[wall].side[1];
      if (back < 0)
	 continue;
      front = ldside(ld)[front].sector;
      back = ldside(ld)[back].sector;
      if (front == sector)
	 g = ldsectord(ld)[back].floor;
      else if (back == sector)
	 g = ldsectord(ld)[front].floor;
      if (next && g >= ldsectord(ld)[sector].floor)
	 continue;
      if (g > f)
	 f = g;
   }
   return f;
}

static fixed
darkest(const LevData *ld, int sector, int adj)
{
   int wall;
   fixed f = FIXED_MIN;
   if (!adj)
      f = ldsectord(ld)[sector].dark;
   for (wall = 0; wall < ldnlines(ld); wall++) {
      fixed g = FIXED_MIN;
      int front = ldline(ld)[wall].side[0];
      int back = ldline(ld)[wall].side[1];
      if (back < 0)
	 continue;
      front = ldside(ld)[front].sector;
      back = ldside(ld)[back].sector;
      if (front == sector)
	 g = ldsectord(ld)[back].dark;
      else if (back == sector)
	 g = ldsectord(ld)[front].dark;
      if (g > f)
	 f = g;
   }
   return f;
}
static fixed
lightest(const LevData *ld, int sector, int adj)
{
   int wall;
   fixed f = FIXED_MAX;
   if (!adj)
      f = ldsectord(ld)[sector].dark;
   for (wall = 0; wall < ldnlines(ld); wall++) {
      fixed g = FIXED_MAX;
      int front = ldline(ld)[wall].side[0];
      int back = ldline(ld)[wall].side[1];
      if (back < 0)
	 continue;
      front = ldside(ld)[front].sector;
      back = ldside(ld)[back].sector;
      if (front == sector)
	 g = ldsectord(ld)[back].dark;
      else if (back == sector)
	 g = ldsectord(ld)[front].dark;
      if (g < f)
	 f = g;
   }
   return f;
}


fixed
get_term_type(const LevData *ld, LT_TermType ltt, int sector)
{
   const SectorDyn *sd = ldsectord(ld) + sector;
   switch (ltt) {
   case (NoTermType):
      return 0;
   case (TTOne):
      return FIXED_ONE;
   case (TTZero):
      return FIXED_ZERO;
   case (Ceiling):
      return sd->ceiling;
   case (Floor):
      return sd->floor;
   case (OrigCeiling):
      return ldsector(ld)[sector].ceiling << 12;
   case (OrigFloor):
      return ldsector(ld)[sector].floor << 12;
      /* ceilings and floors */
   case (LowestCeiling):
      return lc(ld, sector, 0, 0);
   case (LowestAdjacentCeiling):
      return lc(ld, sector, 1, 0);
   case (LowestFloor):
      return lf(ld, sector, 0, 0);
   case (LowestAdjacentFloor):
      return lf(ld, sector, 1, 0);
   case (HighestCeiling):
      return hc(ld, sector, 0, 0);
   case (HighestAdjacentCeiling):
      return hc(ld, sector, 1, 0);
   case (HighestFloor):
      return hf(ld, sector, 0, 0);
   case (HighestAdjacentFloor):
      return hf(ld, sector, 1, 0);
      /* light */
   case (DarkestSector):
      return darkest(ld, sector, 0);
   case (DarkestAdjacentSector):
      return darkest(ld, sector, 1);
   case (LightestSector):
      return lightest(ld, sector, 0);
   case (LightestAdjacentSector):
      return lightest(ld, sector, 1);
      /* next highest/lowest */
      /* that nHEF is implemented by calling LEF is *not* a typo */
   case (NextHighestCeiling):
      return lc(ld, sector, 1, 1);
   case (NextLowestCeiling):
      return hc(ld, sector, 1, 1);
   case (NextHighestFloor):
      return lf(ld, sector, 1, 1);
   case (NextLowestFloor):
      return hf(ld, sector, 1, 1);
   default:
      logprintf(LOG_ERROR, 'M', _("unimplemented LT_TermType %d"),
		(int) ltt);
   }
   return sd->floor;
}

// Local Variables:
// c-basic-offset: 3
// End:
