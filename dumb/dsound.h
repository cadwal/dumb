
#ifndef DSOUND_H

#include "lib/fixed.h"
#include "lib/endian.h"
#include "render/view.h"

#define MAX_REDIR_SOUNDS 7

typedef struct {
   char lumpname[10];
   LE_int16 bend_range,chance;
   LE_int16 nredir,bend_const;
   LE_int16 redir[MAX_REDIR_SOUNDS];
} SoundEnt;

void init_dsound(void);
void reset_dsound(void);

void dsound_setview(View *v);

void play_dsound_local(int sound,fixed x,fixed y,fixed radius);
void play_dsound(int sound,fixed x,fixed y,fixed radius);

#endif
