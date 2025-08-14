/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbwad/wadstruct.h: Structures in a Doom WAD file.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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

#ifndef LIBDUMBWAD_WADSTRUCT_H
#define LIBDUMBWAD_WADSTRUCT_H

#include "libdumbutil/endiantypes.h"

/* I grepped for "8" and replaced it with this where it seemed
   appropriate.  But there are still null-terminated buffers which
   have some other length like char[10].  Those should be changed to
   [LUMPNAMELEN+1], but be careful not to break anything.

   Also... we might some day check where this number is mandated by
   Doom WADs, and make those use DOOM_LUMPNAMELEN or some such.  Then
   we could increase the number elsewhere and see what happens.
       - 1999-04-04 Kalle Niemitalo <tosi@stekt.oulu.fi> */
#define LUMPNAMELEN 8

/* WAD structures */

typedef struct {
   char sig[4];
   LE_uint32 nlumps;
   LE_uint32 diroffset;
} WadHeader;

typedef struct {
   LE_uint32 offset;
   LE_uint32 size;
   char name[LUMPNAMELEN];
} WadDirEntry;

#endif

// Local Variables:
// c-basic-offset: 3
// End:
