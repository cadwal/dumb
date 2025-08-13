/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/view.h: Views.  Used for rendering and sounds.
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

#ifndef VIEW_H
#define VIEW_H

#include "libdumbutil/fixed.h"

typedef struct {
   fixed x, y, height;
   fixed angle;
   fixed arc;
   fixed view_plane_size;
   fixed eye_distance;
   fixed horizon;
   int sector;
} View;

typedef struct {
   fixed angle, offset;
} ViewTrans;

#define VIEW_ARC FIXED_HALF_PI
#define VIEW_EYE FIXED_ONE

void init_view(View *v);

#endif /* VIEW_H */

// Local Variables:
// c-basic-offset: 3
// End:
