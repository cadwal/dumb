/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/gettableinwad.h: Encoding and decoding Gettables in WAD.
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

#ifndef LIBDUMB_GETTABLEINWAD_H
#define LIBDUMB_GETTABLEINWAD_H

#include "libdumb/gettablestruct.h"

/* These opaque structures are defined in gettableinwad.c.  */
typedef struct Gettable_inwad Gettable_inwad;
typedef struct Gettables_inwad Gettables_inwad;
typedef struct Gets_inwad Gets_inwad;

/* These functions translate Gettables between internal and external
   formats.  They depend only on "safem" and "log".  */


/* Functions for parsing the GETTABLE lump */

/* Check that the memory area beginning at LUMPDATA and having length
   *LENGTHP contains a valid Gettables_inwad header.  Return the
   address of the data after the header and subtract the length of the
   header from *LENGTHP.  Return NULL if the header is invalid.
   LUMPNAME is used for error messages unless NULL.  */
const Gettable_inwad *first_gettableinwad(const Gettables_inwad *lumpdata,
					  size_t *lengthp,
					  const char *lumpname);

/* Return true iff there is a Gettable block in the memory area
   beginning at DATA and having length LENGTH.  */
int is_gettableinwad(const Gettable_inwad *data, size_t length);

/* Advance *DATAP to point to the next Gettable block.  Subtract the
   length of the previous block from *LENGTHP.  Call this only if
   is_gettableinwad(*DATAP, *LENGTHP) already returned true.  */
void next_gettableinwad(const Gettable_inwad **datap, size_t *lengthp);

/* Decode the in-WAD form *SRC to the in-memory form *DEST.  Always
   overwrite all the members of *DEST.  */
void decode_gettableinwad(Gettable *dest, const Gettable_inwad *src);

/* Count the Gettables in the memory area beginning at HDR and having
   length LENGTH, and return how many there are.  If DESTP is
   non-null, allocate enough memory for the Gettables, decode them and
   save the address of the array in *DESTP.  The original value of
   *DESTP doesn't matter.  LUMPNAME is used for error messages unless
   NULL.  */
size_t decode_gettableinwad_array(Gettable **destp,
				  const Gettables_inwad *hdr,
				  size_t length,
				  const char *lumpname);


/* Functions for creating the GETTABLE lump */

/* Allocate memory for a Gettables_inwad header, fill it up, save its
   length in *LENGTHP and return its address.

   This function might get more parameters in the future.  */
Gettables_inwad *begin_gettablesinwad(size_t *lengthp);

/* Encode the in-memory form *SRC to the in-WAD form, allocating
   enough memory.  Return the address of the block and save its length
   in *LENGTHP.  */
Gettable_inwad *encode_gettableinwad(const Gettable *src, size_t *lengthp);


/* Functions for Gets structures */

/* Decode NGETS Gets structures from the memory area beginning at
   GETSINWAD, allocating enough memory.  Return the address of the
   decoded array, or NULL if NGETS==0.  */
Gets *decode_getsinwad_array(const Gets_inwad getsinwad[], size_t ngets);

/* Encode NGETS Gets structures from the array GETS, allocating enough
   memory.  Return the address of the in-WAD format array, and save
   its length (in bytes) in *LENGTHP.  */
Gets_inwad *encode_getsinwad_array(const Gets gets[], size_t ngets,
				   size_t *lengthp);

#endif /* LIBDUMB_GETTABLEINWAD_H */

// Local Variables:
// c-basic-offset: 3
// End:
