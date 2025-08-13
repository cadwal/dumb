/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/le64_fbrerend.c: Rescaling the framebuffer.  Little-endian 64-bit ver.
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
      if ((xfact & yfact) == 2 && (xlace | ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;

	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i += 4) {
	       register mul = i * xfact;
	       unsigned long register tmp;
	       unsigned char register pix1 = fb[i], pix2 = fb[i + 1],
	        pix3 = fb[i + 2], pix4 = fb[i + 3];
	       tmp = ((unsigned long) pix1
		      + ((unsigned long) pix1 << 8)
		      + ((unsigned long) pix2 << 16)
		      + ((unsigned long) pix2 << 24)
		      + ((unsigned long) pix3 << 32)
		      + ((unsigned long) pix3 << 40)
		      + ((unsigned long) pix4 << 48)
		      + ((unsigned long) pix4 << 56));
	       *((unsigned long *) (rendfb + mul + width))
		   = *((unsigned long *) (rendfb + mul)) = tmp;
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else if ((xfact & yfact) == 4 && (xlace | ylace) == 0) {
	 int i, j;
	 register width = xsize * xfact;
	 int rendinc = width * yfact;

	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i += 2) {
	       register mul = i * xfact;
	       unsigned long register tmp;
	       unsigned char register pix1 = fb[i], pix2 = fb[i + 1];
	       tmp = ((unsigned long) pix1
		      + ((unsigned long) pix1 << 8)
		      + ((unsigned long) pix1 << 16)
		      + ((unsigned long) pix1 << 24)
		      + ((unsigned long) pix2 << 32)
		      + ((unsigned long) pix2 << 40)
		      + ((unsigned long) pix2 << 48)
		      + ((unsigned long) pix2 << 56));
	       *((unsigned long *) (rendfb + mul + width * 3))
		   = *((unsigned long *) (rendfb + mul + width * 2))
		   = *((unsigned long *) (rendfb + mul + width))
		   = *((unsigned long *) (rendfb + mul)) = tmp;
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else {
	 register i, j, k, l, width = xsize * xfact;
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
/* This is UNTESTED /Marcus */
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
      if ((xfact & yfact) == 2 && (xlace | ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;
	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i += 2) {
	       register mul = i * xfact;
	       unsigned long register tmp;
	       unsigned short register pix1 = fb[i], pix2 = fb[i + 1];
	       tmp = ((unsigned long) pix1
		      + ((unsigned long) pix1 << 16)
		      + ((unsigned long) pix2 << 32)
		      + ((unsigned long) pix2 << 48));
	       *((unsigned long *) (rendfb + mul + width))
		   = *((unsigned long *) (rendfb + mul)) = tmp;
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else if ((xfact & yfact) == 4 && (xlace | ylace) == 0) {
	 int i, j;
	 register width = xsize * xfact;
	 int rendinc = width * yfact;

	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register mul = i * xfact;
	       unsigned long register tmp;
	       unsigned short register pix1 = fb[i];
	       tmp = ((unsigned long) pix1
		      + ((unsigned long) pix1 << 16)
		      + ((unsigned long) pix1 << 32)
		      + ((unsigned long) pix1 << 48));
	       *((unsigned long *) (rendfb + mul + width * 3))
		   = *((unsigned long *) (rendfb + mul + width * 2))
		   = *((unsigned long *) (rendfb + mul + width))
		   = *((unsigned long *) (rendfb + mul)) = tmp;
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
/* This is UNTESTED /Marcus */
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
      if ((xfact & yfact) == 2 && (xlace | ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;

	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register mul = i * xfact;
	       unsigned long register tmp;
	       unsigned int register pix1 = fb[i];
	       tmp = ((unsigned long) pix1
		      + ((unsigned long) pix1 << 32));
	       *((unsigned long *) (rendfb + mul + width))
		   = *((unsigned long *) (rendfb + mul)) = tmp;
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      } else if ((xfact & yfact) == 4 && (xlace | ylace) == 0) {
	 int i, j, width = xsize * xfact;
	 int rendinc = width * yfact;

	 for (j = 0; j < ysize; j++) {
	    for (i = 0; i < xsize; i++) {
	       register mul = i * xfact;
	       unsigned long register tmp;
	       unsigned int register pix1 = fb[i];
	       tmp = ((unsigned long) pix1 + ((unsigned long) pix1 << 32));
	       *((unsigned long *) (rendfb + mul + width * 3))
		   = *((unsigned long *) (rendfb + mul + width * 2))
		   = *((unsigned long *) (rendfb + mul + width))
		   = *((unsigned long *) (rendfb + mul))
		   = *((unsigned long *) (rendfb + mul + width * 3 + 2))
		   = *((unsigned long *) (rendfb + mul + width * 2 + 2))
		   = *((unsigned long *) (rendfb + mul + width + 2))
		   = *((unsigned long *) (rendfb + mul + 2)) = tmp;
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
