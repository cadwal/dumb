/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/protoinwad.h: Encoding and decoding ProtoThings in WAD.
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

#ifndef LIBDUMB_PROTOINWAD_H
#define LIBDUMB_PROTOINWAD_H

#include "libdumb/prothingstruct.h"

/* This opaque structure is defined in protoinwad.c.  */
typedef struct ProtoThing_inwad ProtoThing_inwad;

/* These functions translate ProtoThings between internal and external
   formats.  They depend only on "safem" and "log".  */


/* Functions for parsing the PROTOS lump */

/* Return true iff there is a ProtoThing block in the memory area
   beginning at DATA and having length LENGTH.  */
int is_protoinwad(const ProtoThing_inwad *data, size_t length);

/* Advance *DATAP to point to the next ProtoThing block.  Subtract the
   length of the previous block from *LENGTHP.  Call this only if
   is_protoinwad(*DATAP, *LENGTHP) already returned true.  */
void next_protoinwad(const ProtoThing_inwad **datap, size_t *lengthp);

/* Decode the in-WAD form *SRC to the in-memory form *DEST.  Always
   overwrite all the members of *DEST.  */
void decode_protoinwad(ProtoThing *dest, const ProtoThing_inwad *src);

/* Count the ProtoThings in the memory area beginning at SRC and
   having length LENGTH, and return how many there are.  If DESTP is
   non-null, allocate enough memory for the ProtoThings, decode them
   and save the address of the array in *DESTP.  The original value of
   *DESTP doesn't matter.  */
size_t decode_protoinwad_array(ProtoThing **destp,
			       const ProtoThing_inwad *src,
			       size_t length);


/* Functions for creating the PROTOS lump */

/* Encode the in-memory form *SRC to the in-WAD form, allocating
   enough memory.  Return the address of the block and save its length
   in *LENGTHP.  */
ProtoThing_inwad *encode_protoinwad(const ProtoThing *src, size_t *lengthp);

#endif /* LIBDUMB_PROTOINWAD_H */

// Local Variables:
// c-basic-offset: 3
// End:
