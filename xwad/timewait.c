/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/timer.c: Waiting for a time under X
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#ifndef HAVE_SELECT
/* This won't work without select().  So don't compile it, and hope
 * nothing needs it.  This could be made a makefile check, but it's
 * easier this way.  */
#else  /* HAVE_SELECT */

#include <sys/time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "libdumbutil/log.h"
#include "libdumbutil/dumb-nls.h"

static void add_x_in_fdset(fd_set *fdset, int *nfds, Display *display);
static void add_fd_in_fdset(fd_set *fdset, int *nfds, int fd);

/* From the GNU libc manual, but added static and const */
static int timeval_subtract (struct timeval *result,
			     const struct timeval *x,
			     const struct timeval *y);


/* BEFORE must not be NULL;
   if you don't want a timeout,
   use the standard XNextEvent().  */
int
xnextevent_before(Display *display, XEvent *event_return,
		  const struct timeval *before)
{
   fd_set fdset;
   int nfds;

   FD_ZERO(&fdset);
   nfds = 0;
   add_x_in_fdset(&fdset, &nfds, display);
   
   while (!XPending(display)) {
      fd_set read_fds;
      struct timeval now, select_timeout;
      
      gettimeofday(&now, NULL);
      if (timeval_subtract(&select_timeout, before, &now))
	 return 0;		/* timed out */

      /* Don't let select() munge x_fdset.  */
      read_fds = fdset;

      /* It doesn't matter what select() returns -- XPending()
       * will let us know if it got something.  */
      select(nfds, &read_fds, NULL, NULL, &select_timeout);
   }

   /* XNextEvent() seems to usually return 0 */
   XNextEvent(display, event_return);
   return 1;
}

static void
add_x_in_fdset(fd_set *fdset, int *nfds, Display *display)
{
   int *x_fd_list;
   int x_fd_count;
   int i;

   add_fd_in_fdset(fdset, nfds, ConnectionNumber(display));

   if (!XInternalConnectionNumbers(display, &x_fd_list, &x_fd_count))
      logfatal('X', _("Out of memory in XInternalConnectionNumbers"));
   for (i=0; i<x_fd_count; i++)
      add_fd_in_fdset(fdset, nfds, x_fd_list[i]);
   XFree(x_fd_list);
}

static void
add_fd_in_fdset(fd_set *fdset, int *nfds, int fd)
{
   FD_SET(fd, fdset);
   if (fd >= *nfds)
      *nfds = fd+1;
}


/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */

static int
timeval_subtract (struct timeval *result,
		  const struct timeval *x,
		  const struct timeval *orig_y)
{
   struct timeval copy_y = *orig_y;
   struct timeval *const y = &copy_y;

   /* Perform the carry for the later subtraction by updating Y. */
   if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
      y->tv_usec -= 1000000 * nsec;
      y->tv_sec += nsec;
   }
   if (x->tv_usec - y->tv_usec > 1000000) {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000;
      y->tv_usec += 1000000 * nsec;
      y->tv_sec -= nsec;
   }

   /* Compute the time remaining to wait.
      `tv_usec' is certainly positive. */
   result->tv_sec = x->tv_sec - y->tv_sec;
   result->tv_usec = x->tv_usec - y->tv_usec;

   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
}

#endif /* HAVE_SELECT */

// Local Variables:
// c-basic-offset: 3
// End:
