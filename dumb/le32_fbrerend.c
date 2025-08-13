/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/le32_fbrerend.c: Rescaling the framebuffer.  Little-endian 32-bit ver.
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
	    for (i = 0; i < xsize; i += 2) {
	       int mul = i * xfact;
	       register unsigned tmp;
	       register unsigned char pix1 = fb[i], pix2 = fb[i + 1];
#ifdef HAVE_MMX
	       __asm__(
		       "movd %2, %%mm0\n\t"
		       "punpcklbw %%mm0, %%mm0\n\t"
		       "movq %%mm0, %0\n\t"
		       "movq %%mm0, %1\n\t"
		       : "=m"(rendfb[mul]), "=m"(rendfb[mul+width])
		       : "m"(fb[i])
		       );
	       /* If we were using MMX, we're really doing 64bit
		  rerendering.  But this can't go in le64_fbrerend, 
		  because when xfact or yfact aren't 2, we'll be using
		  32bit again.  So, we need to bump up i by 2, just
	          if we were really doing 64bit */
	       i += 2;
#else  /* !HAVE_MMX */
	       tmp = ((unsigned int) pix1
		      + ((unsigned int) pix1 << 8)
		      + ((unsigned int) pix2 << 16)
		      + ((unsigned int) pix2 << 24));
	       *((unsigned int *) (rendfb + mul + width))
		   = *((unsigned int *) (rendfb + mul)) = tmp;
#endif /* !HAVE_MMX */
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
	       register int pixval = fb[i], m = i * xfact;
	       for (k = 0; k < xifactor; k++)
		  for (l = 0; l < yifactor; l++) {
		     rendfb[m + k + width * l] = pixval;
		  }
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      }
#ifdef HAVE_MMX
      /* tidy up the MMX/FP state */
      __asm__("emms\n\t");
#endif /* HAVE_MMX */
      return retfb;
   }
}
#endif /* DUMB_CONFIG_8BPP */


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
	       int mul = i * xfact;
	       register unsigned short pix1 = fb[i];
#ifdef HAVE_MMX
	       /* I have NOT TESTED THIS! -- josh */
	       __asm__(
		       "movd %2, %%mm0\n\t"
		       "punpcklwd %%mm0, %%mm0\n\t"
		       "movq %%mm0, %0\n\t"
		       "movq %%mm0, %1\n\t"
		       : "=m"(rendfb[mul]), "=m"(rendfb[mul+width])
		       : "m"(fb[i])
		       );
	       i ++;
#else  /* !HAVE_MMX */
	       *((unsigned int *) (rendfb + mul + width))
		   = *((unsigned int *) (rendfb + mul))
		   = ((unsigned int) pix1 + ((unsigned int) pix1 << 16));
#endif /* !HAVE_MMX */
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
	       register int pixval = fb[i], m = i * xfact;
	       for (k = 0; k < xifactor; k++)
		  for (l = 0; l < yifactor; l++) {
		     rendfb[m + k + width * l] = pixval;
		  }
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      }
#ifdef HAVE_MMX
      /* tidy up the MMX/FP state */
      __asm__("emms\n\t");
#endif /* HAVE_MMX */
      return retfb;
   }
}
#endif /* DUMB_CONFIG_16BPP */


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
	       register int mul = i * xfact;
#ifdef HAVE_MMX
	       /* I have NOT TESTED THIS! -- josh */
	       __asm__(
		       "movd %2, %%mm0\n\t"
		       "punpckldq %%mm0, %%mm0\n\t"
		       "movq %%mm0, %0\n\t"
		       "movq %%mm0, %1\n\t"
		       : "=m"(rendfb[mul]), "=m"(rendfb[mul+width])
		       : "m"(fb[i])
		       );
#else  /* !HAVE_MMX */
	       rendfb[width + mul + 1]
		  = rendfb[width + mul]
		  = rendfb[mul + 1]
		  = rendfb[mul]
		  = fb[i];
#endif /* !HAVE_MMX */
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
	       register int pixval = fb[i], m = i * xfact;
	       for (k = 0; k < xifactor; k++)
		  for (l = 0; l < yifactor; l++) {
		     rendfb[m + k + width * l] = pixval;
		  }
	    }
	    rendfb += rendinc;
	    fb += xsize;
	 }
      }
#ifdef HAVE_MMX
      /* tidy up the MMX/FP state */
      __asm__("emms\n\t");
#endif /* HAVE_MMX */
      return retfb;
   }
}
#endif /* DUMB_CONFIG_32BPP */

// Local Variables:
// c-basic-offset: 3
// End:
