/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/render.h: The renderer.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#ifndef RENDER_H
#define RENDER_H

#include "libdumb/view.h"
#include "levdata.h"

typedef unsigned char Pixel8;
typedef unsigned short Pixel16;
typedef unsigned int Pixel32;

typedef void INIT_RENDERER_FN (int width, int height,
			       int real_width, int real_height);
typedef void RESET_RENDERER_FN (void);
typedef void RENDER_FN (void *fb, LevData *ld, const View *v);

INIT_RENDERER_FN init_renderer8, init_renderer16, init_renderer32;
RESET_RENDERER_FN reset_renderer8, reset_renderer16, reset_renderer32;
RENDER_FN render8, render16, render32;

extern int renderer_debug_flags;

#endif

// Local Variables:
// c-basic-offset: 3
// End:
