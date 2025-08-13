/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/levdyn.h: Initializing dynamic parts of a level.  Creating things.
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

#ifndef LEVDYN_H
#define LEVDYN_H

#include "libdumbutil/fixed.h"
#include "libdumb/texture.h"
#include "libdumb/prothing.h"
#include "levdata.h"

#define LD_DYNDECL(type) DYN_INIT_FUNC(type##_dyninit)
#define LD_DYNINIT(type,dyn,lump) type##Dyn *dyn=_dyn;const type##Data *lump=_lump

struct LineDyn_struct;

typedef struct {
   fixed x, y, tx, ty, proj;
} VertexDyn;

LD_DYNDECL(Vertex);

typedef struct {
   Texture *utex, *mtex, *ltex;
   struct LineDyn_struct *lined;
   int uanim, manim, lanim;
   int uanimstate, manimstate, lanimstate;
} SideDyn;

LD_DYNDECL(Side);

typedef struct LineDyn_struct {
   fixed length;
   fixed cent_x, cent_y;
   char block_activation, had_keymsg, _spare1, _spare2;
} LineDyn;

LD_DYNDECL(Line);

/* values for crush_effect are defined in levdata.h */

typedef struct {
   Texture *ftex, *ctex;
   fixed floor, ceiling;
   fixed cent_x, cent_y;
   fixed cent_r;
   int fanim, canim;
   fixed dark;		/* 0 is infinite light (see forever), 1 is very dark */
   int fanimstate, canimstate;
   int type;
   char sky, crush_effect, _spare1, _spare2;
   /* sky: bit 1 is ceiling, bit 2 floor */
} SectorDyn;

LD_DYNDECL(Sector);

#define MAX_SPARE_THINGS 64
typedef struct {
   ProtoThing *proto;
   const ThingPhase *phase_tbl;
   fixed x, y, z;
   fixed angle, elev;
   int sector;
   int hits, armour, tmpinv, tmpgod;
   int flags;
   int phase;
   /* changes in members after this don't require an update */
   fixed dx, dy, dz, dangle, delev;
   fixed bouncemax;
   ThingSignal pending_signal;
   Texture *image;
   int mirror_image;
   int phase_wait;
   int target;
   int owner;
   int wakeness;
   int spawncount;
   /* time at which this object last hit an impassible wall, for monster AI */
   Ticks last_hit_wall;
} ThingDyn;

LD_DYNDECL(Thing);

#define THING_MAGIC_JELLYBEAN offsetof(ThingDyn,dx)

/* ThingDyn::flags */
/* Only 16 bits have been allocated for these in ThingCode.  */
#define THINGDF_NOCLIP 0x0001  /* spispopd */

#define ldvertexd(ld) ((VertexDyn *)(ld->dyn[ML_VERTEX]))
#define ldsectord(ld) ((SectorDyn *)(ld->dyn[ML_SECTOR]))
#define ldlined(ld) ((LineDyn *)(ld->dyn[ML_LINE]))
#define ldsided(ld) ((SideDyn *)(ld->dyn[ML_SIDE]))
#define ldthingd(ld) ((ThingDyn *)(ld->dyn[ML_THING]))

#define ldthingph(ld,on) (ldthingd(ld)[on].phase_tbl+ldthingd(ld)[on].phase)

#define wall_length(ld,ln) (ldlined(ld)[ln].length)

#endif

// Local Variables:
// c-basic-offset: 3
// End:
