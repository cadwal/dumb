/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/private.h: Definitions private to libdumbworldb.
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

#ifndef PRIVATE_H
#define PRIVATE_H

#include "dumbworldb.h"

void _dwdb_del_thingsec(struct dwdb_level *, struct dwdb_thingsec *);
void _dwdb_new_thingsec(struct dwdb_level *,
			unsigned thing, unsigned sector);

/* Ensures that side SIDEIND is not in any line.  Notifies observers
 * if appropriate.  */
void _dwdb_detach_side(struct dwdb_level *, int sideind);

/* These call the corresponding functions in all observers.  */
void _dwdb_after_new(struct dwdb_level *, enum dwdb_object_type,
		     int index);
void _dwdb_after_ch(struct dwdb_level *, enum dwdb_object_type,
		    int index);
void _dwdb_before_del(struct dwdb_level *, enum dwdb_object_type,
		      int index);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
