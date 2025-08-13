/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/timer.h: Measuring real time.
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

#ifndef TIMER_H
#define TIMER_H

/* how long a tick is, in micro-seconds */
#define MSEC_PER_TICK 10

#define USEC_PER_TICK (MSEC_PER_TICK*1000)

void init_timer(void);
void reset_timer(void);
int read_timer(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
