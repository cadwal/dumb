/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/bogothing.c: Bogothings.
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

#include <stddef.h>

#include "draw.h"
#include "things.h"
#include "bogothing.h"

void
draw_bogothings(const LevData *ld, void *fb, int width, int height)
{
   int i;
   ThingDyn *td = ldthingd(ld);
   for (i = 0; i < ldnthings(ld); i++, td++) {
      if (!td->proto || !(td->proto->flags & PT_BOGUS))
	 continue;
      if (td->owner != ld->player[ld->localplayer])
	 continue;
      thing_rotate_image(ld, i, 0);
      if (td->image)
	 draw(fb, td->image,
	      (width - td->image->width) / 2,
	      height - td->image->height);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
