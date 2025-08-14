/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/fbrerend.c: Rescaling the framebuffer.  Type resolution.
 * Copyright (C) 1999 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#include "fbrerend.h"

void
fbrerender(int bytes_per_pixel,
	   void *destfb, const void *srcfb,
	   int xsize, int ysize,
	   int xfact, int yfact,
	   int xlace, int ylace)
{
   switch (bytes_per_pixel) {
   case 1:
      fbrerender8((Pixel8 *) destfb, (const Pixel8 *) srcfb,
		  xsize, ysize, xfact, yfact, xlace, ylace);
      break;
   case 2:
      fbrerender16((Pixel16 *) destfb, (const Pixel16 *) srcfb,
		   xsize, ysize, xfact, yfact, xlace, ylace);
      break;
   case 4:	 
      fbrerender32((Pixel32 *) destfb, (const Pixel32 *) srcfb,
		   xsize, ysize, xfact, yfact, xlace, ylace);
      break;
   default:
      abort();
   }
}

/* The other functions are in generic_fbrerend.c, le32_fbrerend.c or
   le64_fbrerend.c */

/*
 * Local Variables:
 * c-basic-offset: 3
 * End:
 */
