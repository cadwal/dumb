/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/updmap.h: Updating the map.  Event queue.
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

#ifndef UPDMAP_H
#define UPDMAP_H

#include "libdumbutil/fixed.h"
#include "levdata.h"

void update_map(LevData *ld, int tickspassed);

void change_sector_type(LevData *ld, int sector, int type);

MapEvent *insert_event(LevData *ld,
		       MapLumpType lumptype,
		       MapEventType etype,
		       int entity,
		       const void *key);

int unqueue_event(LevData *ld,
		  MapLumpType lumptype,
		  MapEventType etype,
		  int entity,
		  const void *key /* NULL means any key */ );

/* called before loading next level */
void unqueue_all_events(LevData *ld);

MapEvent *find_active_event(LevData *ld,
			    MapLumpType lumptype,
			    MapEventType etype,
			    int entity);

int find_event(LevData *ld,
	       MapLumpType lumptype,
	       MapEventType etype,
	       int entity);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
