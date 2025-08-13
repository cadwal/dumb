/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/xtexture.c: Drawing textures in X11.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xtexture.h"

#define scrn DefaultScreen(dpy)
#define rootw RootWindow(dpy,scrn)
#define visual DefaultVisual(dpy,scrn)

#ifndef __cplusplus
#define c_class class
#endif

#define rotate(Pixel) \
   for(x=0;x<t->width;x++) { \
      Pixel *tx=t->texels; \
      tx+=(t->width-x-1)<<t->log2height; \
      for(y=t->height-1;y>=0;y--,tx++) \
	 XPutPixel(image,x,y,*tx==-1?0:*tx); \
   }
#define mirror(Pixel) \
   for(x=0;x<t->width;x++) { \
      Pixel *tx=t->texels; \
      tx+=x<<t->log2height; \
      for(y=t->height-1;y>=0;y--,tx++) \
	 XPutPixel(image,x,y,*tx==-1?0:*tx); \
   }

static Display *cmdpy = NULL;
static Colormap cmap = None;

void
setpf(unsigned char idx,
      unsigned char red,
      unsigned char green,
      unsigned char blue)
{
   XColor xc =
   {idx, red << 8, green << 8, blue << 8, DoRed | DoGreen | DoBlue, 0};
   if (cmap != None && cmdpy != NULL)
      XStoreColor(cmdpy, cmap, &xc);
}

Colormap
make_xtexture_cmap(Display *dpy, Window w)
{
   if (visual->c_class != PseudoColor)
      return None;
   cmap = XCreateColormap(cmdpy = dpy, w, visual, AllocAll);
   if (cmap != None)
      set_playpal(0, setpf);
   return cmap;
}

void
set_xtexture_cmap(Display *dpy, Window w)
{
   if (cmap == None)
      make_xtexture_cmap(dpy, w);
   if (cmap != None) {
      XSetWindowColormap(dpy, w, cmap);
      XInstallColormap(dpy, cmap);
   }
}

void
xtexture(Display *dpy, Drawable d, Texture *t, int do_mirror)
{
   int width, height;
   int frame_x, frame_y, depth, border;
   Window frame_root;
   int bypp;
   int x, y;
   int xo = 0, yo = 0;
   XImage *image;
   if (t == NULL)
      return;
   XGetGeometry(dpy, d, &frame_root,
		&frame_x, &frame_y, &width, &height,
		&border, &depth);
   if (width > t->width)
      xo = (width - t->width) / 2;
   if (height > t->height)
      yo = (height - t->height) / 2;
   if (do_mirror)
      do_mirror = 1;
   bypp = depth / 8;
   if (bypp == 3)
      bypp++;
   if (t->xpix[do_mirror] == 0) {
      load_texels(t, bypp);
      image = XCreateImage(dpy,
			   visual,
			   depth,
			 ZPixmap, 0, malloc(bypp * t->height * t->width),
			   t->width, t->height,
			   8, bypp * t->width);
      if (do_mirror)
	 switch (bypp) {
	 case (1):
	    mirror(signed char);
	    break;
	 case (2):
	    mirror(signed short);
	    break;
	 case (4):
	    mirror(signed int);
	    break;
      } else
	 switch (bypp) {
	 case (1):
	    rotate(signed char);
	    break;
	 case (2):
	    rotate(signed short);
	    break;
	 case (4):
	    rotate(signed int);
	    break;
	 }
      t->xpix[do_mirror] = XCreatePixmap(dpy, d, t->width, t->height, depth);
      XPutImage(dpy, t->xpix[do_mirror] ? t->xpix[do_mirror] : d,
		DefaultGC(dpy, scrn), image, 0, 0,
		t->xpix[do_mirror] ? 0 : xo, t->xpix[do_mirror] ? 0 : yo,
		t->width, t->height);
      XDestroyImage(image);
      if (t->xpix[do_mirror])
	 free_texels(t);
   }
   if (t->xpix[do_mirror])
      XCopyArea(dpy, t->xpix[do_mirror], d, DefaultGC(dpy, scrn),
		0, 0, t->width, t->height, xo, yo);
}

// Local Variables:
// c-basic-offset: 3
// End:
