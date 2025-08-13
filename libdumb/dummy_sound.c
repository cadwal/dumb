/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/dummy_sound.c: Dummy sound driver.
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

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "sound.h"

void
init_sound(int speed)
{
}

/* stop playing now, and throw away anything you were about to play */
void
purge_sound(void)
{
}

/* do all of the above, and shut down the sound device, clean up, etc */
void
reset_sound(void)
{
   purge_sound();
}

void
set_sound_volume(fixed vol)
{
}

void
play_sound(const unsigned char *sample, int len,
	   SoundBalance balance, int myspeed)
{
}

void
poll_sound(void)
{
}

ConfItem sound_conf[] =
{
   CONFITEM_END
};

// Local Variables:
// c-basic-offset: 3
// End:
