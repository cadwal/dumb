/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/useitem.c: Using inventory items and weapons.
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "things.h"
#include "gettable.h"


#define TARG_ARC				\
   (ldthingd(ld)[t].proto->aim_arc		\
    + ldthingd(ld)[t].proto->shootarc_h		\
    + FIXED_PI/16)

/*#define TARG_ARC (FIXED_PI/4) */

int
use_item(LevData *ld, int pl, const Gettable *gt)
{
   if (gt->flags & GK_WEPSELECT) {
      int t = ld->plwep[pl];
      /*logprintf(LOG_DEBUG, 'U', "use_item: pl=%d t=%d", pl, t); */
      if (t < 0 || ldthingd(ld)[t].proto == NULL)
	 return 0;
      /* only do autotarget + shoot if both player and bogothing are ready */
      if (thing_sig_ok(ld, ld->player[pl], TS_SHOOT) &&
	  thing_sig_ok(ld, t, TS_SHOOT)) {
	 thing_autotarget(ld, t, TARG_ARC);
	 thing_send_sig(ld, ld->player[pl], TS_SHOOT);
	 thing_send_sig(ld, t, TS_SHOOT);
	 return 1;
      }
   }
   if (gt->flags & GK_SPESELECT)
      return 1;
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
