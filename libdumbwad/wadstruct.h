/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbwad/wadstruct.h: Structures in a Doom WAD file.
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

#ifndef WADSTRUCT_H
#define WADSTRUCT_H

#include "libdumbutil/endiantypes.h"
#include "libdumbwad/wadio.h"	/* LUMPNAMELEN */

/* wad structures don't get aligned: old GCCs might need a #pragma instead */
/*#define PACKED __attribute__((packed)) */

/* Annoyingly, the above doesn't seem to do anything on systems which need
   it, AND it breaks C++ compilation.  If anyone can explain this to me,
   please do. */
#define PACKED

#ifdef __cplusplus
#define UMEMB(n) n()
#else
#define UMEMB(n) n
#endif

/* WAD structures */

typedef struct {
   char sig[4];
   LE_uint32 nlumps;
   LE_uint32 diroffset;
} WadHeader;

typedef struct {
   LE_uint32 offset;
   LE_uint32 size;
   char name[LUMPNAMELEN];
} WadDirEntry;


/* Sounds */

typedef struct {
   LE_uint16 sig, hz, len, _spare;
   unsigned char data[1];
} SoundHdr;


/* Pictures and Patches */

typedef struct {
   LE_uint16 width, height;
   LE_int16 xoffset, yoffset;
   LE_int32 idx[1];
} PictHeader;

typedef struct {		/* dumb-only extension */
   char sig[2];			/* {'J','1'} */
   unsigned char log2width, log2height;
   LE_int32 width, height;
   /* followed by width<<log2_height bytes of data */
   unsigned char data[1];
} AltPictData;

#ifndef __cplusplus
typedef union {
   unsigned char data[1];
   PictHeader hdr;
   AltPictData alt;
} PictData;

#else  /* !__cplusplus */

typedef struct {
   unsigned char data[1];
   PictHeader &hdr() {
      return *(PictHeader *) data;
   } AltPictData &alt() {
      return *(AltPictData *) data;
   } const PictHeader &hdr() const {
      return *(PictHeader *) data;
   } const AltPictData &alt() const {
      return *(AltPictData *) data;
   }
} PictData;

#endif /* !__cplusplus */

#define IS_JPATCH(p) ((p)->UMEMB(alt).sig[0]=='J'&&(p)->UMEMB(alt).sig[1]=='1')


/* Wall Textures */

typedef struct {
   LE_int16 x, y;
   LE_uint16 pnum;
   LE_uint16 stepdir;		/* ??? */
   LE_uint16 colmap;		/* ??? */
} PatchDescData;

#define MAXPATCHDS 64

typedef struct {
   char name[LUMPNAMELEN];
   LE_int32 dummy1;
   LE_int16 dx, dy;
   int dummy2;
   LE_int16 npatches;
   PatchDescData patch[MAXPATCHDS];
} TextureData;

typedef struct {
   LE_int32 num;
   char name[1];
} PNameTable;

typedef struct {
   LE_int32 ntxts;
   LE_int32 idx[1];
} TextureHdr;

#ifndef __cplusplus
typedef union {
   TextureHdr hdr;
   char data[1];
} TextureTable;

#else
typedef struct {
   char data[1];
   TextureHdr &hdr() {
      return *(TextureHdr *) data;
   } const TextureHdr &hdr() const {
      return *(TextureHdr *) data;
   }
} TextureTable;

#endif


/*** Levels ***/

/* the thingdefs */

typedef struct {
   LE_int16 x, y;
   LE_int16 angle;		/* this is in *degrees*, not bams */
   LE_int16 type;
   LE_flags16 flags;
} PACKED ThingData;

#define THING_12    1
#define THING_3     2
#define THING_45    4
#define THING_DEAF  8
#define THING_MULTI 16

/* the linedefs */

#define LINE_IMPASSIBLE 0x0001
#define LINE_MIMPASSIBLE 0x0002
#define LINE_TWOSIDED 0x0004
#define LINE_UPUNPEG 0x0008
#define LINE_LOUNPEG 0x0010
#define LINE_SECRET 0x0020
#define LINE_BLKSOUND 0x0040
#define LINE_NOMAP 0x0080
#define LINE_FORCEMAP 0x0100

/* these are dumb-specific */
#define LINE_POSTER 0x0200

typedef struct {
   LE_int16 ver2, ver1;
   LE_flags16 flags;
   LE_int16 type;
   LE_int16 tag;
   LE_int16 side[2];
} PACKED LineData;


/* the sidedefs */

typedef struct {
   LE_int16 xoffset, yoffset;
   char utexture[LUMPNAMELEN];
   char ltexture[LUMPNAMELEN];
   char texture[LUMPNAMELEN];
   LE_int16 sector;
} PACKED SideData;


/* the vertices */

typedef struct {
   LE_int16 x, y;
} PACKED VertexData;


/* the sectordefs */

typedef struct {
   LE_int16 floor, ceiling;
   char ftexture[LUMPNAMELEN];
   char ctexture[LUMPNAMELEN];
   LE_int16 light;
   LE_int16 type;
   LE_int16 tag;
} PACKED SectorData;


/* the BSP data: segs, ssectors, nodes */

typedef struct {
   LE_int16 ver1, ver2;
   LE_int16 angle;		/* this is in BAMs, not degrees */
   LE_int16 line;
   LE_int16 isleft;		/* 1 if on 2nd side of line, otherwise 0 */
   LE_int16 offset;		/* along line */
} PACKED SegData;

typedef struct {
   LE_int16 nsegs, seg1;
} PACKED SSectorData;

typedef struct {
   /* these coordinates refer to the Node's partition line */
   LE_int16 x, y;
   LE_int16 dx, dy;
   /* the boundaries of the right child-node */
   LE_int16 rymax, rymin;
   LE_int16 rxmin, rxmax;
   /* the boundaries of the left child-node */
   LE_int16 lymax, lymin;
   LE_int16 lxmin, lxmax;
   /* the children */
   int right:15;
   int rleaf:1;			/* 1 if the right child is a leaf node, ie a ssector */
   int left:15;
   int lleaf:1;
} PACKED NodeData;


/* BLOCKMAP */

#define BM_BLOCKSIZE 128

typedef struct {
   LE_int16 minx, miny;
   LE_int16 numx, numy;
   LE_uint16 idx[1];
} BlockMapHdr;

#ifndef __cplusplus
typedef union {
   BlockMapHdr hdr;
   LE_int16 data[1];
} BlockMap;
#else  /* __cplusplus */
typedef struct {
   LE_int16 data[1];
   BlockMapHdr &hdr() {
      return *(BlockMapHdr *) data;
   }
   const BlockMapHdr &hdr() const {
      return *(const BlockMapHdr *) data;
   }
} BlockMap;
#endif /* __cplusplus */

/* blocklist(x,y) is at data+(x-minx)/BMBS+((y-miny)/BMBS)*numx
 * "each blocklist starts with a 0 and ends with a 0xffff" */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
