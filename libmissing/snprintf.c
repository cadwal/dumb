/* DUMB: A Doom-like 3D game engine.
 *
 * libmissing/snprintf.c: Stupid snprintf() replacement.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <stdarg.h>
#include <stdio.h>

#include "libdumbutil/dumb-nls.h"

#include "libmissing.h"
#include "libdumbutil/safem.h"

/* This is an arbitrary limit.  Get a real C library, and you won't
   need this stupid replacement.  */
#define SNPRINTF_MAX 1000

/* And this makes the functions non-reentrant.  It is not automatic
   because the return address of vsprintf() could easily get
   clobbered.  */
static char buf[SNPRINTF_MAX + 1];

int
vsnprintf(char *s, size_t size, const char *format, va_list ap)
{
   size_t len;
   len = vsprintf(buf, format, ap);
   if (len > SNPRINTF_MAX) {
      /* No way to know what else has been overwritten.  */
      abort();
   }
   if (size > 0)
      memcpy(s, buf, (len < size) ? len+1 : size);
   return len;
}

int
snprintf(char *s, size_t size, const char *format, ...)
{
   va_list ap;
   int ret;
   va_start(ap, format);
   ret = vsnprintf(s, size, format, ap);
   va_end(ap);
   return ret;
}

// Local Variables:
// c-basic-offset: 3
// End:
