/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/utf8.c: UTF-8 multibyte decoding.
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

#include <errno.h>

#include "utf8.h"

static utf8_mbstate_t default_state;

size_t
utf8_mbrtowc(wchar_t *restrict pwc,
	     const char *restrict s, size_t n,
	     utf8_mbstate_t *restrict ps)
{
   size_t used = 0;
   if (ps == NULL)
      ps = &default_state;
   if (n == 0)
      return (size_t) -2;
   if (ps->bytes_left == 0) {
      const unsigned char lead_byte = *s++;
      used++;
      if (lead_byte < 0x80) {
	 *pwc = lead_byte;
	 if (*pwc == (wchar_t) 0)
	    return 0;		/* and return to the initial state */
	 else
	    return used;	/* used == 1 */
      } else if (lead_byte < 0xC0) {
	 errno = EILSEQ;
	 return (size_t) -1;
	 /* The conversion state is undefined.  */
      } else if (lead_byte < 0xE0) {
	 ps->sum = lead_byte & 0x1F;
	 ps->bytes_left = 1;	/* 5 + 1*6 = 11 bits */
      } else if (lead_byte < 0xF0) {
	 ps->sum = lead_byte & 0x0F;
	 ps->bytes_left = 2;	/* 4 + 2*6 = 16 bits */
      } else if (lead_byte < 0xF8) {
	 ps->sum = lead_byte & 0x07;
	 ps->bytes_left = 3;	/* 3 + 3*6 = 21 bits */
      } else if (lead_byte < 0xFC) {
	 ps->sum = lead_byte & 0x03;
	 ps->bytes_left = 4;	/* 2 + 4*6 = 26 bits */
      } else if (lead_byte < 0xFE) {
	 ps->sum = lead_byte & 0x01;
	 ps->bytes_left = 5;	/* 1 + 5*6 = 31 bits */
      } else {
	 /* 0x80...0xBF, 0xFE, 0xFF */
	 errno = EILSEQ;
	 return (size_t) -1;
	 /* The conversion state is undefined.  */
      }
   }
   while (++used <= n) {
      /* Here, ps->bytes_left != 0.
	 If it becomes 0, the function returns.  */
      const unsigned char cont_byte = *s++;
      if ((cont_byte & 0xC0) == 0x80) {
	 ps->sum = (ps->sum << 6) | (cont_byte & 0x3F);
	 if (--ps->bytes_left == 0) {
	    /* Character complete.  */
	    *pwc = ps->sum;
	    return used;
	    /* ps->sum is reset when the next multibyte character
	       begins.  */
	 }
      } else {
	 /* Invalid multibyte character.  */
	 errno = EILSEQ;
	 return (size_t) -1;
	 /* The conversion state is undefined.  */
      }
   }
   /* It may be a valid multibyte character, but more bytes are
      needed.  */
   return (size_t) -2;
}

// Local Variables:
// c-basic-offset: 3
// End:
