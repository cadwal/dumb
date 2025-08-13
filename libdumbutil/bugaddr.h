/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/bugaddr.h: Where to report bugs.
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

#ifndef LIBDUMBUTIL_BUGADDR_H
#define LIBDUMBUTIL_BUGADDR_H

#include <stdio.h>		/* FILE */

extern const char *program_bug_address;

/* Perhaps the function and the pointer should be in separate files.
 * Then programs could redefine the pointer instead of assigning to
 * it.  */
void print_bugaddr_message(FILE *dest);

#endif /* LIBDUMBUTIL_BUGADDR_H */

// Local Variables:
// c-basic-offset: 3
// End:
