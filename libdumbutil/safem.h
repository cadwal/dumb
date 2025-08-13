/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/safem.h: "Safe" (checked) memory allocation.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1994 by Chris Laurel
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

#ifndef SAFEM_H
#define SAFEM_H

#include <stddef.h>		/* size_t */

/* all of these logfatal() on error */

void *safe_realloc(void *p, size_t s);
void *safe_calloc(size_t s1, size_t s2);
void *safe_malloc(size_t size);
void safe_free(void *ptr);
char *safe_strdup(const char *s);

#ifndef HAVE_MMAP

#define safe_vcalloc(s) safe_calloc(s,1)
#define safe_vmalloc(s) safe_malloc(s)
#define safe_vfree(p,s) safe_free(p)

#else  /* HAVE_MMAP */

/*
   vcalloc gets zeroed memory by copy-on-write mmapping /dev/zero
   "great if you can get it" for big arrays that we want initialised.
 */
void *safe_vcalloc(size_t size);
#define safe_vmalloc safe_vcalloc
void safe_vfree(void *ptr, size_t size);

#endif /* HAVE_MMAP */

#endif /* !SAFEM_H */

// Local Variables:
// c-basic-offset: 3
// End:
