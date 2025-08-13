/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/globals.h: Global definitions.
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

#ifndef GLOBALS_H
#define GLOBALS_H

#define NAMELEN 32
#define ALLOC_BLK 64

/* If true, translate name tokens to strings where needed.  Set with
   --fake-strings.  */
extern int fake_strings_flag;

#endif

// Local Variables:
// c-basic-offset: 3
// End:
