/* DUMB: A Doom-like 3D game engine.
 *
 * libmissing/libmissing.h: Functions which the C library may lack.
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

#ifndef LIBMISSING_LIBMISSING_H
#define LIBMISSING_LIBMISSING_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_STRNLEN
size_t strnlen (const char *string, size_t maxlen);
#endif

#ifndef HAVE_HSTRERROR
const char *hstrerror(int err_num);
#endif

#ifndef HAVE_SNPRINTF
int vsnprintf(char *s, size_t size, const char *format, va_list ap);
int snprintf(char *s, size_t size, const char *format, ...);
#endif   

#ifdef __cplusplus
}
#endif

#endif /* LIBMISSING_LIBMISSING_H */
