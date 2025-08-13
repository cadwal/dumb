/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/gettablestruct.h: Format of GETTABLE lump.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef GETTABLESTRUCT_H
#define GETTABLESTRUCT_H

#include "libdumbutil/endiantypes.h"

/* The GETTABLE lump consists of blocks of varying length.  Each block
   begins with a header which says how long the block is, including
   the header.  When DUMB starts up, it scans the lump and makes a
   list of the blocks' locations.

   Each block can also contain other information referred to by the
   header.  The location of each such piece of information is saved as
   a byte count from the beginning of the block.  Thus the meaning of
   a block does not depend on its position in the lump, and lumps can
   be concatenated.

   The length of each block should be a multiple of 8.  */

typedef struct {
   LE_int32 block_length;
   LE_flags32 flags;
   LE_int32 xo, yo;		/* IconPos <x:integer> <y:integer> */
   LE_int16 initial;		/* Initial <integer> */
   LE_int16 defaultmax;		/* DefaultMaximum <integer> */
   LE_int16 backpackmax;	/* WithBackpack <integer>, or ==defaultmax */
   LE_int16 key;
   LE_int16 ammotype;  		/* Ammo <gettable> <integer> */
   LE_int16 ammocount;
   LE_int16 special;		/* see ../doom/dumbdefs.pt */
   LE_int16 bogotype;
   LE_int16 weaponnumber;	/* which key to press; 0=none, 1...10 */
   LE_int16 replaceweapon;	/* gettable number to replace/disable */
   LE_int16 decay;
   char iconname[10];
   char iconanim, _spare;
   LE_int16 timing;
   LE_int16 usesound;
} Gettable;

#define GK_XCENTERICON 0x0001
#define GK_YCENTERICON 0x0002
#define GK_WEPSELECT   0x0008	/* can use as weapon */
#define GK_SPESELECT   0x0010	/* can use as special item */
#define GK_ONEMAPONLY  0x0040	/* lose this type between levels */
#define GK_REVANIM     0x0080

#endif

// Local Variables:
// c-basic-offset: 3
// End:
