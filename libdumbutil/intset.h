/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/intset.h: Possibly unsorted set of integers.
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

#ifndef DUMBUTIL_INTSET_H
#define DUMBUTIL_INTSET_H

#include <stddef.h>

struct intset {
   /* private: */
   int *data;
   size_t allocated, used;
#ifndef NDEBUG
   /* intset_foreach() increments this on entry and decrements it on
    * exit.  Functions that change the set then assert(readonly==0).  */
   int readonly;
#endif
};

void intset_init(struct intset *);
void intset_init_copy(struct intset *, const struct intset *);
void intset_fini(struct intset *);

void intset_clear(struct intset *);
void intset_prealloc(struct intset *, size_t atleast);

void intset_add(struct intset *, int);	/* doesn't add if already there */
void intset_remove(struct intset *, int);	/* no error if not found */

/* Return 1 if VALUE is in the set and 0 if not.  */
int intset_contains(const struct intset *, int value);

/* Call FN once for each entry in the set.  FN must not change the set.  */
void intset_foreach(const struct intset *,
		    void (*fn) (int, void *clientdata),
		    void *clientdata);

/* Like intset_foreach() but removes each entry from the list just
 * before calling FN for it.  This makes it safe for FN to call other
 * functions which may try to remove the entry.  However, removing
 * other entries is still not safe.  */
void intset_foreach_remove(struct intset *,
			   void (*fn) (int, void *clientdata),
			   void *clientdata);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
