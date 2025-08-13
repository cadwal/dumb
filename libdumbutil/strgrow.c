/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/strgrow.c: Buffer for a growing string
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

#include <config.h>

#include "libdumbutil/dumb-nls.h"

#include "strgrow.h"
#include "libdumbutil/safem.h"

/* Maybe we should use obstacks instead? */

void
strgrow_init(struct strgrow *sg)
{
   sg->current = 0;
   sg->max = 64;
   sg->str = (char *) safe_malloc(sg->max);
}

void
strgrow_fini(struct strgrow *sg)
{
   safe_free(sg->str);
}

void
strgrow_grow(struct strgrow *sg, char c)
{
   if (sg->current == sg->max) {
      sg->max *= 2;
      sg->str = (char *) safe_realloc(sg->str, sg->max);
   }
   sg->str[sg->current++] = c;
}

void
strgrow_clear(struct strgrow *sg)
{
   sg->current = 0;
}

char *
strgrow_strdup_clear(struct strgrow *sg)
{
   char *s;
   strgrow_grow(sg, '\0');
   s = safe_strdup(sg->str);
   strgrow_clear(sg);
   return s;
}

// Local Variables:
// c-basic-offset: 3
// End:
