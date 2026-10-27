/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/timer.c: Measuring real time.
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

#ifdef _WIN32
# include <windows.h>
#else
# include <time.h>
#endif

#include "timer.h"

/* read_timer() never returns more than this, like the old setitimer()
   version, which stopped counting after a second.  It keeps the game
   from jumping ahead after a long pause.  */
#define MAX_TICKS 99

/* Microseconds from an arbitrary fixed point; never goes backwards.  */
static long long
now_usec(void)
{
#ifdef _WIN32
   static LARGE_INTEGER freq;
   LARGE_INTEGER count;
   if (freq.QuadPart == 0)
      QueryPerformanceFrequency(&freq);
   QueryPerformanceCounter(&count);
   return count.QuadPart / freq.QuadPart * 1000000
      + count.QuadPart % freq.QuadPart * 1000000 / freq.QuadPart;
#else
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return (long long) ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#endif
}

static long long start_usec;

void
init_timer(void)
{
   start_usec = now_usec();
}

void
reset_timer(void)
{
   /* Keep the part of a tick that has already passed, so that time
      doesn't drift when the caller resets after every frame.  */
   long long now = now_usec();
   long long elapsed = now - start_usec;
   if (elapsed / USEC_PER_TICK > MAX_TICKS)
      start_usec = now;
   else
      start_usec = now - elapsed % USEC_PER_TICK;
}

int
read_timer(void)
{
   long long ticks = (now_usec() - start_usec) / USEC_PER_TICK;
   return ticks > MAX_TICKS ? MAX_TICKS : (int) ticks;
}

// Local Variables:
// c-basic-offset: 3
// End:
