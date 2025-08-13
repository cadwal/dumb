/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/dummy_video.c: Dummy video driver.
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

#include <stdarg.h>
#include <stdio.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "video.h"

ConfItem video_conf[] =
{
   CONFITEM_END
};

static void *fb = NULL;

void
video_preinit(void)
{
}

void
init_video(int *width, int *height, int *bpp, int *real_width)
{
   if (fb == NULL)
      fb = safe_calloc((*width) * (*height), *bpp);
   *real_width = *width;
#ifdef FB_IN_GS			/* FB_IN_GS is i386 only wizardry */
   /* make sure that gs overrides are harmless */
   asm("movw %%ds,%%ax\n\tmovw %%ax,%%gs\n\t": : :"eax");
#endif
}

void
reset_video(void)
{
   if (fb)
      safe_free(fb);
}

void
video_setpal(unsigned char idx,
	     unsigned char red, unsigned char green, unsigned char blue)
{
}

void *
video_newframe(void)
{
   return fb;
}

void
video_updateframe(void *v)
{
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
}

// Local Variables:
// c-basic-offset: 3
// End:
