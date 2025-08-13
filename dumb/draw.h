/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/draw.h: Drawing on the framebuffer without scaling.
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

#ifndef DRAW_H
#define DRAW_H

#include "libdumb/texture.h"

void init_draw(int width, int height, int bpp, int real_width);

void draw(void *framebuf, Texture *t, int x, int y);
void draw_outline(void *framebuf, Texture *t, int x, int y);

void draw_center(void *fb, Texture *t);

void drawtext(void *fb, const char *text, int len, int font, int x, int y);
void drawstr(void *fb, const char *text, int font, int x, int y);

#endif /* DRAW_H */

// Local Variables:
// c-basic-offset: 3
// End:
