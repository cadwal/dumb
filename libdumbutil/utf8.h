/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/utf8.h: UTF-8 multibyte decoding.
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

#ifndef LIBDUMBUTIL_UTF8_H
#define LIBDUMBUTIL_UTF8_H

/* <wchar.h> defines mbstate_t and mbrtowc(), but the standard doesn't
   say what character set and encoding those use.  So here we have a
   version specifically for UTF-8.  */

#include <wchar.h>

typedef struct {
   wchar_t sum;
   int bytes_left;
} utf8_mbstate_t;

size_t utf8_mbrtowc(wchar_t *restrict pwc,
		    const char *restrict s, size_t n,
		    utf8_mbstate_t *restrict ps);

#endif /* LIBDUMBUTIL_UTF8_H */

// Local Variables:
// c-basic-offset: 3
// End:
