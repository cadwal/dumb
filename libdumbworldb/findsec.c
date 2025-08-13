/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/findsec.c: Which sector is this point in?
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

#include "libdumbutil/fixed.h"
#include "dumbworldb.h"

/* This is derived from findsector() in dumb/things.c.  */
int
dwdb_find_sector_2d(const struct dwdb_level *lev, int x, int y)
{
   int closest_side = -1;
   fixed closest = FIXED_MAX;
   int lineind;

   for (lineind = lev->line_alloc.first_used;
	lineind != -1;
	lineind = lev->lines[lineind].next_in_chain) {
      const struct dwdb_line *line = &lev->lines[lineind];
      fixed y1 = lev->vertices[line->ver1].y;
      fixed y2 = lev->vertices[line->ver2].y;

      if ((y >= y1 && y < y2) || (y >= y2 && y < y1)) {
	 fixed dist;
	 const struct dwdb_vertex *v1 = &lev->vertices[line->ver1];
	 const struct dwdb_vertex *v2 = &lev->vertices[line->ver2];

	 /* Kludge to avoid division overflows for near-horizontal
	  * walls.  */
	 if (FIXED_ABS(y1 - y2) < FIXED_EPSILON)
	    dist = MIN(v1->x - x, v2->x - x);
	 else
	    dist = fixmul(fixdiv(v1->x - v2->x, v1->y - v2->y),
			  y - v1->y) + v1->x - x;
	 if (dist > FIXED_ZERO && dist < closest) {
	    closest = dist;
	    if (y1 < y2)
	       closest_side = line->side[0];
	    else
	       closest_side = line->side[1];
	 }
      }				/* if y between y1,y2 */
   }				/* for lineind */
   if (closest_side >= 0)
      return lev->sides[closest_side].sector;
   else
      return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
