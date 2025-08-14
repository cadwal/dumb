/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/generic_fbrerend.c: Rescaling the framebuffer.  Generic version.
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

#include <config.h>

#include <stdlib.h>

#include "render.h"
#include "fbrerend.h"

void
fbrerender8(Pixel8 *destfb,
	    const Pixel8 *srcfb,
	    int xsize, int ysize,
	    int xfact, int yfact,
	    int xlace, int ylace)
{
   if (xfact == 1 && yfact == 1)
      abort();			/* Don't do that, then!  */
   else if (xfact == 2 && yfact == 2 && xlace == 0 && ylace == 0) {
      int i, j, width = xsize * xfact;
      int destinc = width * yfact;
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int mul = i * xfact;
	    destfb[width + mul + 1]
	       = destfb[width + mul]
	       = destfb[mul + 1]
	       = destfb[mul]
	       = srcfb[i];
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else {
      int i, j, k, l;
      int width = xsize * xfact;
      int destinc = width * yfact;
      int xifactor = xfact - xlace, yifactor = yfact - ylace;
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int pixval = srcfb[i], m = i * xfact;
	    for (k = 0; k < xifactor; k++)
	       for (l = 0; l < yifactor; l++) {
		  destfb[m + k + width * l] = pixval;
	       }
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   }
}

void
fbrerender16(Pixel16 *destfb,
	     const Pixel16 *srcfb,
	     int xsize, int ysize,
	     int xfact, int yfact,
	     int xlace, int ylace)
{
   if (xfact == 1 && yfact == 1)
      abort();
   else if (xfact == 2 && yfact == 2 && xlace == 0 && ylace == 0) {
      int i, j, width = xsize * xfact;
      int destinc = width * yfact;
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int mul = i * xfact;
	    destfb[width + mul + 1]
	       = destfb[width + mul]
	       = destfb[mul + 1]
	       = destfb[mul]
	       = srcfb[i];
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else {
      int i, j, k, l;
      int width = xsize * xfact;
      int destinc = width * yfact;
      int xifactor = xfact - xlace, yifactor = yfact - ylace;
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int pixval = srcfb[i], m = i * xfact;
	    for (k = 0; k < xifactor; k++)
	       for (l = 0; l < yifactor; l++) {
		  destfb[m + k + width * l] = pixval;
	       }
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   }
}

void
fbrerender32(Pixel32 *destfb,
	     const Pixel32 *srcfb,
	     int xsize, int ysize,
	     int xfact, int yfact,
	     int xlace, int ylace)
{
   if (xfact == 1 && yfact == 1)
      abort();
   else if (xfact == 2 && yfact == 2 && xlace == 0 && ylace == 0) {
      int i, j, width = xsize * xfact;
      int destinc = width * yfact;
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int mul = i * xfact;
	    destfb[width + mul + 1]
	       = destfb[width + mul]
	       = destfb[mul + 1]
	       = destfb[mul]
	       = srcfb[i];
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else {
      int i, j, k, l;
      int width = xsize * xfact;
      int destinc = width * yfact;
      int xifactor = xfact - xlace, yifactor = yfact - ylace;
      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int pixval = srcfb[i], m = i * xfact;
	    for (k = 0; k < xifactor; k++)
	       for (l = 0; l < yifactor; l++) {
		  destfb[m + k + width * l] = pixval;
	       }
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
