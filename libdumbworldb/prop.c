/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/prop.c: Properties of a level.
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
#include "dumbworldb.h"
#include "private.h"

void
dwdb_set_name(struct dwdb_level *lev, const char *name)
{
   if (lev->name)
      safe_free(lev->name);
   lev->name = safe_strdup(name);
   _dwdb_after_ch(lev, DWDB_OT_LEVEL, -1);
}

void
dwdb_set_longname(struct dwdb_level *lev, const char *longname)
{
   if (lev->longname)
      safe_free(lev->longname);
   lev->longname = safe_strdup(longname);
   _dwdb_after_ch(lev, DWDB_OT_LEVEL, -1);
}

// Local Variables:
// c-basic-offset: 3
// End:
