/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/intset.c: Possibly unsorted set of integers.
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include "libdumbutil/safem.h"
#include "libdumbutil/intset.h"

typedef int SetContent;

void
intset_init(struct intset *ints)
{
   ints->data = NULL;
   ints->allocated = ints->used = 0;
#ifndef NDEBUG
   ints->readonly = 0;
#endif
}

void
intset_init_copy(struct intset *ints, const struct intset *src)
{
   if (src->used == 0)
      intset_init(ints);
   else {
      ints->allocated = ints->used = src->used;
      ints->data = safe_malloc(ints->allocated * sizeof(SetContent));
      memcpy(ints->data, src->data, ints->allocated * sizeof(SetContent));
   }
}

void
intset_fini(struct intset *ints)
{
   /* the assert macro has its own #ifndef NDEBUG */
   assert(ints->readonly == 0);
   if (ints->allocated)
      safe_free(ints->data);
}

void
intset_clear(struct intset *ints)
{
   assert(ints->readonly == 0);
   ints->used = 0;
   /* keep the table in case it's needed again */
}

void
intset_prealloc(struct intset *ints, size_t atleast)
{
   size_t want_alloc = ints->used + atleast;
   if (ints->allocated >= want_alloc)
      return;
   assert(ints->readonly == 0);
   if (want_alloc < 8)
      want_alloc = 8;
   else if (want_alloc < 1024) {
      unsigned pow2 = 8;
      while (pow2 < want_alloc)
	 pow2 *= 2;
      want_alloc = pow2;
   } else if (want_alloc % 1024)
      want_alloc = (want_alloc | 1023) + 1;	/* round to next multiple of 1024 */
   ints->data = safe_realloc(ints->data, want_alloc * sizeof(SetContent));
   ints->allocated = want_alloc;
}

void
intset_add(struct intset *ints, int value)
{
   if (intset_contains(ints, value))
      return;
   assert(ints->readonly == 0);
   intset_prealloc(ints, 1);
   ints->data[ints->used++] = value;
}

void
intset_remove(struct intset *ints, int value)
{
   SetContent *end = &ints->data[ints->used];
   SetContent *p;
   for (p = ints->data; p < end; p++) {
      if (*p == value) {	/* found it */
	 /* The assertion is here so that it isn't tested if the value
	  * wasn't in the list in the first place.  */
	 assert(ints->readonly == 0);
	 if (p != end - 1)
	    *p = *(end - 1);	/* move last item to the hole */
	 ints->used--;
	 return;
      }
   }
   /* not found */
}

int
intset_contains(const struct intset *ints, int value)
{
   const SetContent *end = &ints->data[ints->used];
   const SetContent *p;
   for (p = ints->data; p < end; p++) {
      if (*p == value)		/* found it */
	 return 1;
   }
   /* not found */
   return 0;
}

void
intset_foreach(const struct intset *ints,
	       void (*fn) (int value, void *clientdata),
	       void *clientdata)
{
   const SetContent *end = &ints->data[ints->used];
   const SetContent *p;
#ifndef NDEBUG
   ints->readonly++;
#endif
   for (p = ints->data; p < end; p++)
      fn(*p, clientdata);
#ifndef NDEBUG
   ints->readonly--;
#endif
}

void
intset_foreach_delete(const struct intset *ints,
		      void (*fn) (int value, void *clientdata),
		      void *clientdata)
{
   assert(ints->readonly == 0);
#ifndef NDEBUG
   ints->readonly++;
#endif
   while (ints->used > 0)
      fn(ints->data[--ints->used], clientdata);
#ifndef NDEBUG
   ints->readonly--;
#endif
}

// Local Variables:
// c-basic-offset: 3
// End:
