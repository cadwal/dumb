/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/generic_fbrerend.c: Rescaling the framebuffer.  Generic version.
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

#ifdef DUMB_CONFIG_8BPP
void *
fbrerender8(Pixel8 *fb,
	    Pixel8 *rendfb,
	    int xsize, int ysize,
	    int xfact, int yfact,
	    int xlace, int ylace)
{
   if ((xfact & yfact) == 1)
      return fb;
   else {
      void *retfb = rendfb;
      if ((xfact & yfact) == 2 && (xlace & ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register mul = i * xfact;
	       rendfb[width + mul + 1]
		  = rendfb[width + mul]
		  = rendfb[mul + 1]
		  = rendfb[mul]
		  = fb[i];
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else {
	 int i, j, k, l;
	 int width = xsize * xfact;
	 int rendinc = width * yfact;
	 int xifactor = xfact - xlace, yifactor = yfact - ylace;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register pixval = fb[i], m = i * xfact;
	       for (k = 0; k < xifactor; k++)
		  for (l = 0; l < yifactor; l++) {
		     rendfb[m + k + width * l] = pixval;
		  }
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      }
      return retfb;
   }
}
#endif


#ifdef DUMB_CONFIG_16BPP
void *
fbrerender16(Pixel16 *fb,
	     Pixel16 *rendfb,
	     int xsize, int ysize,
	     int xfact, int yfact,
	     int xlace, int ylace)
{
   if ((xfact & yfact) == 1)
      return fb;
   else {
      void *retfb = rendfb;
      if ((xfact & yfact) == 2 && (xlace & ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register mul = i * xfact;
	       rendfb[width + mul + 1]
		  = rendfb[width + mul]
		  = rendfb[mul + 1]
		  = rendfb[mul]
		  = fb[i];
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else {
	 int i, j, k, l;
	 int width = xsize * xfact;
	 int rendinc = width * yfact;
	 int xifactor = xfact - xlace, yifactor = yfact - ylace;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register pixval = fb[i], m = i * xfact;
	       for (k = 0; k < xifactor; k++)
		  for (l = 0; l < yifactor; l++) {
		     rendfb[m + k + width * l] = pixval;
		  }
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      }
      return retfb;
   }
}
#endif


#ifdef DUMB_CONFIG_32BPP
void *
fbrerender32(Pixel32 *fb,
	     Pixel32 *rendfb,
	     int xsize, int ysize,
	     int xfact, int yfact,
	     int xlace, int ylace)
{
   if ((xfact & yfact) == 1)
      return fb;
   else {
      void *retfb = rendfb;
      if ((xfact & yfact) == 2 && (xlace & ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register mul = i * xfact;
	       rendfb[width + mul + 1]
		  = rendfb[width + mul]
		  = rendfb[mul + 1]
		  = rendfb[mul]
		  = fb[i];
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else {
	 int i, j, k, l;
	 int width = xsize * xfact;
	 int rendinc = width * yfact;
	 int xifactor = xfact - xlace, yifactor = yfact - ylace;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register pixval = fb[i], m = i * xfact;
	       for (k = 0; k < xifactor; k++)
		  for (l = 0; l < yifactor; l++) {
		     rendfb[m + k + width * l] = pixval;
		  }
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      }
      return retfb;
   }
}
#endif

// Local Variables:
// c-basic-offset: 3
// End:
