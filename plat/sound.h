
#ifndef SOUND_H
#define SOUND_H

#include "lib/fixed.h"

/* use 0 for these params to get defaults */
void init_sound(int speed);
void reset_sound(void);

void set_sound_volume(fixed vol);

typedef struct {
   fixed lfvol,rfvol,lbvol,rbvol;
   int bend;
} SoundBalance;

#define FLAT_BALANCE {FIXED_ONE,FIXED_ONE,FIXED_ONE,FIXED_ONE,0}

/* queue a sound to be played */
void play_sound(const unsigned char *sample,int len,
		SoundBalance balance,int myspeed);

/* stop playing every sound now */
void purge_sound(void);

/* gets called frequently */
void poll_sound(void);

#endif
