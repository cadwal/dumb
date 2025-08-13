/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/dumbworldb.h: How levels are stored in memory.
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

#ifndef DWDB_H
#define DWDB_H

#include "libdumbutil/fixed.h"

#include "libdumb/texture.h"	/* FIXME */


/*--------------------------------------------------------------------
 * Structures
 */

struct dwdb_level;

/* This defines the internal format of levels in DUMB.  When a level
 * is loaded, it is converted to this format.
 *
 * Various fields here are indices to tables.  They might be turned to
 * pointers some day.
 *
 * Do not change the values in these structures directly.  Use the
 * functions instead.  They'll notify any observers.
 *
 * Many of the structures here have members next_in_chain and
 * prev_in_chain.  These are -1 if the item is last or first,
 * respectively.  */

struct dwdb_vertex {
   int x, y;
   /* There is one chain for all used vertices and another for free
    * ones.  */
   int next_in_chain, prev_in_chain;
};

struct dwdb_thingsec {
   unsigned thing, sector;
   struct dwdb_thingsec *next_thing, *prev_thing;
   struct dwdb_thingsec *next_sector, *prev_sector;
};

struct dwdb_sector {
   int floor_level, ceiling_level;
   Texture *ftexture, *ctexture;
   int light;
   int type;
   int tag;
   /* private: */
   /* The rest of the members are not taken from the model when
    * creating a new sector.  */
   int first_side;
   struct dwdb_thingsec *things;
   /* There is one chain for all used sectors and another for free
    * ones.  */
   int next_in_chain, prev_in_chain;
};

struct dwdb_sidetex {
   Texture *texture;
   int xoffset, yoffset;
   unsigned flags;
   /* These aren't chained, as they are only used as parts of sides.  */
};

/* dwdb_sidetex::flags */
#define DWDB_STF_UNPEGGED 0x0001
#define DWDB_STF_POSTER   0x0002

struct dwdb_side {
   struct dwdb_sidetex upper, middle, lower;
   int sector;
   /* private: */
   /* The rest of the members are not taken from the model when
    * creating a new side.  */
   /* A side can be in three states:
    * - free: chained in lev->side_alloc.first_free
    * - used (sector==-1): chained in lev->side_alloc.first_used
    * - used (sector!=-1): chained in lev->sectors[sector].first_side
    *
    * Also, even if a side is used, it can be detached, meaning that
    * line==-1.  */
   int next_in_chain, prev_in_chain;
   int line;			/* which line this side belongs to */
};

struct dwdb_line {
   int ver1, ver2;
   unsigned flags;
   int type;
   int tag;
   int side[2];
   /* private: */
   /* There is one chain for used lines and another for free lines.  */
   int next_in_chain, prev_in_chain;
};

/* dwdb_line::flags */
#define DWDB_LF_IMPASSABLE  0x0001
#define DWDB_LF_MIMPASSABLE 0x0002
#define DWDB_LF_SECRET      0x0004
#define DWDB_LF_MBLKSOUND   0x0008
#define DWDB_LF_NOMAP       0x0010
#define DWDB_LF_MAPPED      0x0020	/* set when player sees this line */

struct dwdb_thing_vtbl {
   void (*init) (struct dwdb_level *, unsigned ind);
   void (*fini) (struct dwdb_level *, unsigned ind);
};

extern const struct dwdb_thing_vtbl dwdb_dummy_thing_vtbl;

struct dwdb_thing {
   // const ProtoThing *proto;
   fixed x, y, z;
   fixed angle;			/* radians */
   fixed elev;
   int center_sector;
   const struct dwdb_thing_vtbl *vtbl;
   void *data;
   /* the rest is not taken from the model */
   struct dwdb_thingsec *sectors;
   unsigned flags;
   int next_in_chain, prev_in_chain;
};

/* dwdb_thing::flags */
#define DWDB_TF_NOCOLLISION 0x0001	/* spispopd */
#define DWDB_TF_PARTINVIS   0x0002	/* partial invisibility */
#define DWDB_TF_DEAF        0x0004	/* doesn't react to sounds */
#define DWDB_TF_IN_SKILL12  0x0400	/* appears in skill levels 1-2 */
#define DWDB_TF_IN_SKILL3   0x0800	/* appears in skill level 3 */
#define DWDB_TF_IN_SKILL45  0x1000	/* appears in skill levels 4-5 */
#define DWDB_TF_IN_SINGLE   0x2000	/* appears in single-player games */
#define DWDB_TF_IN_COOP     0x4000	/* appears in cooperative games */
#define DWDB_TF_IN_DM       0x8000	/* appears in DumbMatch games */

struct dwdb_alloc {
   /* Each structure pointer in struct dwdb_level points to an array of
    * items.
    *
    * ALLOCED is the size of that array.  When so many new items are
    * created that the array can't hold them all, it is resized and
    * ALLOCED is updated.
    *
    * INITED is how many items in the array have been used.  FREE is
    * how many of INITED aren't currently in use.  FIRST_FREE points
    * to the first free item or is -1 if there aren't any.  Each free
    * item contains a pointer to the next free one.
    *
    * FIRST_USED is not used at all for sides, which are chained by
    * sector instead.  */
   unsigned alloced;
   unsigned inited;
   unsigned free;
   int first_free, first_used;
};

struct dwdb_observer;

struct dwdb_level {
   /* These strings are allocated with strdup().  */
   char *name;			/* "E1M1" */
   char *longname;		/* "Hangar" */
   struct dwdb_vertex *vertices;
   struct dwdb_sector *sectors;
   struct dwdb_side *sides;
   struct dwdb_line *lines;
   struct dwdb_thing *things;
   /* private: */
   struct dwdb_alloc vertex_alloc, side_alloc, line_alloc, sector_alloc,
    thing_alloc;
   struct dwdb_observer *first_observer;
};


/*--------------------------------------------------------------------
 * Main functions
 */

/* The constructor and destructor.  */
void dwdb_init(struct dwdb_level *);
void dwdb_fini(struct dwdb_level *);

/* Copy an existing level structure.  Does not copy observers.  Might
 * be useful for quickly restarting the level when the player dies.
 *
 * Not yet implemented!
 *
 * FIXME: Can object index numbers in the copy differ from those in
 * the original?  In other words, can the copy operation compress the
 * data and throw deleted objects away?  */
void dwdb_init_copy(struct dwdb_level *, const struct dwdb_level *source);

/* Load a level from a Doom-style WAD file.  The name of this function
 * contains the magic word "init", meaning that the function
 * initializes the structure without looking at its previous contents.
 *
 * The newly-loaded level will contain things of all difficulty
 * levels.  There should be a function for setting the difficulty
 * level and getting rid of everything that doesn't belong to it, but
 * that hasn't been written yet.  */
void dwdb_init_doom(struct dwdb_level *, const char *name);

/* Save the level to a Doom-style WAD file.  This does not alter the
 * in-memory level in any way.  Index numbers are not retained.  */
void dwdb_save_doom(const struct dwdb_level *, const char *filename);

/* Change the name of the level.  Libdumbworldb doesn't limit the
 * length of the name but Doom-style WADs can only store 8 characters.
 */
void dwdb_set_name(struct dwdb_level *, const char *name);

/* Change the long name of the level.  The length of the name is
 * unlimited.  */
void dwdb_set_longname(struct dwdb_level *, const char *longname);

/* Delete all things that shouldn't exist in DIFFICULTY.
 * This may only be called once per level.  */
void dwdb_set_difficulty(struct dwdb_level *, int difficulty);


/*--------------------------------------------------------------------
 * Creating/deleting/counting objects
 */

/* These make room for NEWCOUNT new vertices/lines/whatever.  The idea
 * is to call these if you know in advance how many you will need.
 * Then the array doesn't have to be resized all the time.
 *
 * If there already are vertices/whatever, the number of existing ones
 * is added to NEWCOUNT.  */
void dwdb_prealloc_vertices(struct dwdb_level *, unsigned newcount);
void dwdb_prealloc_sides(struct dwdb_level *, unsigned newcount);
void dwdb_prealloc_lines(struct dwdb_level *, unsigned newcount);
void dwdb_prealloc_sectors(struct dwdb_level *, unsigned newcount);
void dwdb_prealloc_things(struct dwdb_level *, unsigned newcount);

/* These create objects and return their index numbers.  If the object
 * can't be created (out of memory?), that's a fatal error.
 *
 * Sectors must be created before sides that refer to them.
 * Sides and vertices must be created before lines that refer to them.  */
unsigned dwdb_new_vertex(struct dwdb_level *, int x, int y);
unsigned dwdb_new_sector(struct dwdb_level *,
			 const struct dwdb_sector *model);
unsigned dwdb_new_side(struct dwdb_level *,
		       const struct dwdb_side *model);
unsigned dwdb_new_line(struct dwdb_level *,
		       const struct dwdb_line *model);
unsigned dwdb_new_thing(struct dwdb_level *,
			const struct dwdb_thing *model);

/* Deleting objects.
 * Deletion does not affect the indices of other objects.  */
void dwdb_del_vertex(struct dwdb_level *, unsigned ind);
void dwdb_del_sector(struct dwdb_level *, unsigned ind);
void dwdb_del_side(struct dwdb_level *, unsigned ind);
void dwdb_del_line(struct dwdb_level *, unsigned ind);
void dwdb_del_thing(struct dwdb_level *, unsigned ind);

/* Checking if an index is valid.
 * These return 1 if the index is valid and 0 if not.
 * Negative indices are never valid.
 * These functions may be rather slow.  */
int dwdb_isok_vertex(const struct dwdb_level *, int ind);
int dwdb_isok_sector(const struct dwdb_level *, int ind);
int dwdb_isok_side(const struct dwdb_level *, int ind);
int dwdb_isok_line(const struct dwdb_level *, int ind);
int dwdb_isok_thing(const struct dwdb_level *, int ind);

/* Returns the number of the sector or -1 if not found.  */
int dwdb_find_sector_2d(const struct dwdb_level *, int x, int y);


/*--------------------------------------------------------------------
 * Setting object attributes
 */

/* dwdb_set_line_side(): If there already was a side on LINEIND,
 * it will be deleted.  If SIDEIND was already on some line, that line
 * will be changed to use -1 instead.  SIDEIND may be -1 to detach the
 * current side from the line.  In that case, the side won't be
 * automatically deleted.  */
void dwdb_set_line_side(struct dwdb_level *, unsigned lineind,
			unsigned fore0_back1, int sideind);


/*--------------------------------------------------------------------
 * Observer stuff
 */

enum dwdb_object_type {
   DWDB_OT_LEVEL,
   DWDB_OT_VERTEX,
   DWDB_OT_SECTOR,
   DWDB_OT_SIDE,
   DWDB_OT_LINE,
   DWDB_OT_THING
};

struct dwdb_observer_funcs {
   /* None of these functions may add/remove observers.  And changing
    * the level may be risky too.
    *
    * Any of these pointers may be NULL, which means the function is a
    * no-op.
    *
    * When type is DWDB_OT_LEVEL, index is always -1.  */
   void (*init) (struct dwdb_level *, void *clientdata);
   void (*fini) (struct dwdb_level *, void *clientdata);
   void (*after_new) (struct dwdb_level *, enum dwdb_object_type type,
		      int index, void *clientdata);
   void (*after_ch) (struct dwdb_level *, enum dwdb_object_type type,
		     int index, void *clientdata);
   void (*before_del) (struct dwdb_level *, enum dwdb_object_type type,
		       int index, void *clientdata);
};

/* Create/register an observer and then call funcs->init if it's
 * non-NULL.  When LEVEL changes in some way, the corresponding
 * function in FUNCS will be called.  CLIENTDATA will be passed to the
 * function as an extra parameter.  Functions of different observers
 * will be called in an unspecified order.  *FUNCS is not needed after
 * the call.  */
struct dwdb_observer *
 dwdb_new_observer(struct dwdb_level *level,
		   const struct dwdb_observer_funcs *funcs,
		   void *clientdata);

/* Delete/unregister OBSERVER which has been previously created with
 * dwdb_new_observer.  It's an error if OBSERVER was created in
 * some other level or has already been deleted.  If an observer isn't
 * deleted with this function, it'll be automatically deleted when the
 * level is deleted.  Whenever an observer is deleted, its fini
 * function is called if non-NULL.  */
void dwdb_del_observer(struct dwdb_level *level,
		       struct dwdb_observer *observer);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
