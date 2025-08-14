/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/lumpver.c: Headers showing what format a lump is in.
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

#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "lumpver.h"

void
set_lumpver(LumpVer_inwad *dest, const LumpVer *src)
{
   memcpy(dest->magic, src->magic, LUMPVER_MAGIC_LEN);
   dest->version = src->version;
}

int
is_lumpver_ok(const LumpVer_inwad *inwad, const LumpVer *ok,
	      const char *lumpname)
{
   if (memcmp(inwad->magic, ok->magic, LUMPVER_MAGIC_LEN) != 0) {
      if (lumpname != NULL)
	 logprintf('F', LOG_ERROR,
		   _("Lump %s is of wrong type"), lumpname);
      return 0;			/* not OK */
   }
   if (inwad->version != ok->version) {
      if (lumpname != NULL)
	 logprintf('F', LOG_ERROR,
		   _("Lump %s has format version %d; should be %d"),
		   lumpname, (int) inwad->version, (int) ok->version);
      return 0;			/* not OK */
   }
   return 1;			/* OK */
}

// Local Variables:
// c-basic-offset: 3
// End:
