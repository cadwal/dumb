/* 
   generic_fbrerend.c

   Copyright (C) 1997 Marcus Sundberg (e94_msu@e.kth.se)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdlib.h>
#include <render/render.h>
#include "fbrerend.h"

#ifdef WANT_1BPP
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
      int i, j, width=xsize*xfact;
      int rendinc = width*yfact;
      for (j=0; j < ysize; j++) {
	for (i=0; i < xsize; i++) {
	  register mul=i*xfact;
	  rendfb[width+mul+1] = rendfb[width+mul] = rendfb[mul+1] = rendfb[mul] = fb[i];
	}
	rendfb += rendinc;
	fb += xsize;
      }
    } else {
      int i, j, k, l;
      int width = xsize*xfact;
      int rendinc = width*yfact, xifactor = xfact-xlace, yifactor = yfact-ylace;
      for (j=0; j < ysize; j++) {
	for (i=0; i < xsize; i++) {
	  register pixval = fb[i], m = i*xfact;
	  for (k=0; k < xifactor; k++)
	    for (l=0; l < yifactor; l++) {
	      rendfb[m+k+width*l] = pixval;
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


#ifdef WANT_2BPP
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
      int i, j, width=xsize*xfact;
      int rendinc = width*yfact;
      for (j=0; j < ysize; j++) {
	for (i=0; i < xsize; i++) {
	  register mul=i*xfact;
	  rendfb[width+mul+1] = rendfb[width+mul] = rendfb[mul+1] = rendfb[mul] = fb[i];
	}
	rendfb += rendinc;
	fb += xsize;
      }
    } else {
      int i, j, k, l;
      int width = xsize*xfact;
      int rendinc = width*yfact, xifactor = xfact-xlace, yifactor = yfact-ylace;
      for (j=0; j < ysize; j++) {
	for (i=0; i < xsize; i++) {
	  register pixval = fb[i], m = i*xfact;
	  for (k=0; k < xifactor; k++)
	    for (l=0; l < yifactor; l++) {
	      rendfb[m+k+width*l] = pixval;
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


#ifdef WANT_4BPP
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
      int i, j, width=xsize*xfact;
      int rendinc = width*yfact;
      for (j=0; j < ysize; j++) {
	for (i=0; i < xsize; i++) {
	  register mul=i*xfact;
	  rendfb[width+mul+1] = rendfb[width+mul] = rendfb[mul+1] = rendfb[mul] = fb[i];
	}
	rendfb += rendinc;
	fb += xsize;
      }
    } else {
      int i, j, k, l;
      int width = xsize*xfact;
      int rendinc = width*yfact, xifactor = xfact-xlace, yifactor = yfact-ylace;
      for (j=0; j < ysize; j++) {
	for (i=0; i < xsize; i++) {
	  register pixval = fb[i], m = i*xfact;
	  for (k=0; k < xifactor; k++)
	    for (l=0; l < yifactor; l++) {
	      rendfb[m+k+width*l] = pixval;
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
