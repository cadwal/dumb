/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/exitcode.h: Values passed to exit()
 * Copyright (C) 1998 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMBUTIL_EXITCODE_H
#define LIBDUMBUTIL_EXITCODE_H

#include <stdlib.h>

#if (EXIT_SUCCESS==0) && (EXIT_FAILURE==1)

#define DUMB_EXIT_OTHER          1
#define DUMB_EXIT_INVALID_DATA   1
#define DUMB_EXIT_INVALID_ARGS   2
#define DUMB_EXIT_FOPEN_FAIL     2 /* or XOpenDisplay() or XLoadQueryFont() */
#define DUMB_EXIT_IO_ERROR       3
#define DUMB_EXIT_INTERNAL_LIMIT 3
#define DUMB_EXIT_OUT_OF_MEMORY  3

#else  /* EXIT_SUCCESS!=0 || EXIT_FAILURE!=1 */

/* This is some strange system... play it safe.  */

#define DUMB_EXIT_OTHER          EXIT_FAILURE
#define DUMB_EXIT_INVALID_DATA   EXIT_FAILURE
#define DUMB_EXIT_INVALID_ARGS   EXIT_FAILURE
#define DUMB_EXIT_FOPEN_FAIL     EXIT_FAILURE
#define DUMB_EXIT_IO_ERROR       EXIT_FAILURE
#define DUMB_EXIT_INTERNAL_LIMIT EXIT_FAILURE
#define DUMB_EXIT_OUT_OF_MEMORY  EXIT_FAILURE

#endif /* EXIT_SUCCESS!=0 || EXIT_FAILURE!=1 */

#endif /* LIBDUMBUTIL_EXITCODE_H */

// Local Variables:
// c-basic-offset: 3
// End:
