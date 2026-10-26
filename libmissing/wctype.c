/* DUMB: A Doom-like 3D game engine.
 *
 * libmissing/wctype.c: <wctype.h> case functions for systems lacking
 * them (e.g. DJGPP 2.05).  Only ASCII and Latin-1 are handled, which
 * covers the characters DUMB's fonts and translations use.
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

#include <wctype.h>

#include "libmissing.h"

/* If towlower() exists, this file is not compiled at all.
   So don't add #ifndef HAVE_TOWLOWER.  */

int
iswupper(wint_t wc)
{
   return ((wc >= 0x41 && wc <= 0x5a)		/* A-Z */
	   || (wc >= 0xc0 && wc <= 0xde && wc != 0xd7)); /* not times */
}

int
iswlower(wint_t wc)
{
   return ((wc >= 0x61 && wc <= 0x7a)		/* a-z */
	   || (wc >= 0xdf && wc <= 0xff && wc != 0xf7)); /* not divide */
}

wint_t
towlower(wint_t wc)
{
   return iswupper(wc) ? wc + 0x20 : wc;
}

wint_t
towupper(wint_t wc)
{
   /* U+00DF sharp s and U+00FF y diaeresis have no Latin-1 capital.  */
   if (iswlower(wc) && wc != 0xdf && wc != 0xff)
      return wc - 0x20;
   return wc;
}

// Local Variables:
// c-basic-offset: 3
// End:
