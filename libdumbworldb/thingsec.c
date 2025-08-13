/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/thingsec.c: Links between things and sectors.
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

#include "libdumbutil/safem.h"
#include "private.h"

void
_dwdb_del_thingsec(struct dwdb_level *lev,
		   struct dwdb_thingsec *ts)
{
   /* Note: this never affects lev->things[ts->thing].center_sector.  */
   if (lev->sectors[ts->sector].things == ts)
      lev->sectors[ts->sector].things = ts->next_thing;
   if (lev->things[ts->thing].sectors == ts)
      lev->things[ts->thing].sectors = ts->next_sector;
   if (ts->next_thing != NULL)
      ts->next_thing->prev_thing = ts->prev_thing;
   if (ts->prev_thing != NULL)
      ts->prev_thing->next_thing = ts->next_thing;
   if (ts->next_sector != NULL)
      ts->next_sector->prev_sector = ts->prev_sector;
   if (ts->prev_sector != NULL)
      ts->prev_sector->next_sector = ts->next_sector;
   safe_free(ts);
}

void
_dwdb_new_thingsec(struct dwdb_level *lev,
		   unsigned thing, unsigned sector)
{
   struct dwdb_thingsec *ts
      = (struct dwdb_thingsec *) safe_malloc(sizeof(struct dwdb_thingsec));
   ts->thing = thing;
   ts->sector = sector;
   ts->next_thing = lev->sectors[sector].things;
   if (ts->next_thing != NULL)
      ts->next_thing->prev_thing = ts;
   ts->prev_thing = NULL;
   lev->sectors[sector].things = ts;
   ts->next_sector = lev->things[thing].sectors;
   if (ts->next_sector != NULL)
      ts->next_sector->prev_sector = ts;
   ts->prev_sector = NULL;
   lev->things[thing].sectors = ts;
}

// Local Variables:
// c-basic-offset: 3
// End:
