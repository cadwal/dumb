/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/initfini.c: Construction and destruction of dumbworldb objects.
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

#include <stddef.h>

#include "libdumbutil/safem.h"
#include "dumbworldb.h"

static void init_alloc(struct dwdb_alloc *);

void
dwdb_init(struct dwdb_level *lev)
{
   lev->name = NULL;
   lev->longname = NULL;
   lev->vertices = NULL;
   init_alloc(&lev->vertex_alloc);
   lev->sides = NULL;
   init_alloc(&lev->side_alloc);
   lev->lines = NULL;
   init_alloc(&lev->line_alloc);
   lev->sectors = NULL;
   init_alloc(&lev->sector_alloc);
   lev->things = NULL;
   init_alloc(&lev->thing_alloc);
   lev->first_observer = NULL;
}

static void
init_alloc(struct dwdb_alloc *alloc)
{
   alloc->alloced = 0;
   alloc->inited = 0;
   alloc->free = 0;
   alloc->first_free = -1;
   alloc->first_used = -1;
}

void
dwdb_fini(struct dwdb_level *lev)
{
   while (lev->first_observer)
      dwdb_del_observer(lev, lev->first_observer);
   if (lev->name)
      safe_free(lev->name);
   if (lev->longname)
      safe_free(lev->longname);
   if (lev->vertex_alloc.alloced)
      safe_free(lev->vertices);
   if (lev->side_alloc.alloced)
      safe_free(lev->sides);
   if (lev->line_alloc.alloced)
      safe_free(lev->lines);
   if (lev->sector_alloc.alloced)
      safe_free(lev->sectors);
   if (lev->thing_alloc.alloced)
      safe_free(lev->things);
   /* Could clear the variables here but why bother.  */
}

void dwdb_init_copy(struct dwdb_level *lev, const struct dwdb_level *source);
/* not yet implemented */

// Local Variables:
// c-basic-offset: 3
// End:
