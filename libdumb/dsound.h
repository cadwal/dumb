/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/dsound.h: Driver-independent sound functions.
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

#ifndef DSOUND_H

#include "libdumbutil/fixed.h"
#include "libdumbutil/endiantypes.h"
#include "libdumb/view.h"

#define MAX_REDIR_SOUNDS 7

typedef struct {
   char lumpname[10];		/* FIXME: why 10? */
   LE_int16 bend_range, chance;
   LE_int16 nredir, bend_const;
   LE_int16 redir[MAX_REDIR_SOUNDS];
} SoundEnt;

void init_dsound(void);
void reset_dsound(void);

void dsound_setview(View *v);

void play_dsound_local(int sound, fixed x, fixed y, fixed radius);
void play_dsound(int sound, fixed x, fixed y, fixed radius);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
