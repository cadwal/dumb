/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/loaddoom.c: Loading Doom levels.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#include <assert.h>

#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "dumbworldb.h"
#include "private.h"
#include "doomwad.h"

/* #define THINGS_DONT_WORK_YET */

static void load_doom_sectors(struct dwdb_level *);
static void load_doom_vertexes(struct dwdb_level *);
static void load_doom_linedefs(struct dwdb_level *);	/* loads sidedefs too */
#ifndef THINGS_DONT_WORK_YET
static void load_doom_things(struct dwdb_level *);
#endif
static Texture *lookup_doom_flattex(const char name[8]);
static Texture *lookup_doom_walltex(const char name[8]);

void
dwdb_init_doom(struct dwdb_level *lev, const char *name)
{
   dwdb_init(lev);
   logprintf(LOG_INFO, 'M', "Loading %s", name);
   dwdb_set_name(lev, name);
   dwdb_set_longname(lev, name);	/* for now */
   /* Sectors must be loaded before sidedefs because creating a
    * sidedef requires adding a link in the sector it belongs in.  */
   load_doom_sectors(lev);
   load_doom_vertexes(lev);
   load_doom_linedefs(lev);	/* loads sidedefs too */
#ifndef THINGS_DONT_WORK_YET
   load_doom_things(lev);
#endif
}

static void
load_doom_sectors(struct dwdb_level *lev)
{
   LumpNum lumpnum;
   unsigned count;
   const struct doom_sector *doom_sectors;
   unsigned ind;
   lumpnum = safe_lookup_lump("SECTORS", lev->name, NULL, LOG_ERROR);
   if (!LUMPNUM_OK(lumpnum))
      return;
   count = get_lump_len(lumpnum) / sizeof(struct doom_sector);
   doom_sectors = (const struct doom_sector *) load_lump(lumpnum);
   dwdb_prealloc_sectors(lev, count);
   for (ind = 0; ind < count; ind++) {
      const struct doom_sector *doom_sector = &doom_sectors[ind];
      struct dwdb_sector model;
      unsigned got_ind;
      model.floor_level = doom_sector->floor;
      model.ceiling_level = doom_sector->ceiling;
      model.ftexture = lookup_doom_flattex(doom_sector->ftexture);
      model.ctexture = lookup_doom_flattex(doom_sector->ctexture);
      model.light = doom_sector->light;
      model.type = doom_sector->type;
      model.tag = doom_sector->tag;
      /* Sectors must get the right numbers, as sidedefs refer to
       * them.  */
      got_ind = dwdb_new_sector(lev, &model);
      assert(got_ind == ind);
   }				/* for ind */
   free_lump(lumpnum);
}

static void
load_doom_vertexes(struct dwdb_level *lev)
{
   LumpNum lumpnum;
   unsigned count;
   const struct doom_vertex *doom_vertexes;
   unsigned ind;
   lumpnum = safe_lookup_lump("VERTEXES", lev->name, NULL, LOG_ERROR);
   if (!LUMPNUM_OK(lumpnum))
      return;
   count = get_lump_len(lumpnum) / sizeof(struct doom_vertex);
   doom_vertexes = (const struct doom_vertex *) load_lump(lumpnum);
   dwdb_prealloc_vertices(lev, count);
   for (ind = 0; ind < count; ind++) {
      /* Vertices must get the right numbers, as linedefs refer to
       * them.  */
      unsigned got_ind = dwdb_new_vertex(lev,
					 doom_vertexes[ind].x,
					 doom_vertexes[ind].y);
      assert(got_ind == ind);
   }
   free_lump(lumpnum);
}

static void
load_doom_linedefs(struct dwdb_level *lev)
{
   LumpNum linedefs_ln, sidedefs_ln;
   unsigned linedef_count, sidedef_count;
   const struct doom_linedef *doom_linedefs;
   const struct doom_sidedef *doom_sidedefs;
   unsigned lineind;
   linedefs_ln = safe_lookup_lump("LINEDEFS", lev->name, NULL, LOG_ERROR);
   if (!LUMPNUM_OK(linedefs_ln))
      return;
   sidedefs_ln = safe_lookup_lump("SIDEDEFS", lev->name, NULL, LOG_ERROR);
   if (!LUMPNUM_OK(sidedefs_ln))
      return;
   linedef_count = (get_lump_len(linedefs_ln)
		    / sizeof(struct doom_linedef));
   sidedef_count = (get_lump_len(sidedefs_ln)
		    / sizeof(struct doom_sidedef));
   doom_linedefs = (const struct doom_linedef *) load_lump(linedefs_ln);
   doom_sidedefs = (const struct doom_sidedef *) load_lump(sidedefs_ln);
   dwdb_prealloc_lines(lev, linedef_count);
   dwdb_prealloc_sides(lev, sidedef_count);
   for (lineind = 0; lineind < linedef_count; lineind++) {
      const struct doom_linedef *doom_linedef = &doom_linedefs[lineind];
      /* The line structure is built in LINEMODEL and then copied to
       * the actual level.  */
      struct dwdb_line linemodel;
      int foreback;
      linemodel.ver1 = doom_linedef->ver1;
      linemodel.ver2 = doom_linedef->ver2;
      linemodel.flags = 0;
      if (doom_linedef->flags & DOOM_LF_IMPASSABLE)
	 linemodel.flags |= DWDB_LF_IMPASSABLE;
      if (doom_linedef->flags & DOOM_LF_MIMPASSABLE)
	 linemodel.flags |= DWDB_LF_MIMPASSABLE;
      if (doom_linedef->flags & DOOM_LF_SECRET)
	 linemodel.flags |= DWDB_LF_SECRET;
      if (doom_linedef->flags & DOOM_LF_MBLKSOUND)
	 linemodel.flags |= DWDB_LF_MBLKSOUND;
      if (doom_linedef->flags & DOOM_LF_NOMAP)
	 linemodel.flags |= DWDB_LF_NOMAP;
      if (doom_linedef->flags & DOOM_LF_FORCEMAP)
	 linemodel.flags |= DWDB_LF_MAPPED;
      linemodel.type = doom_linedef->type;
      linemodel.tag = doom_linedef->tag;
      for (foreback = 0; foreback <= 1; foreback++) {
	 int doom_sideind = doom_linedef->side[foreback];
	 if (doom_sideind >= 0
	     && (unsigned) doom_sideind < sidedef_count) {
	    const struct doom_sidedef *doom_sidedef
	    = &doom_sidedefs[doom_sideind];
	    struct dwdb_side sidemodel;
	    sidemodel.upper.texture
		= lookup_doom_walltex(doom_sidedef->utexture);
	    sidemodel.middle.texture
		= lookup_doom_walltex(doom_sidedef->texture);
	    sidemodel.lower.texture
		= lookup_doom_walltex(doom_sidedef->ltexture);
	    sidemodel.upper.xoffset = sidemodel.middle.xoffset
		= sidemodel.lower.xoffset = doom_sidedef->xoffset;
	    sidemodel.upper.yoffset = sidemodel.middle.yoffset
		= sidemodel.lower.yoffset = doom_sidedef->yoffset;
	    sidemodel.upper.flags = sidemodel.middle.flags
		= sidemodel.lower.flags = 0;
	    if (doom_linedef->flags & DOOM_LF_UPUNPEGGED) {
	       sidemodel.upper.flags |= DWDB_STF_UNPEGGED;
	       sidemodel.middle.flags |= DWDB_STF_UNPEGGED;
	    }
	    if (doom_linedef->flags & DOOM_LF_LOUNPEGGED) {
	       sidemodel.lower.flags |= DWDB_STF_UNPEGGED;
	    }
	    if (doom_linedef->flags & DOOM_LF_POSTER) {
	       sidemodel.upper.flags |= DWDB_STF_POSTER;
	       sidemodel.middle.flags |= DWDB_STF_POSTER;
	       sidemodel.lower.flags |= DWDB_STF_POSTER;
	    }
	    sidemodel.sector = doom_sidedef->sector;
	    linemodel.side[foreback] = dwdb_new_side(lev, &sidemodel);
	 } else
	    linemodel.side[foreback] = -1;
      }				/* for foreback */
      dwdb_new_line(lev, &linemodel);
   }				/* for lineind */
   free_lump(linedefs_ln);
   free_lump(sidedefs_ln);
}

#ifndef THINGS_DONT_WORK_YET
static void
load_doom_things(struct dwdb_level *lev)
{
   LumpNum things_ln;
   unsigned thing_count;
   const struct doom_thing *doom_things;
   unsigned ind;
   things_ln = safe_lookup_lump("THINGS", lev->name, NULL, LOG_ERROR);
   if (!LUMPNUM_OK(things_ln))
      return;
   thing_count = (get_lump_len(things_ln)
		  / sizeof(struct doom_thing));
   doom_things = (const struct doom_thing *) load_lump(things_ln);
   dwdb_prealloc_things(lev, thing_count);
   for (ind = 0; ind < thing_count; ind++) {
      const struct doom_thing *doom_thing = &doom_things[ind];
      struct dwdb_thing model;
      unsigned dl_thingind;
/*       model.proto = find_thing_proto(doom_thing->type); */
/*       if (!model.proto) */
/*       continue;              /* don't add an empty thing */
      /* okay, now we know for sure that this thing has a proto */
      model.x = doom_thing->x;
      model.y = doom_thing->y;
      /* don't initialize z until we know the sector */
      model.angle = fixmul(INT_TO_FIXED(doom_thing->angle) / 360, FIXED_2PI);
      model.elev = 0;
      model.center_sector = dwdb_find_sector_2d(lev, model.x, model.y);
      /* now the z */
      if (model.center_sector != -1) {
	 const struct dwdb_sector *sector
	 = &lev->sectors[model.center_sector];
	 /* FIXME: hotspots */
/*       if (model.proto->flags & PT_HANGING) */
/*          model.z = sector->ceiling - model.proto->height; */
/*       else if (model.proto->flags & PT_CAN_FLY) */
/*          model.z = (sector->ceiling + sector->floor) / 2; */
/*       else */
	 model.z = sector->floor_level;
      } else			/* sector not ok */
	 model.z = 0;		/* a safe default */
      model.flags = (DWDB_TF_IN_SINGLE
		     | DWDB_TF_IN_COOP
		     | DWDB_TF_IN_DM);
/*       if (model.proto->flags & PT_PHANTOM) */
/*       model.flags |= DWDB_TF_NOCOLLISION; */
/*       if (model.proto->flags & PT_PINVIS) */
/*       model.flags |= DWDB_TF_PARTINVIS; */
      if (doom_thing->flags & DOOM_TF_DEAF)
	 /* FIXME: deaf monsters */ ;
      if (doom_thing->flags & DOOM_TF_SKILL12)
	 model.flags |= DWDB_TF_IN_SKILL12;
      if (doom_thing->flags & DOOM_TF_SKILL3)
	 model.flags |= DWDB_TF_IN_SKILL3;
      if (doom_thing->flags & DOOM_TF_SKILL45)
	 model.flags |= DWDB_TF_IN_SKILL45;
      if (doom_thing->flags & DOOM_TF_MULTIONLY)
	 model.flags &= ~DWDB_TF_IN_SINGLE;
      dl_thingind = dwdb_new_thing(lev, &model);
      if (model.center_sector != -1)
	 _dwdb_new_thingsec(lev, dl_thingind, model.center_sector);
      /* FIXME: read diameter & height from proto and add other sectors */
   }				/* for thing */
}
#endif

static Texture *
lookup_doom_flattex(const char name[8])
{
   char name0[8 + 1];
   memcpy(name0, name, 8);
   name0[8] = '\0';
   /* Actually, get_flat_texture() doesn't require the '\0'.  But I
    * add it in case some later version supports textures with longer
    * names.  */
   /* FIXME: sky */
   return get_flat_texture(name0);
}

static Texture *
lookup_doom_walltex(const char name[8])
{
   char name0[8 + 1];
   memcpy(name0, name, 8);
   name0[8] = '\0';
   if (name0[0] == '\0' || !strcmp(name0, "-"))
      return NULL;
   /* Actually, get_wall_texture() doesn't require the '\0'.  But I
    * add it in case some later version supports textures with longer
    * names.  */
   return get_wall_texture(name0);
}

// Local Variables:
// c-basic-offset: 3
// End:
