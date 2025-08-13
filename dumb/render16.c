/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/render16.c: Renderer for framebuffers with 16 bits per pixel.
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

#include "render.h"

#define USE_COLORMAP
#define COLORMAP "COLORM16"
typedef Pixel16 Pixel;
typedef Pixel8 TPixel;
#define render render16
#define init_renderer init_renderer16
#define reset_renderer reset_renderer16
#define BPP 2

#include "renderc.h"

// Local Variables:
// c-basic-offset: 3
// End:
