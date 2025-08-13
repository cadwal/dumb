/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/game.h: Input processing.
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

#ifndef GAME_H

#include "input.h"
#include "levdata.h"

/* mostly what's declared here gets defined in dumb.c
   it will be moved to game.c when the main loop gets tidied up */

void game_message(int player, const char *fmt,...)
     __attribute__((format(printf, 2, 3)));

void game_want_newlvl(int secret);
void game_want_quit(int really);

/* how often to check whether monsters wake up */
#define WAKE_TICKS (1000/MSEC_PER_TICK)

/* player stuff */

extern ConfItem playconf[];
void slave_input(LevData *ld, const PlayerInput *in, int tickspassed);
void process_input(LevData *ld, const PlayerInput *in, int tickspassed,
		   int pl);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
