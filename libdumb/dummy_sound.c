#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "sound.h"

void init_sound(int speed) {
}

/* stop playing now, and throw away anything you were about to play */
void purge_sound(void) {
}

/* do all of the above, and shut down the sound device, clean up, etc */
void reset_sound(void) {
   purge_sound();
}

void set_sound_volume(fixed vol) {
}

void play_sound(const unsigned char *sample,int len,
		SoundBalance balance,int myspeed) {
}

void poll_sound(void) {}




