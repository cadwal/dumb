/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/view.c: Views.  Used for rendering and sounds.
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

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "libdumbutil/fixed.h"
#include "libdumbutil/log.h"
#include "view.h"

void
init_view(View *v)
{
   memset(v, 0, sizeof(View));
   v->arc = VIEW_ARC;
   v->eye_distance = VIEW_EYE;
   v->view_plane_size = FIXED_ONE;
/*     fixmul(FLOAT_TO_FIXED(tan(FIXED_TO_FLOAT(v->arc) / 2.0)),
   v->eye_distance); */
/*   logprintf(LOG_INFO, 'R', "init_view: arc=%f focal dist=%f width=%f",
   FIXED_TO_FLOAT(v->arc),
   FIXED_TO_FLOAT(v->eye_distance),
   FIXED_TO_FLOAT(v->view_plane_size)
   ); */
   v->sector = -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
