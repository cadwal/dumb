/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/video.h: Declarations for functions each video driver must define.
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

#ifndef VIDEO_H
#define VIDEO_H

#include "libdumbutil/confdef.h"

extern ConfItem video_conf[];

void init_video(int *width, int *height, int *bpp, int *real_width);
void reset_video(void);

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue);

void *video_newframe(void);
void video_updateframe(void *v);

/* accessing the video subsystem on many systems requires special permissions
 * video_preinit() should do whatever initialization requires these, and
 * then release them, for security reasons.  It should not change the
 * video mode.  It will be called first thing by main() */
void video_preinit(void);

/* set information to be used by a window manager */
void video_winstuff(const char *desc, int xdim, int ydim);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
