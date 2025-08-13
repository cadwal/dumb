/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/del.c: Deletion of items in struct dwdb_level.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"

#include "dumbworldb.h"
#include "private.h"

/* TODO: Ensure that nothing refers to the deleted items.  (Already
 * done for sides and sectors; lines don't need this; only vertices
 * are left.)  */

void
dwdb_del_vertex(struct dwdb_level *lev, unsigned ind)
{
   int next, prev;
   /* Notify observers.  */
   _dwdb_before_del(lev, DWDB_OT_VERTEX, ind);
   /* Take it off the chain it is now */
   next = lev->vertices[ind].next_in_chain;
   prev = lev->vertices[ind].prev_in_chain;
   if (next != -1)
      lev->vertices[next].prev_in_chain = prev;
   if (prev != -1)
      lev->vertices[prev].next_in_chain = next;
   if (lev->vertex_alloc.first_used == (int) ind)
      lev->vertex_alloc.first_used = next;
   /* Put it in the free chain */
   next = lev->thing_alloc.first_free;
   lev->vertices[ind].next_in_chain = next;
   if (next != -1)
      lev->vertices[next].prev_in_chain = ind;
   lev->vertices[ind].prev_in_chain = -1;
   lev->vertex_alloc.first_free = ind;
   lev->vertex_alloc.free++;
}

void
dwdb_del_sector(struct dwdb_level *lev, unsigned ind)
{
   int next, prev;
   struct dwdb_sector *sector = &lev->sectors[ind];
   if (sector->first_side != -1)
      logprintf('M', LOG_ERROR, _("deleting sector %u which has sides"), ind);
   /* Notify observers.  */
   _dwdb_before_del(lev, DWDB_OT_SECTOR, ind);
   /* Delete all thingsecs referring to this sector.  */
   while (sector->things != NULL) {
      int thingind = sector->things->thing;
      if (lev->things[thingind].center_sector == (int) ind) {
	 logprintf('M', LOG_WARNING, _("sector deleted under thing %d"),
		   thingind);
	 lev->things[thingind].center_sector = -1;
      }
      _dwdb_del_thingsec(lev, sector->things);
      /* This changed sector->things because the deleted thingsec was
       * the first one in the list.  */
   }
   /* Take the sector off the chain the it is now in.  */
   next = sector->next_in_chain;
   prev = sector->prev_in_chain;
   if (next != -1)
      lev->sectors[next].prev_in_chain = prev;
   if (prev != -1)
      lev->sectors[prev].next_in_chain = next;
   if (lev->sector_alloc.first_used == (int) ind)
      lev->sector_alloc.first_used = next;
   /* Put it in the free chain.  */
   next = lev->sector_alloc.first_free;
   sector->next_in_chain = next;
   if (next != -1)
      lev->sectors[next].prev_in_chain = ind;
   sector->prev_in_chain = -1;
   lev->sector_alloc.first_free = ind;
   lev->sector_alloc.free++;
}

void
dwdb_del_side(struct dwdb_level *lev, unsigned ind)
{
   int next, prev, sector;
   /* Notify observers.  */
   _dwdb_before_del(lev, DWDB_OT_SIDE, ind);
   /* Take it off the line it is in */
   _dwdb_detach_side(lev, ind);
   /* Take it off the chain it is now in */
   next = lev->sides[ind].next_in_chain;
   prev = lev->sides[ind].prev_in_chain;
   sector = lev->sides[ind].sector;
   if (next != -1)
      lev->sides[next].prev_in_chain = prev;
   if (prev != -1)
      lev->sides[prev].next_in_chain = next;
   if (sector == -1) {
      if (lev->side_alloc.first_used == (int) ind)
	 lev->side_alloc.first_used = next;
   } else {
      if (lev->sectors[sector].first_side == (int) ind)
	 lev->sectors[sector].first_side = next;
   }
   /* Put it in the free chain */
   next = lev->side_alloc.first_free;
   lev->sides[ind].next_in_chain = next;
   if (next != -1)
      lev->sides[next].prev_in_chain = ind;
   lev->sides[ind].prev_in_chain = -1;
   lev->side_alloc.first_free = ind;
   lev->side_alloc.free++;
}

void
dwdb_del_line(struct dwdb_level *lev, unsigned ind)
{
   int next, prev;
   /* Notify observers.  */
   _dwdb_before_del(lev, DWDB_OT_LINE, ind);
   /* Detach the sides of the line.  But don't delete them.  */
   _dwdb_detach_side(lev, lev->lines[ind].side[0]);
   _dwdb_detach_side(lev, lev->lines[ind].side[1]);
   /* Take it off the chain it is now in */
   next = lev->lines[ind].next_in_chain;
   prev = lev->lines[ind].prev_in_chain;
   if (next != -1)
      lev->lines[next].prev_in_chain = prev;
   if (prev != -1)
      lev->lines[prev].next_in_chain = next;
   if (lev->line_alloc.first_used == (int) ind)
      lev->line_alloc.first_used = next;
   /* Put it in the free chain */
   next = lev->line_alloc.first_free;
   lev->lines[ind].next_in_chain = next;
   if (next != -1)
      lev->lines[next].prev_in_chain = ind;
   lev->lines[ind].prev_in_chain = -1;
   lev->line_alloc.first_free = ind;
   lev->line_alloc.free++;
}

void
dwdb_del_thing(struct dwdb_level *lev, unsigned ind)
{
   int next, prev;
   struct dwdb_thing *thing = &lev->things[ind];
   /* Notify observers.  */
   _dwdb_before_del(lev, DWDB_OT_THING, ind);
   /* Delete all thingsecs for this thing */
   while (thing->sectors) {
      _dwdb_del_thingsec(lev, thing->sectors);
   }
   /* Take it off the chain it is now in */
   next = thing->next_in_chain;
   prev = thing->prev_in_chain;
   if (next != -1)
      lev->things[next].prev_in_chain = prev;
   if (prev != -1)
      lev->things[prev].next_in_chain = next;
   if (lev->thing_alloc.first_used == (int) ind)
      lev->thing_alloc.first_used = next;
   /* FIXME: sleeping chain? */
   /* Put it in the free chain */
   next = lev->thing_alloc.first_free;
   thing->next_in_chain = next;
   if (next != -1)
      lev->things[next].prev_in_chain = ind;
   thing->prev_in_chain = -1;
   lev->thing_alloc.first_free = ind;
   lev->thing_alloc.free++;
}

// Local Variables:
// c-basic-offset: 3
// End:
