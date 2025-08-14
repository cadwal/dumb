/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/lumpver.h: Headers showing what format a lump is in.
 * Copyright (C) 1999 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMB_LUMPVER_H
#define LIBDUMB_LUMPVER_H

#include "libdumbutil/endiantypes.h"

#define LUMPVER_MAGIC_LEN 12

typedef struct {
   char magic[LUMPVER_MAGIC_LEN]; /* memcmp, memcpy */
   int version;
} LumpVer;

typedef struct {
   char magic[LUMPVER_MAGIC_LEN];
   LE_int32 version;
} LumpVer_inwad;

void set_lumpver(LumpVer_inwad *dest, const LumpVer *src);
int is_lumpver_ok(const LumpVer_inwad *inwad, const LumpVer *ok,
		  const char *lumpname_for_messages);

#endif /* LIBDUMB_LUMPVER_H */

// Local Variables:
// c-basic-offset: 3
// End:
