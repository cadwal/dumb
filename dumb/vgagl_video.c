/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/vgagl_video.c: Video driver for vgagl, whatever that is.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1994 by Chris Laurel
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vga.h>
#include <vgagl.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "video.h"

#ifndef linux
#error Compiling vgagl_video.c for a non-linux system???
/* Well, maybe they're trying to port to BSD or something... */
#endif

#define MAX_PAGES 2
#define MIN_PAGES 2

static int vgamode = TEXT;

static GraphicsContext physicalscreen;

void
video_preinit(void)
{
   vga_init();
}

void
init_video(int *_width, int *_height, int *_bpp)
{
   /*
    * For security reasons, we shouldn't call vga_init() here:
    * it MUST be called as the first thing in main()
    */

   switch (*_bpp) {
   case (4):
      switch (*_width) {
      case (1280):
	 vgamode = (G1280x1024x16M);
	 break;
      case (1024):
	 vgamode = (G1024x768x16M);
	 break;
      case (800):
	 vgamode = (G800x600x16M);
	 break;
      case (640):
	 vgamode = (G640x480x16M);
	 break;
      case (320):
	 vgamode = (G320x200x16M);
	 break;
      }
      break;
   case (2):
      switch (*_width) {
      case (1280):
	 vgamode = (G1280x1024x64K);
	 break;
      case (1024):
	 vgamode = (G1024x768x64K);
	 break;
      case (800):
	 vgamode = (G800x600x64K);
	 break;
      case (640):
	 vgamode = (G640x480x64K);
	 break;
      case (320):
	 vgamode = (G320x200x64K);
	 break;
      }
      break;
   case (1):
      switch (*_width) {
      case (1280):
	 vgamode = (G1280x1024x256);
	 break;
      case (1024):
	 vgamode = (G1024x768x256);
	 break;
      case (800):
	 vgamode = (G800x600x256);
	 break;
      case (640):
	 vgamode = (G640x480x256);
	 break;
      case (360):
	 vgamode = (G360x480x256);
	 break;
      default:
      case (320):
	 switch (*_height) {
	 default:
	 case (400):
	    vgamode = (G320x400x256);
	    break;
	 case (240):
	    vgamode = (G320x240x256);
	    break;
	 case (200):
	    vgamode = (G320x200x256);
	    break;
	 }
      }
      break;
   default:
      logfatal('V', _("Bad BPP (%d) in init_video"), *_bpp);
   }

   vga_setmode(vgamode);
   logprintf(LOG_INFO, 'V', _("VGA-linear=%x"),
	     vga_setlinearaddressing());
   gl_setcontextvga(vgamode);
   gl_getcontext(&physicalscreen);
   logprintf(LOG_INFO, 'V', _("GL-flipmode=%d"),
	     gl_enablepageflipping(&physicalscreen));
   gl_setcontextvgavirtual(vgamode);

   *_width = vga_getxdim();
   *_height = vga_getydim();
}

void
reset_video(void)
{
   vga_setmode(TEXT);
}

void
video_setpal(unsigned char idx,
	     unsigned char red,
	     unsigned char green,
	     unsigned char blue)
{
   vga_setpalette(idx, red >> 2, green >> 2, blue >> 2);
}

void *
video_newframe(void)
{
   return VBUF;
}

void
video_updateframe(void *v)
{
   gl_copyscreen(&physicalscreen);
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
}

// Local Variables:
// c-basic-offset: 3
// End:
