/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/alloc.c: Allocation of vertices, lines etc.  Array resizing.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"

#include "dumbworldb.h"
#include "private.h"

#define ALLOC_HUNK 256

static void prealloc_any(void **, struct dwdb_alloc *, size_t item_size,
			 unsigned newcount);

void
dwdb_prealloc_vertices(struct dwdb_level *lev, unsigned newcount)
{
   prealloc_any((void **) &lev->vertices, &lev->vertex_alloc,
		sizeof(struct dwdb_vertex),
		newcount);
}

void
dwdb_prealloc_sectors(struct dwdb_level *lev, unsigned newcount)
{
   prealloc_any((void **) &lev->sectors, &lev->sector_alloc,
		sizeof(struct dwdb_sector),
		newcount);
}

void
dwdb_prealloc_sides(struct dwdb_level *lev, unsigned newcount)
{
   prealloc_any((void **) &lev->sides, &lev->side_alloc,
		sizeof(struct dwdb_side),
		newcount);
}

void
dwdb_prealloc_lines(struct dwdb_level *lev, unsigned newcount)
{
   prealloc_any((void **) &lev->lines, &lev->line_alloc,
		sizeof(struct dwdb_line),
		newcount);
}

void
dwdb_prealloc_things(struct dwdb_level *lev, unsigned newcount)
{
   prealloc_any((void **) &lev->things, &lev->thing_alloc,
		sizeof(struct dwdb_thing),
		newcount);
}

static void
prealloc_any(void **datap, struct dwdb_alloc *alloc, size_t item_size,
	     unsigned newcount)
{
   if (alloc->free >= newcount)
      return;			/* the free chain has enough items */
   alloc->alloced = alloc->inited + newcount - alloc->free;
   *datap = safe_realloc(*datap, alloc->alloced * item_size);
}


unsigned
dwdb_new_vertex(struct dwdb_level *lev, int x, int y)
{
   int ind, next;
   if (lev->vertex_alloc.free) {
      /* Take from the beginning of the free chain.  */
      ind = lev->vertex_alloc.first_free;
      next = lev->vertices[ind].next_in_chain;
      if (next != -1)
	 lev->vertices[next].prev_in_chain = -1;
      lev->vertex_alloc.first_free = next;
      lev->vertex_alloc.free--;
   } else {
      /* Add to the end, perhaps allocating more.  */
      if (lev->vertex_alloc.inited == lev->vertex_alloc.alloced)
	 dwdb_prealloc_vertices(lev, ALLOC_HUNK);
      ind = lev->vertex_alloc.inited++;
   }
   /* Fill in the data.  */
   lev->vertices[ind].x = x;
   lev->vertices[ind].y = y;
   /* Add to the beginning of the used chain.  */
   next = lev->vertex_alloc.first_used;
   lev->vertices[ind].prev_in_chain = -1;
   lev->vertices[ind].next_in_chain = next;
   if (next != -1)
      lev->vertices[next].prev_in_chain = ind;
   lev->vertex_alloc.first_used = ind;
   /* Notify observers.  */
   _dwdb_after_new(lev, DWDB_OT_VERTEX, ind);
   return ind;
}

unsigned
dwdb_new_sector(struct dwdb_level *lev,
		const struct dwdb_sector *model)
{
   int ind, next;
   struct dwdb_sector *newsec;
   if (lev->sector_alloc.free) {
      /* Take from the beginning of the free chain.  */
      ind = lev->sector_alloc.first_free;
      next = lev->sectors[ind].next_in_chain;
      if (next != -1)
	 lev->sectors[next].prev_in_chain = -1;
      lev->sector_alloc.first_free = next;
      lev->sector_alloc.free--;
   } else {
      /* Add to the end, perhaps allocating more.  */
      if (lev->sector_alloc.inited == lev->sector_alloc.alloced)
	 dwdb_prealloc_sectors(lev, ALLOC_HUNK);
      ind = lev->sector_alloc.inited++;
   }
   newsec = &lev->sectors[ind];
   /* Fill in the data.  */
   *newsec = *model;
   newsec->first_side = -1;
   newsec->things = NULL;
   /* Add to the beginning of the used chain.  */
   next = lev->sector_alloc.first_used;
   newsec->prev_in_chain = -1;
   newsec->next_in_chain = next;
   if (next != -1)
      lev->sectors[next].prev_in_chain = ind;
   lev->sector_alloc.first_used = ind;
   /* Notify observers.  */
   _dwdb_after_new(lev, DWDB_OT_SECTOR, ind);
   return ind;
}

unsigned
dwdb_new_side(struct dwdb_level *lev, const struct dwdb_side *model)
{
   int ind, next;
   if (lev->side_alloc.free) {
      /* Take from the beginning of the free chain.  */
      ind = lev->side_alloc.first_free;
      next = lev->sides[ind].next_in_chain;
      if (next != -1)
	 lev->sides[next].prev_in_chain = -1;
      lev->side_alloc.first_free = next;
      lev->side_alloc.free--;
   } else {
      /* Add to the end, perhaps allocating more.  */
      if (lev->side_alloc.inited == lev->side_alloc.alloced)
	 dwdb_prealloc_sides(lev, ALLOC_HUNK);
      ind = lev->side_alloc.inited++;
   }
   /* Fill in the data.  */
   lev->sides[ind] = *model;
   /* The new side is not yet in any line.  */
   lev->sides[ind].line = -1;
   /* Add to the beginning of the chain of its sector.  */
   if (model->sector == -1) {
      next = lev->side_alloc.first_used;
      lev->sides[ind].prev_in_chain = -1;
      lev->sides[ind].next_in_chain = next;
      if (next != -1)
	 lev->sides[next].prev_in_chain = ind;
      lev->side_alloc.first_used = ind;
   } else {
      next = lev->sectors[model->sector].first_side;
      lev->sides[ind].prev_in_chain = -1;
      lev->sides[ind].next_in_chain = next;
      if (next != -1)
	 lev->sides[next].prev_in_chain = ind;
      lev->sectors[model->sector].first_side = ind;
   }
   /* Notify observers.  */
   _dwdb_after_new(lev, DWDB_OT_SIDE, ind);
   return ind;
}

unsigned
dwdb_new_line(struct dwdb_level *lev, const struct dwdb_line *model)
{
   int ind, next;
   if (lev->line_alloc.free) {
      /* Take from the beginning of the free chain.  */
      ind = lev->line_alloc.first_free;
      next = lev->lines[ind].next_in_chain;
      if (next != -1)
	 lev->lines[next].prev_in_chain = -1;
      lev->line_alloc.first_free = next;
      lev->line_alloc.free--;
   } else {
      /* Add to the end, perhaps allocating more.  */
      if (lev->line_alloc.inited == lev->line_alloc.alloced)
	 dwdb_prealloc_lines(lev, ALLOC_HUNK);
      ind = lev->line_alloc.inited++;
   }
   /* Fill in the data.  */
   lev->lines[ind] = *model;
   /* Link the sides back to this line, making sure that they aren't
    * in any other line yet.  */
   if (model->side[0] != -1) {
      _dwdb_detach_side(lev, model->side[0]);
      lev->sides[model->side[0]].line = ind;
   }
   if (model->side[1] != -1) {
      _dwdb_detach_side(lev, model->side[1]);
      lev->sides[model->side[1]].line = ind;
   }
   /* Add to the beginning of the used chain.  */
   next = lev->line_alloc.first_used;
   lev->lines[ind].prev_in_chain = -1;
   lev->lines[ind].next_in_chain = next;
   if (next != -1)
      lev->lines[next].prev_in_chain = ind;
   lev->line_alloc.first_used = ind;
   /* Notify observers.  */
   _dwdb_after_new(lev, DWDB_OT_LINE, ind);
   return ind;
}

unsigned
dwdb_new_thing(struct dwdb_level *lev, const struct dwdb_thing *model)
{
   int ind, next;
   struct dwdb_thing *newthg;
   if (lev->thing_alloc.free) {
      /* Take from the beginning of the free chain.  */
      ind = lev->thing_alloc.first_free;
      next = lev->things[ind].next_in_chain;
      if (next != -1)
	 lev->things[next].prev_in_chain = -1;
      lev->thing_alloc.first_free = next;
      lev->thing_alloc.free--;
   } else {
      /* Add to the end, perhaps allocating more.  */
      if (lev->thing_alloc.inited == lev->thing_alloc.alloced)
	 dwdb_prealloc_things(lev, ALLOC_HUNK);
      ind = lev->thing_alloc.inited++;
   }
   newthg = &lev->things[ind];
   /* Fill in the data.  */
   *newthg = *model;
   newthg->sectors = NULL;
   /* could add center_sector to sectors but that'll be done together
    * with the rest of them. */
   /* Add to the beginning of the used chain.  */
   next = lev->thing_alloc.first_used;
   lev->things[ind].prev_in_chain = -1;
   lev->things[ind].next_in_chain = next;
   if (next != -1)
      lev->things[next].prev_in_chain = ind;
   lev->thing_alloc.first_used = ind;
   /* Notify observers.  */
   _dwdb_after_new(lev, DWDB_OT_THING, ind);
   return ind;
}

// Local Variables:
// c-basic-offset: 3
// End:
