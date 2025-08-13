/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/hexenwad.h: Data structures in Hexen WAD files.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1997 by Josh Parsons <josh@schlick.anu.edu.au>
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

#ifndef HEXENWAD_H
#define HEXENWAD_H

/* The data is from The Official Hexen Technical Specs v0.9 by Ben
 * Morris <bmorris@islandnet.com>.
 *
 * NOTE: Although this file exists in libdumbworldb, nothing #includes
 * it, and DUMB doesn't really support Hexen levels.  */

/* This file is meant to be used by libdumbworldb only.  */

#include "libdumbutil/endiantypes.h"

struct hexen_vertex {
   LE_int16 x, y;
};

struct hexen_sector {
   LE_int16 floor, ceiling;
   char ftexture[8];
   char ctexture[8];
   LE_int16 light;
   LE_int16 type;
   LE_int16 tag;
};

struct hexen_sidedef {
   LE_int16 xoffset, yoffset;
   char utexture[8];
   char ltexture[8];
   char texture[8];
   LE_int16 sector;
};

struct hexen_linedef {
   LE_int16 ver1, ver2;
   LE_flags16 flags;
   unsigned char special;
   unsigned char args[5];
   LE_int16 side[2];
};

/* hexen_linedef::flags */
#define HEXEN_LF_IMPASSABLE  0x0001	/* impassable */
#define HEXEN_LF_MIMPASSABLE 0x0002	/* impassable for monsters */
#define HEXEN_LF_TWOSIDED    0x0004	/* two-sided */
#define HEXEN_LF_UPUNPEGGED  0x0008	/* upper texture unpegged */
#define HEXEN_LF_LOUNPEGGED  0x0010	/* lower/middle texture unpegged */
#define HEXEN_LF_SECRET      0x0020	/* appears as impassable on the automap */
#define HEXEN_LF_MBLKSOUND   0x0040	/* block sound for monsters */
#define HEXEN_LF_NOMAP       0x0080	/* never draw on the automap */
#define HEXEN_LF_FORCEMAP    0x0100	/* always draw on the automap */
#define HEXEN_LF_REPEATABLE  0x0200	/* special action is repeatable */
#define HEXEN_LF_WHEN_MASK   0x1C00	/* when is the line activated? */
#define HEXEN_LFW_PLCROSS     0x0000	/* player crosses the line */
#define HEXEN_LFW_PLUSE       0x0400	/* player uses the line */
#define HEXEN_LFW_MCROSS      0x0800	/* monster crosses the line */
#define HEXEN_LFW_PRDAMAGE    0x0C00	/* projectile hits the wall */
#define HEXEN_LFW_PLTHUMP     0x1000	/* player pushes the wall */
#define HEXEN_LFW_PRCROSS     0x1400	/* projectile crosses the line */

struct hexen_thing {
   LE_int16 tag;
   LE_int16 x, y;
   LE_int16 altitude;		/* measured from floor */
   LE_int16 angle;		/* degrees */
   LE_int16 type;
   LE_flags16 flags;
   unsigned char special;
   unsigned char args[5];
};

/* hexen_thing::flags */
#define HEXEN_TF_SKILL12  0x0001	/* appears in skill levels 1-2 */
#define HEXEN_TF_SKILL3   0x0002	/* appears in skill level 3 */
#define HEXEN_TF_SKILL45  0x0004	/* appears in skill levels 4-5 */
#define HEXEN_TF_DEAF     0x0008	/* sleeps until hurt or sees player */
#define HEXEN_TF_DORMANT  0x0010	/* sleeps until specially activated */
#define HEXEN_TF_FIGHTER  0x0020	/* appears for the Fighter class */
#define HEXEN_TF_CLERIC   0x0040	/* appears for the Cleric class */
#define HEXEN_TF_MAGE     0x0080	/* appears for the Mage class */
#define HEXEN_TF_SINGLE   0x0100	/* appears in single-player games */
#define HEXEN_TF_COOP     0x0200	/* appears in cooperative games */
#define HEXEN_TF_DM       0x0400	/* appears in DeathMatch games */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
