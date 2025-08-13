/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/animtex.h: Animated textures.
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

#ifndef ANIMTEX_H
#define ANIMTEX_H

#include "libdumb/animtexstruct.h"
#include "libdumb/texture.h"

typedef short AnimTexNum;

AnimTexNum get_animtex(const char *name);
int update_anim_state(AnimTexNum num, int seqnum, int tickspassed);
Texture *get_anim_texture(AnimTexNum num, int seqnum);
void init_animtex(void);
void reset_animtex(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
