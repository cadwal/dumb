/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/savedoom.c: Saving Doom levels.
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

#include <assert.h>		/* guess what */
#include <stdlib.h>		/* free() */
#include <string.h>		/* strncpy() */

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadwr.h"
#include "doomwad.h"
#include "dumbworldb.h"

/* Define to create an array for converting dumbworldb line indices to
 * saved ones.  Similar arrays are always created for vertices,
 * sectors and sides.  Lines don't normally need them, as line numbers
 * aren't referred to.  */
#undef CREATE_LINEMAP

/* I could implement this with static variables but I don't want to.
 * Let's say this is for reentrancy.  */
struct savedoom_ctx {
   const struct dwdb_level *level;
   WADWR *wadwr;
   /* vertexmap[3] is the number of the Doom vertex corresponding to
    * vertex 3 in LEVEL.  */
   int *vertexmap;
   int *sectormap;
   int *sidemap;
#ifdef CREATE_LINEMAP
   int *linemap;
#endif
};

static void save_doom_vertexes(struct savedoom_ctx *);
static void save_doom_sectors(struct savedoom_ctx *);
static void save_doom_sidedefs(struct savedoom_ctx *);
static void save_doom_linedefs(struct savedoom_ctx *);
static void store_texname(char doomname[8], const Texture *);

void
dwdb_save_doom(const struct dwdb_level *lev, const char *filename)
{
   struct savedoom_ctx ctx;
   logprintf(LOG_INFO, 'M', _("Saving level %s to %s"), lev->name, filename);
   ctx.level = lev;
   ctx.wadwr = wadwr_open(filename, 'P');	/* PWAD */
   wadwr_lump(ctx.wadwr, lev->name);
   save_doom_vertexes(&ctx);
   save_doom_sectors(&ctx);
   save_doom_sidedefs(&ctx);
   save_doom_linedefs(&ctx);
   wadwr_close(ctx.wadwr);
   free(ctx.vertexmap);
   free(ctx.sectormap);
   free(ctx.sidemap);
#ifdef CREATE_LINEMAP
   free(ctx.linemap);
#endif
   /* ctx.level is const, so don't free it.  */
}

static void
save_doom_vertexes(struct savedoom_ctx *ctx)
{
   struct doom_vertex *dvertexes;
   int ind, dind, count, dcount;
   /* count = how many exist (free or not) */
   count = ctx->level->vertex_alloc.inited;
   /* dcount = how many will be saved */
   dcount = count - ctx->level->vertex_alloc.free;
   ctx->vertexmap = safe_malloc(count * sizeof(int));
   dvertexes = safe_malloc(dcount * sizeof(struct doom_vertex));
   /* Clear the map.  */
   for (ind = 0; ind < count; ind++)
      ctx->vertexmap[ind] = -1;
   /* Traverse lists, encode vertices and add them to the map.
    * The map will be used later when encoding lines.  */
   dind = 0;
   for (ind = ctx->level->vertex_alloc.first_used;
	ind != -1;
	ind = ctx->level->vertices[ind].next_in_chain) {
      dvertexes[dind].x = ctx->level->vertices[ind].x;
      dvertexes[dind].y = ctx->level->vertices[ind].y;
      ctx->vertexmap[ind] = dind++;
   }
   assert(dind == dcount);
   /* Write them out.  */
   wadwr_lump(ctx->wadwr, "VERTEXES");
   wadwr_write(ctx->wadwr, dvertexes, dcount * sizeof(struct doom_vertex));
   /* Local cleanup.  */
   free(dvertexes);
}

static void
save_doom_sectors(struct savedoom_ctx *ctx)
{
   struct doom_sector *dsectors;
   int ind, dind, count, dcount;
   /* count = how many exist (free or not) */
   count = ctx->level->sector_alloc.inited;
   /* dcount = how many will be saved */
   dcount = count - ctx->level->sector_alloc.free;
   ctx->sectormap = safe_malloc(count * sizeof(int));
   dsectors = safe_malloc(dcount * sizeof(struct doom_sector));
   /* Clear the map.  */
   for (ind = 0; ind < count; ind++)
      ctx->sectormap[ind] = -1;
   /* Traverse lists, encode vertices and add them to the map.
    * The map will be used later when encoding lines.  */
   dind = 0;
   for (ind = ctx->level->sector_alloc.first_used;
	ind != -1;
	ind = ctx->level->sectors[ind].next_in_chain) {
      const struct dwdb_sector *sector = &ctx->level->sectors[ind];
      dsectors[dind].floor = sector->floor_level;
      dsectors[dind].ceiling = sector->ceiling_level;
      store_texname(dsectors[dind].ftexture, sector->ftexture);
      store_texname(dsectors[dind].ctexture, sector->ctexture);
      dsectors[dind].light = sector->light;
      dsectors[dind].type = sector->type;
      dsectors[dind].tag = sector->tag;
      ctx->sectormap[ind] = dind++;
   }
   assert(dind == dcount);
   /* Write them out.  */
   wadwr_lump(ctx->wadwr, "SECTORS");
   wadwr_write(ctx->wadwr, dsectors, dcount * sizeof(struct doom_sector));
   /* Local cleanup.  */
   free(dsectors);
}

static void
save_doom_sidedefs(struct savedoom_ctx *ctx)
{
   struct doom_sidedef *dsidedefs;
   int ind, dind, count, dcount;
   int sectind;
   /* count = how many exist (free or not) */
   count = ctx->level->side_alloc.inited;
   /* dcount = how many will be saved */
   dcount = count - ctx->level->side_alloc.free;
   ctx->sidemap = safe_malloc(count * sizeof(int));
   dsidedefs = safe_malloc(dcount * sizeof(struct doom_sidedef));
   /* Clear the map.  */
   for (ind = 0; ind < count; ind++)
      ctx->sidemap[ind] = -1;
   /* Traverse lists, encode vertices and add them to the map.
    * The map will be used later when encoding lines.
    * This is more complicated than with vertices and sectors because
    * the used sides are listed by sector; there is no single list.
    * This will fail to find those sides that aren't in any sector but
    * I don't think that matters.  */
   dind = 0;
   for (sectind = ctx->level->sector_alloc.first_used;
	sectind != -1;
	sectind = ctx->level->sectors[sectind].next_in_chain) {
      for (ind = ctx->level->sectors[sectind].first_side;
	   ind != -1;
	   ind = ctx->level->sides[ind].next_in_chain) {
	 struct dwdb_side *side = &ctx->level->sides[ind];
	 /* Doom supports only one pair of offsets; I arbitrarily pick
	  * the ones from the middle texture.  */
	 dsidedefs[dind].xoffset = side->middle.xoffset;
	 dsidedefs[dind].yoffset = side->middle.yoffset;
	 store_texname(dsidedefs[dind].utexture, side->upper.texture);
	 store_texname(dsidedefs[dind].texture, side->middle.texture);
	 store_texname(dsidedefs[dind].ltexture, side->lower.texture);
	 dsidedefs[dind].sector = (side->sector == -1
				   ? -1
				   : ctx->sectormap[side->sector]);
	 ctx->sidemap[ind] = dind++;
      }
   }
   /* Can't assert dind==dcount, as sides that aren't in any sector
    * weren't found.  If it is necessary to save them, you can make
    * this function iterate from ctx->level->side_alloc.first_side
    * too.  */
   assert(dind <= dcount);
   /* Write them out.  */
   wadwr_lump(ctx->wadwr, "SIDEDEFS");
   wadwr_write(ctx->wadwr, dsidedefs, dcount * sizeof(struct doom_sidedef));
   /* Local cleanup.  */
   free(dsidedefs);
}

static void
save_doom_linedefs(struct savedoom_ctx *ctx)
{
   struct doom_linedef *dlinedefs;
   int ind, dind, count, dcount;
   /* count = how many exist (free or not) */
   count = ctx->level->line_alloc.inited;
   /* dcount = how many will be saved */
   dcount = count - ctx->level->line_alloc.free;
#ifdef CREATE_LINEMAP
   ctx->linemap = safe_malloc(count * sizeof(int));
#endif
   dlinedefs = safe_malloc(dcount * sizeof(struct doom_linedef));
#ifdef CREATE_LINEMAP
   /* Clear the map.  */
   for (ind = 0; ind < count; ind++)
      ctx->linemap[ind] = -1;
#endif
   /* Traverse lists, encode vertices and add them to the map.
    * The map will be used later when encoding lines.  */
   dind = 0;
   for (ind = ctx->level->line_alloc.first_used;
	ind != -1;
	ind = ctx->level->lines[ind].next_in_chain) {
      const struct dwdb_line *line = &ctx->level->lines[ind];
      unsigned dflags = 0;
      int lineside;
      /* Vertex numbers can never be -1.  */
      dlinedefs[dind].ver1 = ctx->vertexmap[line->ver1];
      dlinedefs[dind].ver2 = ctx->vertexmap[line->ver2];
      if (line->flags & DWDB_LF_IMPASSABLE)
	 dflags |= DOOM_LF_IMPASSABLE;
      if (line->flags & DWDB_LF_MIMPASSABLE)
	 dflags |= DOOM_LF_MIMPASSABLE;
      if (line->flags & DWDB_LF_SECRET)
	 dflags |= DOOM_LF_SECRET;
      if (line->flags & DWDB_LF_MBLKSOUND)
	 dflags |= DOOM_LF_MBLKSOUND;
      if (line->flags & DWDB_LF_NOMAP)
	 dflags |= DOOM_LF_NOMAP;
      if (line->flags & DWDB_LF_MAPPED)
	 dflags |= DOOM_LF_FORCEMAP;
#ifdef DWDB_LF_TWOSIDED
      if (line->flags & DWDB_LF_TWOSIDED)
	 dflags |= DOOM_LF_TWOSIDED;
#else  /* cheat it */
      if (line->side[1] != -1)
	 dflags |= DOOM_LF_TWOSIDED;
#endif
      /* This loop handles sides and the rest of flags.  */
      for (lineside = 0; lineside <= 1; lineside++) {
	 int sideind = line->side[lineside];
	 if (sideind == -1)
	    dlinedefs[dind].side[lineside] = -1;
	 else {
	    struct dwdb_side *side = &ctx->level->sides[sideind];
	    dlinedefs[dind].side[lineside] = ctx->sidemap[sideind];
	    if (side->upper.flags
		& DWDB_STF_UNPEGGED)
	       dflags |= DOOM_LF_UPUNPEGGED;
	    if ((side->middle.flags | side->lower.flags)
		& DWDB_STF_UNPEGGED)
	       dflags |= DOOM_LF_LOUNPEGGED;
	    if ((side->upper.flags | side->middle.flags | side->lower.flags)
		& DWDB_STF_POSTER)
	       dflags |= DOOM_LF_POSTER;
	 }			/* if side!=-1 */
      }				/* for lineside */
      dlinedefs[dind].flags = dflags;
      dlinedefs[dind].type = line->type;
      dlinedefs[dind].tag = line->tag;
#ifdef CREATE_LINEMAP
      ctx->linemap[ind] = dind++;
#else
      dind++;
#endif
   }
   assert(dind == dcount);
   /* Write them out.  */
   wadwr_lump(ctx->wadwr, "LINEDEFS");
   wadwr_write(ctx->wadwr, dlinedefs, dcount * sizeof(struct doom_linedef));
   /* Local cleanup.  */
   free(dlinedefs);
}

static void
store_texname(char doomname[8], const Texture *tex)
{
   /* FIXME: sky */
   if (tex == NULL)
      strncpy(doomname, "-", 8);
   else
      strncpy(doomname, tex->name, 8);
}

// Local Variables:
// c-basic-offset: 3
// End:
