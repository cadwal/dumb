/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/gettablestruct.h: In-memory format of Gettables.
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

#include "libdumbwad/wadio.h"	/* LUMPNAMELEN */

/* The Gettables have different format in memory and in the WAD file.
   DUMB converts them when it starts up.  The in-WAD format is defined
   in gettableinwad.c -- nothing else should need it.  The in-memory
   structure is defined below.  */

typedef struct {
   int gtid;			/* Gettable ID */
   int change;			/* >0 gives, <0 takes */
   int maximum;			/* 0 means use default */
} Gets;

typedef struct {
   unsigned flags;
   int xo, yo;			/* IconPos <x:integer> <y:integer> */
   int initial;			/* Initial <integer> */
   int defaultmax;		/* DefaultMaximum <integer> */
   int backpackmax;		/* WithBackPack <integer>, or ==defaultmax */
   int key;
   int ngets;			/* Ammo|Gets <gettable> <integer> */
   Gets *gets;
   int special;			/* see ../dumb/dumbdefs.pt */
   int bogotype;
   int powered_bogotype;
   int weaponnumber;		/* (NYI) which key to press; 0=none, 1...10 */
   int replaceweapon;		/* (NYI) gettable number to replace/disable */
   int decay;
   char iconname[LUMPNAMELEN+1];
   char iconanim;
   int timing;
   int usesound;
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
