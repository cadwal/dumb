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

#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "timer.h"

/* alarm will occur after this many micro-seconds */
#define INIT_USEC 999999
#define INIT_SEC 0

void
init_timer(void)
{
   reset_timer();
   signal(SIGALRM, SIG_IGN);
}

void
reset_timer(void)
{
   struct itimerval it;
   it.it_value.tv_usec = INIT_USEC;
   it.it_value.tv_sec = INIT_SEC;
   it.it_interval.tv_usec = 0;
   it.it_interval.tv_sec = 0;
   setitimer(ITIMER_REAL, &it, NULL);
}

int
read_timer(void)
{
   struct itimerval it;
   getitimer(ITIMER_REAL, &it);
   if (it.it_value.tv_sec > 0)
      return 0;
   return (INIT_USEC - it.it_value.tv_usec) / USEC_PER_TICK;
}

// Local Variables:
// c-basic-offset: 3
// End:
