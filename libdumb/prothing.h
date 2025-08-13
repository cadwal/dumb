/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/prothing.h: ProtoThings.
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

#ifndef PROTHING_H
#define PROTHING_H

#include "libdumb/prothingstruct.h"
#include "libdumb/texture.h"

/* XProtoThing needs these functions.  */
void init_prothings(void);
void reset_prothings(void);
const ProtoThing *find_protothing(int id);
const ThingPhase *find_first_thingphase(int id);
#define find_thingphase(id,offset) (find_first_thingphase(id)+(offset))
Texture *find_phase_sprite(const ProtoThing *proto,
			   int phase,
			   char rot);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
