/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/dyncode.h: Encoding game data for network.
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

#ifndef DYNCODE_H
#define DYNCODE_H

#include "libdumbutil/endiantypes.h"
#include "levdata.h"

typedef struct {
   LE_int16 uas, las, mas, _spare;
} SideCode;

typedef struct {
   LE_int32 floor, ceiling;
   LE_int16 fas, cas;
} SectorCode;

typedef struct {
   LE_int32 x, y, z, angle, elev;
   LE_int16 proto, phase, owner, sector;
   LE_int16 hits, armour, tmpinv, tmpgod;
   LE_int16 flags;
} ThingCode;

#define DECL_ENCODE(struc) DYN_ENCODE_FUNC(struc##_encode)
#define DECL_DECODE(struc) DYN_DECODE_FUNC(struc##_decode)

#define DECL_DYNCODE_FUNCS(struc) DECL_ENCODE(struc);DECL_DECODE(struc)

DECL_DYNCODE_FUNCS(Vertex);
DECL_DYNCODE_FUNCS(Side);
DECL_DYNCODE_FUNCS(Line);
DECL_DYNCODE_FUNCS(Sector);
DECL_DYNCODE_FUNCS(Thing);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
