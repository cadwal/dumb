
#ifndef UPDMAP_H
#define UPDMAP_H

#include "libdumbutil/fixed.h"
#include "levdata.h"

void update_map(LevData *ld,int tickspassed);

void change_sector_type(LevData *ld,int sector,int type);

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
