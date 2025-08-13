/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/strgrow.h: Buffer for a growing string
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

/* struct strgrow is a buffer where characters can be added one at a
   time.  The string is at all times readable via the str member, but
   not automatically null-terminated -- you can add the terminator
   with strgrow_grow() if you need it.

   An efficient way of using strgrow is to have one strgrow object
   where strings are built one at a time.  When a string is complete,
   call strgrow_strdup_clear() to add the null terminator, strdup it
   and clear the buffer.  This way, only one optimal-size malloc() is
   needed for most strings.  */

struct strgrow
{
   /* public: */
   char *str;			/* not automatically null-terminated */
   /* private: */
   size_t current, max;
};

/* Constructor */
void strgrow_init(struct strgrow *);
/* Destructor */
void strgrow_fini(struct strgrow *);
/* Add a character to the end (may be '\0') */
void strgrow_grow(struct strgrow *, char);
/* Clear the buffer but leave it allocated */
void strgrow_clear(struct strgrow *);
/* Add '\0', strdup and clear */
char *strgrow_strdup_clear(struct strgrow *);

#endif /* LIBDUMBUTIL_STRGROW_H */

// Local Variables:
// c-basic-offset: 3
// End:
