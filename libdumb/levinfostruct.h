/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/levinfostruct.h: Format of LEVINFO lump.
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

#ifndef LEVINFOSTRUCT_H
#define LEVINFOSTRUCT_H

#include "libdumbutil/endiantypes.h"

#define LEVLONGNAME_LEN 64

typedef struct {
   char name[10];
   char music[10];
   char sky[10];
   LE_int16 next, secret, _spare;
   LE_flags32 flags;
   char longname[LEVLONGNAME_LEN];
} LevInfo;

#define LI_START  0x0001	/* OK to start playing here */
#define LI_END    0x0002	/* endgame at end of level */
#define LI_WAFFLE 0x0004	/* not a level, but a message to show to player */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
