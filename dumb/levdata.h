/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/levdata.h: Loading and updating levels.
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

#ifndef LEVDATA_H
#define LEVDATA_H

#include <stdlib.h>		/* size_t */

#include "libdumbutil/fixed.h"
#include "libdumbwad/wadstruct.h"
#include "libdumbwad/wadio.h"
#include "libdumb/texture.h"
#include "libdumb/levdatanums.h"

struct _LevData;

#define DYN_INIT_FUNC(name) \
  void (name)(void *_dyn,const void *_lump,struct _LevData *ld)
#define DYN_ENCODE_FUNC(name) \
  int (name)(void *code,const void *dyn,const void *backup,struct _LevData *ld)
#define DYN_DECODE_FUNC(name) \
  void (name)(void *dyn,const void *code,int len,struct _LevData *ld)

#define DYN_CODE_BUF_LEN 255

typedef DYN_INIT_FUNC(*DynInitFuncPtr);
typedef DYN_ENCODE_FUNC(*DynEncodeFuncPtr);
typedef DYN_DECODE_FUNC(*DynDecodeFuncPtr);

typedef struct {
   char lumpname[10];
   size_t lumpgrain, dyngrain, lumpalign;
   DynInitFuncPtr dyninit;
   int dynextra;
   DynEncodeFuncPtr encode;
   DynDecodeFuncPtr decode;
} MapLumpTypeInfo;

#define MLIENTG(lname) {lname,1,0,1,NULL,0,NULL,NULL}
#define MLIENT(lname,struc) {lname,sizeof(struc##Data),0,__alignof__(struc##Data),NULL,0}
#define MLIENTD(lname,struc,x) {lname,sizeof(struc##Data),sizeof(struc##Dyn),__alignof__(struc##Data),struc##_dyninit,x,struc##_encode,struc##_decode}

#define ML_INFO {\
 MLIENTD("VERTEXES",Vertex,0),\
 MLIENTD("SIDEDEFS",Side,0),\
 MLIENTD("LINEDEFS",Line,0),\
 MLIENTD("SECTORS",Sector,0),\
 MLIENTD("THINGS",Thing,MAX_SPARE_THINGS),\
 /*MLIENT("SEGS",Seg),\
 MLIENT("SSECTORS",SSector),\
 MLIENT("NODES",Node),*/\
 MLIENTG("REJECT"),\
 MLIENTG("BLOCKMAP")\
}

typedef unsigned int Ticks;

typedef struct {
   Ticks start, stop;
   short entity, sound, stopsound, contsound;
   short ltype, type;
   short wait, _spare;
   short model;
   /* this is tricky: 0 or 1 tell us what stage we're in, <0 means
      go to (stage+2) next time, copying delta[stage] and term[stage]
      to curdelta and curterm */
   signed char stage;
   /* meaningful for sector events only */
   char crush_effect;
   /* key is used for unqueuing events */
   const void *key;
   /* the meaning of these fields will vary */
   fixed curdelta, curterm;
   fixed delta[2], term[2];
   fixed x, y, z, angle;
} MapEvent;

/* values for crush_effect: what to do if a moving sector hits something */
#define MEC_NOTHING 0		/* ignore completely */
#define MEC_FASTHURT 1		/* kill whatever it is nowish (type 6) */
#define MEC_SLOWHURT 2		/* give them a chance to get out (type 25) */
#define MEC_STOP 3		/* stop moving (???) */
#define MEC_REVERSE 4		/* reverse, guess where to stop (doors) */

#define NEXT_STAGE(me) ((me)->stage=((me)->stage?-2:-1))
#define EVENTVALID(e) ((e).type!=ME_NONE)

#define MAXPLAYERS 4
#define MAXEVENTS 64

typedef struct _LevData {
   char name[12];
   LumpNum lumpnum[ML_NTYPES];
   int count[ML_NTYPES];
   const void *lump[ML_NTYPES];
   void *dyn[ML_NTYPES];
   void *dyn_bkup[ML_NTYPES];
   int player[MAXPLAYERS];
   int plwep[MAXPLAYERS];
   int plflags[MAXPLAYERS];
   /* plinfo_len, plinfo and plbkup are mostly handled by gettable.c */
   int plinfo_len;
   int *plinfo[MAXPLAYERS], *plbkup[MAXPLAYERS];
   Ticks map_ticks;
   MapEvent event[MAXEVENTS];
   int difficulty;
   int mplayer;
   int localplayer;
   int levinfo_id;
   int boss_id, boss_count;
} LevData;

#define PLF_WSEL     0x0001	/* player wanted wsel last input */
#define PLF_SSEL     0x0002	/* player wanted ssel last input */
#define PLF_CHEAT    0x0004	/* player cheated last input */

#define ldvertex(ld) ((const VertexData *)(ld->lump[ML_VERTEX])
#define ldline(ld) ((const LineData *)(ld->lump[ML_LINE]))
#define ldside(ld) ((const SideData *)(ld->lump[ML_SIDE]))
#define ldsector(ld) ((const SectorData *)(ld->lump[ML_SECTOR]))
#define ldseg(ld) ((const SegData *)(ld->lump[ML_SEG]))
#define ldssector(ld) ((const SSectorData *)(ld->lump[ML_SSECTOR]))
#define ldnode(ld) ((const NodeData *)(ld->lump[ML_NODE]))
#define ldreject(ld) ((const unsigned char *)(ld->lump[ML_REJECT]))
#define ldblockmap(ld) ((const BlockMap *)(ld->lump[ML_BLOCKMAP]))

#define ldnthings(ld) ld->count[ML_THING]
#define ldnvertices(ld) ld->count[ML_VERTEX]
#define ldnlines(ld) ld->count[ML_LINE]
#define ldnsides(ld) ld->count[ML_SIDE]
#define ldnsectors(ld) ld->count[ML_SECTOR]
#define ldnsegs(ld) ld->count[ML_SEG]
#define ldnssectors(ld) ld->count[ML_SSECTOR]
#define ldnnodes(ld) ld->count[ML_NODE]

void load_level(LevData *data, const char *name, int difficulty, int mp);
void free_level(LevData *data);
void reset_level(LevData *data);
void free_levlumptype(LevData *data, MapLumpType mlt);

int get_plinfo_len(void);
void init_plinfo(LevData *ld, int pl);

void init_player(LevData *ld, int plnum, int thnum);

char *get_next_level_name(char *buf, const char *name);

void do_tag666(LevData *ld);

void generate_updates(LevData *ld);
void apply_update(LevData *ld, MapLumpType mlt, int ofs,
		  const void *code, int codelen);
void apply_plinfo_update(LevData *ld, int player, int offset, int value);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
