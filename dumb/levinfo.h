
#ifndef LEVINFO_H
#define LEVINFO_H

#include "libdumb/levinfostruct.h"
#include "levdata.h"

void init_levinfo(void);
void reset_levinfo(void);

void levinfo_startgame(LevData *ld,int episode,int difficulty,int mplayer);
void levinfo_next(LevData *ld,int secret);

const LevInfo *find_levinfo(LevData *ld);

const char *get_skyname(LevData *ld);

#endif
