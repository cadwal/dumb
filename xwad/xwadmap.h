/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/xwadmap.h: Coordinate transformation macros.
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

#ifndef XWADMAP_H
#define XWADMAP_H

#define MYSHR(x,y) ((y)<0?((x)<<-(y)):((x)>>(y)))

/* scale from map to screen */
#define SCALE(i) MYSHR(i,inst->scale)
#define UNSCALE(i) MYSHR(i,-inst->scale)

/* convert map coords to screen ones */
#define VER_SCREENX(v) (SCALE((int)(v)->x-inst->xoffset)+inst->map_width/2)
#define VER_SCREENY(v) (SCALE(-(int)(v)->y+inst->yoffset)+inst->map_height/2)

#define TH_SCREENX VER_SCREENX
#define TH_SCREENY VER_SCREENY

/* convert screen to map */
#define MAPX(x) (UNSCALE((x)-inst->map_width/2)+inst->xoffset)
#define MAPY(x) (-(UNSCALE((x)-inst->map_height/2)-inst->yoffset))

/* as this is, things are a bit strange around the origin: should fix! */
/* #define GRIDIFY(x) ((x/inst->gridsize)*inst->gridsize) */
#define GRIDIFY(x) ((x)<0 \
		    ? ((x)-inst->gridsize/2)/inst->gridsize*inst->gridsize \
		    : ((x)+inst->gridsize/2)/inst->gridsize*inst->gridsize)

#endif

// Local Variables:
// c-basic-offset: 3
// End:
