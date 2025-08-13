/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/lineprop.c: Properties of a line.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#include "dumbworldb.h"
#include "private.h"

void
dwdb_set_line_side(struct dwdb_level *lev, unsigned lineind,
		   unsigned fore_back, int sideind)
{
   int old_side = lev->lines[lineind].side[fore_back];
   if (old_side == sideind)
      return;			/* no change */
   if (old_side != -1)
      _dwdb_detach_side(lev, old_side);
   if (sideind != -1)
      _dwdb_detach_side(lev, sideind);
   lev->lines[lineind].side[fore_back] = sideind;
   _dwdb_after_ch(lev, DWDB_OT_LINE, lineind);
   if (sideind != -1) {
      lev->sides[sideind].line = lineind;
      _dwdb_after_ch(lev, DWDB_OT_SIDE, sideind);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
