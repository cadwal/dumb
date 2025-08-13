/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/sound.c: Prototypes for functions each sound driver must provide.
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

#ifndef SOUND_H
#define SOUND_H

#include "libdumbutil/fixed.h"
#include "libdumbutil/confdef.h"

/* use 0 for these params to get defaults */
void init_sound(int speed);
void reset_sound(void);

void set_sound_volume(fixed vol);

typedef struct {
   fixed lfvol, rfvol, lbvol, rbvol;
   int bend;
} SoundBalance;

#define FLAT_BALANCE {FIXED_ONE,FIXED_ONE,FIXED_ONE,FIXED_ONE,0}

/* queue a sound to be played */
void play_sound(const unsigned char *sample, int len,
		SoundBalance balance, int myspeed);

/* stop playing every sound now */
void purge_sound(void);

/* gets called frequently */
void poll_sound(void);

extern ConfItem sound_conf[];

#endif

// Local Variables:
// c-basic-offset: 3
// End:
