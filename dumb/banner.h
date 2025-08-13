/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/banner.h: Banners that scroll over the view.
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

#ifndef BANNER_H
#define BANNER_H

#include "libdumb/texture.h"

int init_banner(int baseline, int start, int stop, int speed);
void reset_banner(int banner);
void reset_banners(void);

void add_to_banner(int banner, Texture *t, int xoffset, int yoffset);
void add_text_to_banner(int banner, int font, const char *text, int len);
void add_str_to_banner(int banner, int font, const char *text);

void update_banners(void *fb, int ticks);

int banner_queuelen(int banner);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
