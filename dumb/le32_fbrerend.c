/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/le32_fbrerend.c: Rescaling the framebuffer.  Little-endian 32-bit ver.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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
	 for (i = 0; i < xsize; i += 2) {
	    int mul = i * xfact;
#ifdef HAVE_MMX
	    __asm__(
		    "movd %2, %%mm0\n\t"
		    "punpcklbw %%mm0, %%mm0\n\t"
		    "movq %%mm0, %0\n\t"
		    "movq %%mm0, %1\n\t"
		    : "=m"(destfb[mul]), "=m"(destfb[mul+width])
		    : "m"(srcfb[i])
		    );
	    /* If we were using MMX, we're really doing 64bit
	       rerendering.  But this can't go in le64_fbrerend, 
	       because when xfact or yfact aren't 2, we'll be using
	       32bit again.  So, we need to bump up i by 2, just
	       if we were really doing 64bit */
	    i += 2;
#else  /* !HAVE_MMX */
	    register unsigned tmp;
	    register unsigned char pix1 = srcfb[i], pix2 = srcfb[i + 1];
	    tmp = ((unsigned int) pix1
		   + ((unsigned int) pix1 << 8)
		   + ((unsigned int) pix2 << 16)
		   + ((unsigned int) pix2 << 24));
	    *((unsigned int *) (destfb + mul + width))
	       = *((unsigned int *) (destfb + mul)) = tmp;
#endif /* !HAVE_MMX */
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
#ifdef HAVE_MMX
      /* tidy up the MMX/FP state */
      __asm__("emms\n\t");
#endif /* HAVE_MMX */
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
	    int mul = i * xfact;
#ifdef HAVE_MMX
	    /* I have NOT TESTED THIS! -- josh */
	    __asm__(
		    "movd %2, %%mm0\n\t"
		    "punpcklwd %%mm0, %%mm0\n\t"
		    "movq %%mm0, %0\n\t"
		    "movq %%mm0, %1\n\t"
		    : "=m"(destfb[mul]), "=m"(destfb[mul+width])
		    : "m"(srcfb[i])
		    );
	    i ++;
#else  /* !HAVE_MMX */
	    register unsigned short pix1 = srcfb[i];
	    *((unsigned int *) (destfb + mul + width))
	       = *((unsigned int *) (destfb + mul))
	       = ((unsigned int) pix1 + ((unsigned int) pix1 << 16));
#endif /* !HAVE_MMX */
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
#ifdef HAVE_MMX
      /* tidy up the MMX/FP state */
      __asm__("emms\n\t");
#endif /* HAVE_MMX */
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
#ifdef HAVE_MMX
	    /* I have NOT TESTED THIS! -- josh */
	    __asm__(
		    "movd %2, %%mm0\n\t"
		    "punpckldq %%mm0, %%mm0\n\t"
		    "movq %%mm0, %0\n\t"
		    "movq %%mm0, %1\n\t"
		    : "=m"(destfb[mul]), "=m"(destfb[mul+width])
		    : "m"(srcfb[i])
		    );
#else  /* !HAVE_MMX */
	       destfb[width + mul + 1]
		  = destfb[width + mul]
		  = destfb[mul + 1]
		  = destfb[mul]
		  = srcfb[i];
#endif /* !HAVE_MMX */
	 }
	 destfb += destinc;
	 srcfb += xsize;
      }
#ifdef HAVE_MMX
      /* tidy up the MMX/FP state */
      __asm__("emms\n\t");
#endif /* HAVE_MMX */
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
