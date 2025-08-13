/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/safem.c: "Safe" (checked) memory allocation.
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1998 by Josh Parsons <josh@schlick.anu.edu.au>
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

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_MMAP
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif /* HAVE_MMAP */

#include "libdumbutil/dumb-nls.h"

#include "log.h"
#include "safem.h"

#ifdef HAVE_MMAP

/* In Solaris, this flag means that the allocation should succeed even
 * if there actually isn't that much memory. */
#ifndef MAP_NORESERVE
/* The operating system doesn't support this flag, so make it a no-op.  */
#define MAP_NORESERVE 0
#endif

/* For some reason, Linux doesn't define MAP_FAILED even though the
 * man page for mmap(2) mentions it?  */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#define DEV_ZERO "/dev/zero"

void *
safe_vcalloc(size_t size)
{
   void *p;
#ifdef MAP_ANONYMOUS
   /* this should get us an all-zero area */
   p = mmap(NULL, size, PROT_READ | PROT_WRITE,
	    MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
   if (p == MAP_FAILED)
      logfatal('S', _("mmapping %lu zero bytes: %s"),
	       (unsigned long) size, strerror(errno));
#else  /* !MAP_ANONYMOUS */
   static int devzerofd = -1;
   if (devzerofd < 0) {
      devzerofd = open(DEV_ZERO, O_RDONLY);
      if (devzerofd < 0)
	 logfatal('S', "%s: %s", DEV_ZERO, strerror(errno));
   }
   p = mmap(NULL, size, PROT_READ | PROT_WRITE,
	    MAP_PRIVATE | MAP_NORESERVE, devzerofd, 0);
   if (p == MAP_FAILED)
      logfatal('S', _("%s: mmapping %lu bytes: %s"),
	       DEV_ZERO, (unsigned long) size, strerror(errno));
#endif /* !MAP_ANONYMOUS */
   return p;
}

void
safe_vfree(void *ptr, size_t size)
{
   if (munmap(ptr, size))
      logfatal('S', _("munmapping %lu bytes @ %p: %s"),
	       (unsigned long) size, ptr,
	       strerror(errno));
}

#endif /* HAVE_MMAP */

void *
safe_realloc(void *p1, size_t size)
{
   if (p1 == NULL) {
      /* Perhaps some pre-ANSI C library can't handle realloc(NULL,
       * size).  Maybe the configure script should check for that but
       * this takes just a few bytes of code anyway.  */
      return safe_malloc(size);
   } else if (size == 0) {
      /* As above...
       * If this wasn't handled separately, you'd have to check for
       * size==0 below when p==NULL.  */
      free(p1);			/* FIXME: maybe safe_free()? */
      return NULL;
   } else {
      void *p = realloc(p1, size);
      if (p == NULL)
	 logfatal('S', _("out of memory reallocating %lu bytes (old=%p)"),
		  (unsigned long) size, p1);
      return p;
   }
}

void *
safe_malloc(size_t l)
{
   void *p = malloc(l);
   /* malloc(0) is valid and may return NULL.  If it returns something
    * else, the only things one can do with the returned pointer is
    * realloc it or free it.
    *
    * BTW, the C++ allocation new char[0] can't return NULL. */
   if (p == NULL && l != 0) {
      /* If we're out of memory, does gettext() work?  Perhaps this
       * string should be pretranslated and saved in a variable.
       *
       * I asked Ulrich Drepper and he told me:
       * 1) the current gettext implementation doesn't need more
       *    memory for translation after it has loaded the catalog;
       * 2) they are planning more complex schemes (compression?)
       *    which might need more memory at runtime.  But even then,
       *    gettext would just return the string untranslated if it
       *    ran out of memory.  */
      logfatal('S', _("out of memory allocating %lu bytes"),
	       (unsigned long) l);
   }
   return p;
}

void *
safe_calloc(size_t l, size_t c)
{
   void *p = calloc(l, c);
   if (p == NULL && l * c != 0)
      logfatal('S', _("out of memory allocating %lu bytes"),
	       (unsigned long) l * c);
   return p;
}

char *
safe_strdup(const char *s)
{
   char *p = (char *) safe_malloc(strlen(s) + 1);
   strcpy(p, s);
   return p;
}

void
safe_free(void *p)
{
   /* FIXME: free(NULL) should be a no-op */
   if (p == NULL)
      logprintf(LOG_ERROR, 'S', _("Attempt to free NULL."));
   /*else logprintf(LOG_DEBUG, 'S', _("Freeing memory at %p"), p); */
   /* *((char *)p)=0; *//* force a crash here, if p is bogus */
   free(p);
}

// Local Variables:
// c-basic-offset: 3
// End:
