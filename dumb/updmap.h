
#ifndef UPDMAP_H
#define UPDMAP_H

#include "lib/fixed.h"
#include "levdata.h"

void update_map(LevData *ld,int tickspassed);

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

MapEvent *find_active_event(LevData *ld,
			    MapLumpType lumptype,
			    MapEventType etype,
			    int entity);

int find_event(LevData *ld,
	       MapLumpType lumptype,
	       MapEventType etype,
	       int entity);

#endif
