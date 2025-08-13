/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/align.h: Data alignment for WADs and such.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMBUTIL_ALIGN_H
#define LIBDUMBUTIL_ALIGN_H

#define ALIGN_LUMP      8	/* all lumps */
#define ALIGN_TEXTURE1  8	/* entries in TEXTURE{1,2} lumps */

/* rounds up to next aligned position */
#define ALIGN(x,alignment) \
   (((x) + (alignment) - 1) / (alignment) * (alignment))

#define IS_ALIGNED(x,alignment) ((x) % (alignment) == 0)

#endif /* LIBDUMBUTIL_ALIGN_H */

// Local Variables:
// c-basic-offset: 3
// End:
