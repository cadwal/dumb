
#ifndef LEVINFO_H
#define LEVINFO_H

#include "lib/endian.h"
#include "levdata.h"

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

void init_levinfo(void);
void reset_levinfo(void);

void levinfo_startgame(LevData *ld,int episode,int difficulty,int mplayer);
void levinfo_next(LevData *ld,int secret);

const LevInfo *find_levinfo(LevData *ld);

const char *get_skyname(LevData *ld);

#endif
