#ifndef LEVDATANUMS_H
#define LEVDATANUMS_H

#define ML_VERTEX 0
#define ML_SIDE 1
#define ML_LINE 2
#define ML_SECTOR 3
#define ML_THING 4
/*ML_SEG,ML_SSECTOR,ML_NODE,*/
#define ML_REJECT 5
#define ML_BLOCKMAP 6
#define ML_NTYPES 7
typedef int MapLumpType;

#define ME_NONE 0
/* move the floor or ceiling of a sector */
#define ME_CEILING 1
#define ME_FLOOR 3
/* change the texture of a switch */
#define ME_SWITCHON 5
#define ME_SWITCHOFF 6
/* teleport a thing */
#define ME_TELEPORT 7
/* finish this level */
#define ME_NEWLEVEL 8
#define ME_SECRETLEVEL 9
/* change the lightlevel of a sector */
#define ME_LIGHT 10
/* change textures and/or types of sectors */ 
#define ME_CEILING_TEX 2
#define ME_FLOOR_TEX 4
#define ME_CEILING_TYPE 11
#define ME_FLOOR_TYPE 12
typedef int MapEventType;

#endif
