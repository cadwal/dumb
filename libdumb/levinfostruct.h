#ifndef LEVINFOSTRUCT_H
#define LEVINFOSTRUCT_H

#include "libdumbutil/endiantypes.h"

#define LEVLONGNAME_LEN 64

typedef struct {
   char name[10];
   char music[10];
   char sky[10];
   LE_int16 next,secret,_spare;
   LE_flags32 flags;
   char longname[LEVLONGNAME_LEN];
} LevInfo;

#define LI_START  0x0001  /* OK to start playing here */
#define LI_END    0x0002  /* endgame at end of level */
#define LI_WAFFLE 0x0004  /* not a level, but a message to show to player */

#endif
