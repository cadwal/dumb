#ifndef LINETYPESTRUCT_H
#define LINETYPESTRUCT_H

#include "libdumbutil/endiantypes.h"
#include "libdumbutil/fixed.h"

typedef struct {
   LE_flags32 flags;
   LE_int16 /*MapLumpType*/ lumptype;
   LE_int16 /*MapEventType*/ eventtype;
   LE_int16 delay;
   LE_int16 spawntype;
   LE_int16 sound,stopsound,contsound,waittype;
   LE_int16 term_offset[2],term_type[2];
   LE_int32 /*fixed*/ speed[2];
} LT_Action;

#define NO_ACTION(lta) ((lta)->eventtype==ME_NONE)

#define LTA_MANUAL 0x0001       /* act on back sector/side, not tagged */
#define LTA_MANUAL_FRONT 0x0002 /* act on front sector/side, not tagged */
#define LTA_DONUT_OUTER 0x0004  /* act on sector "surrounding" tagged sect */
#define LTA_STAIR 0x0008        /* recursively "stairify" */
#define LTA_UNQUEUE_ALL 0x0010  /* cancel all events acting here first */
#define LTA_FASTCRUSH 0x0020    
#define LTA_SLOWCRUSH 0x0040    
#define LTA_NOCRUSH 0x0080      
#define LTA_DONUT_INNER 0x0100  /* use "surrounding sector" to get term */
#define LTA_NUM_MODEL 0x0200    /* load event->model using numeric rule */


/* values for term-type */ 
typedef enum {
   NoTermType,
   Floor,Ceiling, /* 1,2 - these numbers are for reading error msgs */
   LowestFloor,LowestAdjacentFloor,
   HighestFloor,HighestAdjacentFloor, /* 5,6 */
   LowestCeiling,LowestAdjacentCeiling,
   HighestCeiling,HighestAdjacentCeiling, /* 9,10 */
   NextHighestFloor,NextHighestCeiling,
   NextLowestFloor,NextLowestCeiling, /* 13,14 */
   FloorPlusSLT,FloorMinusSLT,
   OrigFloor,OrigCeiling,  /* 17,18 */
   TTOne,TTZero, 
   DarkestSector,DarkestAdjacentSector,  /* 21,22 */
   LightestSector,LightestAdjacentSector
} LT_TermType;

#define MAX_LT_ACTIONS 4

typedef struct {
   LE_flags32 flags;
   LE_int16 sound,damage;
   LE_int16 keytype,spottype;
   LE_int32 scrolldx, scrolldy; /* pixel/tick == 1<<12 */
   LT_Action action[MAX_LT_ACTIONS];
} LineType,SectorType;

#define LT_REPEATABLE 0x0001
#define LT_ALLOW_PLAYER 0x0002
#define LT_ALLOW_NONPLAYER 0x0004
#define LT_ON_THUMPED 0x0008
#define LT_ON_CROSSED 0x0010
#define LT_ON_ACTIVATED 0x0020
#define LT_ON_DAMAGED 0x0040
#define LT_FRONT_ONLY 0x0080	/* only allow activation from in front */

#define ST_SECRET 0x10000	/* player gets credit for entering sector */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
