/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/difficulty.c: Setting the difficulty level.
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <stdlib.h>

#include "dumbworldb.h"

static int thing_should_exist(const struct dwdb_thing *, int difficulty);

void
dwdb_set_difficulty(struct dwdb_level *lev, int difficulty)
{
   int ind;
   ind = lev->thing_alloc.first_used;
   while (ind != -1) {
      int next = lev->things[ind].next_in_chain;
      if (!thing_should_exist(&lev->things[ind], difficulty))
	 dwdb_del_thing(lev, ind);
      ind = next;
   }
}

static int
thing_should_exist(const struct dwdb_thing *thing, int difficulty)
{
   switch (difficulty) {
   case 1:
   case 2:
      return (thing->flags & DWDB_TF_IN_SKILL12) != 0;
   case 3:
      return (thing->flags & DWDB_TF_IN_SKILL3) != 0;
   case 4:
   case 5:
      return (thing->flags & DWDB_TF_IN_SKILL45) != 0;
   default:
      abort();
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
