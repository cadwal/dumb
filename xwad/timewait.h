/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/timer.h: Waiting for a time under X
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

#ifndef XWAD_TIMER_H
#define XWAD_TIMER_H

#include <X11/Xlib.h>

/* Returns True if an event was received. */
int xnextevent_before(Display *display, XEvent *event_return,
		      const struct timeval *before);

#endif /* XWAD_TIMER_H */

// Local Variables:
// c-basic-offset: 3
// End:
