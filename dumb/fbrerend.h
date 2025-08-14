/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/fbrerend.h: Rescaling the framebuffer.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1997 by Marcus Sundberg <e94_msu@e.kth.se>
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

#ifndef FBREREND_H
#define FBREREND_H

#include "render.h"

/* In DUMB 0.13.9, these functions returned the address of the
   framebuffer where they had drawn.  It was usually DESTFB but could
   be SRCFB if XFACT and YFACT were 1.

   However, the video routines expect DUMB to render to the
   framebuffer returned by video_newframe(), so these functions can't
   just return another pointer and hope it will work.

   In DUMB 0.13.10, the functions call abort() if they notice there's
   nothing to do.  The alternative would be to memcpy() the
   framebuffer but it's better to use the right one in the first
   place.  */

void fbrerender8(Pixel8 *destfb, const Pixel8 *srcfb,
		 int xsize, int ysize,
		 int xfact, int yfact,
		 int xlace, int ylace);
void fbrerender16(Pixel16 *destfb, const Pixel16 *srcfb,
		  int xsize, int ysize,
		  int xfact, int yfact,
		  int xlace, int ylace);
void fbrerender32(Pixel32 *destfb, const Pixel32 *srcfb,
		  int xsize, int ysize,
		  int xfact, int yfact,
		  int xlace, int ylace);

/* Calls one of the above */
void fbrerender(int bytes_per_pixel,
		void *destfb, const void *srcfb,
		int xsize, int ysize,
		int xfact, int yfact,
		int xlace, int ylace);

#endif /* FBREREND_H */

// Local Variables:
// c-basic-offset: 3
// End:
