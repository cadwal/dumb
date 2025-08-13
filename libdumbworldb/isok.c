/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/isok.c: Checking whether an index is valid.
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

#include "dumbworldb.h"

int
dwdb_isok_vertex(const struct dwdb_level *lev, int ind)
{
   if (ind < 0)
      return 0;
   if ((unsigned) ind >= lev->vertex_alloc.inited)
      return 0;
   return 1;			/* FIXME: deleted?  */
}

int
dwdb_isok_sector(const struct dwdb_level *lev, int ind)
{
   if (ind < 0)
      return 0;
   if ((unsigned) ind >= lev->sector_alloc.inited)
      return 0;
   return 1;			/* FIXME: deleted?  */
}

int
dwdb_isok_side(const struct dwdb_level *lev, int ind)
{
   if (ind < 0)
      return 0;
   if ((unsigned) ind >= lev->side_alloc.inited)
      return 0;
   return 1;			/* FIXME: deleted?  */
}

int
dwdb_isok_line(const struct dwdb_level *lev, int ind)
{
   if (ind < 0)
      return 0;
   if ((unsigned) ind >= lev->line_alloc.inited)
      return 0;
   return 1;			/* FIXME: deleted?  */
}

int
dwdb_isok_thing(const struct dwdb_level *lev, int ind)
{
   if (ind < 0)
      return 0;
   if ((unsigned) ind >= lev->thing_alloc.inited)
      return 0;
   return 1;			/* FIXME: deleted?  */
}

// Local Variables:
// c-basic-offset: 3
// End:
