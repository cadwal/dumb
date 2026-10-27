/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/le64_fbrerend.c: Rescaling the framebuffer.  Little-endian 64-bit ver.
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

#include <stdint.h>
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
      abort();			/* Dont do that, then!  */
   else if (xfact == 2 && yfact == 2 && xlace == 0 && ylace == 0) {
      int i, j, width = xsize * xfact;
      int destinc = width * yfact;

      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i += 4) {
	    register int mul = i * xfact;
	    register uint64_t tmp;
	    register unsigned char pix1 = srcfb[i], pix2 = srcfb[i + 1],
				   pix3 = srcfb[i + 2], pix4 = srcfb[i + 3];
	    tmp = ((uint64_t) pix1
		   + ((uint64_t) pix1 << 8)
		   + ((uint64_t) pix2 << 16)
		   + ((uint64_t) pix2 << 24)
		   + ((uint64_t) pix3 << 32)
		   + ((uint64_t) pix3 << 40)
		   + ((uint64_t) pix4 << 48)
		   + ((uint64_t) pix4 << 56));
	    *((uint64_t *) (destfb + mul + width))
	       = *((uint64_t *) (destfb + mul)) = tmp;
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else if (xfact == 4 && yfact == 4 && xlace == 0 && ylace == 0) {
      int i, j;
      register int width = xsize * xfact;
      int destinc = width * yfact;

      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i += 2) {
	    register int mul = i * xfact;
	    register uint64_t tmp;
	    register unsigned char pix1 = srcfb[i], pix2 = srcfb[i + 1];
	    tmp = ((uint64_t) pix1
		   + ((uint64_t) pix1 << 8)
		   + ((uint64_t) pix1 << 16)
		   + ((uint64_t) pix1 << 24)
		   + ((uint64_t) pix2 << 32)
		   + ((uint64_t) pix2 << 40)
		   + ((uint64_t) pix2 << 48)
		   + ((uint64_t) pix2 << 56));
	    *((uint64_t *) (destfb + mul + width * 3))
	       = *((uint64_t *) (destfb + mul + width * 2))
	       = *((uint64_t *) (destfb + mul + width))
	       = *((uint64_t *) (destfb + mul)) = tmp;
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else {
      register int i, j, k, l, width = xsize * xfact;
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

/* This is UNTESTED /Marcus */
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
	 for (i = 0; i < xsize; i += 2) {
	    register int mul = i * xfact;
	    register uint64_t tmp;
	    register unsigned short pix1 = srcfb[i], pix2 = srcfb[i + 1];
	    tmp = ((uint64_t) pix1
		   + ((uint64_t) pix1 << 16)
		   + ((uint64_t) pix2 << 32)
		   + ((uint64_t) pix2 << 48));
	    *((uint64_t *) (destfb + mul + width))
	       = *((uint64_t *) (destfb + mul)) = tmp;
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else if (xfact == 4 && yfact == 4 && xlace == 0 && ylace == 0) {
      int i, j;
      register int width = xsize * xfact;
      int destinc = width * yfact;

      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int mul = i * xfact;
	    register uint64_t tmp;
	    register unsigned short pix1 = srcfb[i];
	    tmp = ((uint64_t) pix1
		   + ((uint64_t) pix1 << 16)
		   + ((uint64_t) pix1 << 32)
		   + ((uint64_t) pix1 << 48));
	    *((uint64_t *) (destfb + mul + width * 3))
	       = *((uint64_t *) (destfb + mul + width * 2))
	       = *((uint64_t *) (destfb + mul + width))
	       = *((uint64_t *) (destfb + mul)) = tmp;
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

/* This is UNTESTED /Marcus */
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
	    register uint64_t tmp;
	    register unsigned int pix1 = srcfb[i];
	    tmp = ((uint64_t) pix1
		   + ((uint64_t) pix1 << 32));
	    *((uint64_t *) (destfb + mul + width))
	       = *((uint64_t *) (destfb + mul)) = tmp;
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
   } else if (xfact == 4 && yfact == 4 && xlace == 0 && ylace == 0) {
      int i, j, width = xsize * xfact;
      int destinc = width * yfact;

      for (j = 0; j < ysize; j++) {
	 for (i = 0; i < xsize; i++) {
	    register int mul = i * xfact;
	    register uint64_t tmp;
	    register unsigned int pix1 = srcfb[i];
	    tmp = ((uint64_t) pix1 + ((uint64_t) pix1 << 32));
	    *((uint64_t *) (destfb + mul + width * 3))
	       = *((uint64_t *) (destfb + mul + width * 2))
	       = *((uint64_t *) (destfb + mul + width))
	       = *((uint64_t *) (destfb + mul))
	       = *((uint64_t *) (destfb + mul + width * 3 + 2))
	       = *((uint64_t *) (destfb + mul + width * 2 + 2))
	       = *((uint64_t *) (destfb + mul + width + 2))
	       = *((uint64_t *) (destfb + mul + 2)) = tmp;
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
