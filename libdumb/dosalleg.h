/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/dosalleg.h: Including Allegro 4 in the MS-DOS drivers.
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

#ifndef LIBDUMB_DOSALLEG_H
#define LIBDUMB_DOSALLEG_H

/* Include this instead of <allegro.h>, and before any DUMB header.  */

/* Allegro 4.2 declares its inline functions "extern __inline__".
   With GCC's default (C99) inline semantics that emits an external
   definition in every file including <allegro.h>, so linking several
   of them fails.  Make them static instead.  */
#ifndef __cplusplus
#define AL_INLINE(type, name, args, code)	\
   static __inline__ type name args;		\
   static __inline__ type name args code
#endif

#include <allegro.h>

/* Allegro has its own fixed type (int32_t, i.e. long in DJGPP) and
   fixed-point functions.  Rename DUMB's in files that use both.  */
#define fixed fixed_dumb
#define fixmul fixmul_dumb
#define fixdiv fixdiv_dumb
#define fixsin fixsin_dumb
#define fixcos fixcos_dumb
#define fixtan fixtan_dumb

#endif /* LIBDUMB_DOSALLEG_H */

// Local Variables:
// c-basic-offset: 3
// End:
