/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/dos_timer.c: Special timer functions for MS-DOS.
 * Copyright (C) 1998 by Ulf Axelsson <ulf@ore.ims.se>
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

#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <allegro.h>

#include "timer.h"

static volatile int tick_cnt;

void
user_tick(void)
{
   tick_cnt++;
}
END_OF_FUNCTION(user_tick)

void
init_timer(void)
{
   install_timer();
   reset_timer();

   LOCK_VARIABLE(tick_cnt);
   LOCK_FUNCTION(user_tick);
   install_int(user_tick, MSEC_PER_TICK);
}

void
reset_timer(void)
{
   tick_cnt = 0;
}

int
read_timer(void)
{
   return tick_cnt;
}

// Local Variables:
// c-basic-offset: 3
// End:
