/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/detach.c: Detaching sides from lines.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include "private.h"

void
_dwdb_detach_side(struct dwdb_level *lev, int sideind)
{
   int lineind = lev->sides[sideind].line;
   int line_changed = 0;
   if (lineind == -1)
      return;			/* already detached */
   if (lev->lines[lineind].side[0] == sideind) {
      lev->lines[lineind].side[0] = -1;
      line_changed = 1;
   }
   if (lev->lines[lineind].side[1] == sideind) {
      lev->lines[lineind].side[1] = -1;
      line_changed = 1;
   }
   if (line_changed)
      _dwdb_after_ch(lev, DWDB_OT_LINE, lineind);
   lev->sides[sideind].line = -1;
   /* This did not affect which sector the side is in, so chains don't
    * have to be changed.  */
   _dwdb_after_ch(lev, DWDB_OT_SIDE, sideind);
}

// Local Variables:
// c-basic-offset: 3
// End:
