/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/texture.h: Textures.
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

#ifndef TEXTURE_H
#define TEXTURE_H

#include "libdumbwad/wadio.h"

typedef enum {
   TT_BAD, TT_WALL, TT_SPRITE, TT_FLAT, TT_PATCH, TT_FONT, TT_MISC
} TextureType;

typedef struct {
   int width, height, topofs, leftofs;
   unsigned char log2width, log2height;
   unsigned char bpp, _spare1;
   void *texels;
   unsigned int avg_texel;
   int xpix[2];
   TextureType type;
   LumpNum lumpnum;
   char opaque, alloced_texels;
   char name[10];
} Texture;

void free_texels(Texture *tex);
void load_texels(Texture *tex, int bytesperpixel);

#define cond_load_texels(t,bpp) if ((t)->texels==NULL) load_texels(t,bpp)

Texture *get_flat_texture(const char *name);
Texture *get_sprite_texture(const char *name);
Texture *get_sprite(const char *name, char phase, char rot, char *mirror_ret);
Texture *get_wall_texture(const char *name);

int get_texture_num(TextureType tt, const char *name);
Texture *get_texture_bynum(TextureType tt, int num);
int count_textures(TextureType tt);

int init_font(const char *fmt, int nchars, int firstchar);
Texture *get_font_texture(int fontnum, unsigned char ch);

Texture *get_misc_texture(const char *tname);

void init_textures(void);
void reset_textures(void);

typedef void (*SetPalFunc) (unsigned char idx,
			    unsigned char red,
			    unsigned char green,
			    unsigned char blue);

void set_playpal(int i, SetPalFunc func);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
