/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/dummyvtbl.c: Dummy vtbl for passive things.
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

static void dummy_init_fini(struct dwdb_level *, unsigned ind);

const struct dwdb_thing_vtbl dwdb_dummy_thing_vtbl =
{
   /* Once again, a GCC extension is used
    * (although I believe this'll be in the next standard too)
    */
   .init = dummy_init_fini,
   .fini = dummy_init_fini
};

static void
dummy_init_fini(struct dwdb_level *lev, unsigned ind)
{
   /* nothing here */
}

// Local Variables:
// c-basic-offset: 3
// End:
