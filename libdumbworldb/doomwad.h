/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/doomwad.h: Data structures in Doom WAD files.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef DOOMWAD_H
#define DOOMWAD_H

/* These structures are in ../libdumbwad/wadstruct.h too, with
 * different names.  I copied them here because I feel this is where
 * they belong.  - Kalle */

/* This file is meant to be used by libdumbworldb only.  */

#include "libdumbutil/endiantypes.h"

/* Note -- the WAD header and directory structures are *not* here.  */

struct doom_vertex {
   LE_int16 x, y;
};

struct doom_sector {
   LE_int16 floor, ceiling;
   char ftexture[8];
   char ctexture[8];
   LE_int16 light;
   LE_int16 type;
   LE_int16 tag;
};

struct doom_sidedef {
   LE_int16 xoffset, yoffset;
   char utexture[8];
   char ltexture[8];
   char texture[8];
   LE_int16 sector;
};

struct doom_linedef {
   LE_int16 ver2, ver1;
   LE_flags16 flags;
   LE_int16 type;
   LE_int16 tag;
   LE_int16 side[2];
};

/* doom_linedef::flags */
#define DOOM_LF_IMPASSABLE  0x0001	/* impassable */
#define DOOM_LF_MIMPASSABLE 0x0002	/* impassable for monsters */
#define DOOM_LF_TWOSIDED    0x0004	/* two-sided */
#define DOOM_LF_UPUNPEGGED  0x0008	/* upper texture unpegged */
#define DOOM_LF_LOUNPEGGED  0x0010	/* lower/middle texture unpegged */
#define DOOM_LF_SECRET      0x0020	/* appears as impassable on the automap */
#define DOOM_LF_MBLKSOUND   0x0040	/* block sound for monsters */
#define DOOM_LF_NOMAP       0x0080	/* never draw on the automap */
#define DOOM_LF_FORCEMAP    0x0100	/* always draw on the automap */
/* DUMB-specific */
#define DOOM_LF_POSTER      0x0200	/* scale texture to fill area */

struct doom_thing {
   LE_int16 x, y;
   LE_int16 angle;		/* degrees */
   LE_int16 type;
   LE_flags16 flags;
};

/* doom_thing::flags */
#define DOOM_TF_SKILL12   0x0001	/* appears in skill levels 1-2 */
#define DOOM_TF_SKILL3    0x0002	/* appears in skill level 3 */
#define DOOM_TF_SKILL45   0x0004	/* appears in skill levels 4-5 */
#define DOOM_TF_DEAF	  0x0008	/* sleeps until hurt or sees player */
#define DOOM_TF_MULTIONLY 0x0010	/* appears in multiplayer games only */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
