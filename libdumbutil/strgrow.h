/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/strgrow.h: Buffer for a growing string
 * Copyright (C) 1998 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMBUTIL_STRGROW_H
#define LIBDUMBUTIL_STRGROW_H

#include <stddef.h>

struct strgrow
{
   char *str;
   size_t current, max;
};

void strgrow_init(struct strgrow *);
void strgrow_fini(struct strgrow *);
void strgrow_grow(struct strgrow *, char);
void strgrow_clear(struct strgrow *);

#endif /* LIBDUMBUTIL_STRGROW_H */

// Local Variables:
// c-basic-offset: 3
// End:
