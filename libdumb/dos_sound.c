/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/dos_sound.c: MS-DOS sound driver.
 * Copyright (C) 1998 by Ulf Axelsson <ulf@ore.ims.se>
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

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <bios.h>
#include <allegro.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"

/* Allegro already uses fixed */
#define fixed fixed_dumb
#include "sound.h"
#undef fixed

static int has_sound = TRUE;
static int digi_voices;

typedef struct SSampQueue {
   SAMPLE *spl;
   int timestamp;
   struct SSampQueue *next;
} SampQueue;

static SampQueue *SQh = NULL, *SQt = NULL;

/* Based on similar function i DOSDOOM, since allegro only loads
 * these from files.
 */
SAMPLE *
make_sample(const unsigned char *data, int len,
	    unsigned short freq)
{
   SAMPLE *spl;

   spl = (SAMPLE *) safe_malloc(sizeof(SAMPLE));
   spl->bits = 8;
   spl->freq = freq;
   spl->len = len;
   spl->priority = 255;
   spl->loop_start = 0;
   spl->loop_end = len;
   spl->param = -1;
   spl->data = (void *) data;

   return spl;
}


void
init_sound(int speed)
{
   digi_voices = detect_digi_driver(DIGI_AUTODETECT);

   if (install_sound(DIGI_AUTODETECT, MIDI_NONE, "")) {
      has_sound = FALSE;

      logprintf(LOG_ERROR, 'S',
		_("Couldn't initialize sound: %s"),
		allegro_error);
   }
   if (has_sound)
      logprintf(LOG_INFO, 'S', _("Sound driver: %s (%s)"),
		digi_driver->name, digi_driver->desc);

   SQh = SQt = (SampQueue *) safe_malloc(sizeof(SampQueue));
   SQh->next = NULL;
   SQh->spl = NULL;
}

/* stop playing now, and throw away anything you were about to play */
void
purge_sound(void)
{
}

/* do all of the above, and shut down the sound device, clean up, etc */
void
reset_sound(void)
{
   SampQueue *lSQ;

   purge_sound();
   remove_sound();

   while (SQh != NULL) {
      lSQ = SQh;
      SQh = SQh->next;
      if (lSQ->spl != NULL) {
	 stop_sample(lSQ->spl);
	 safe_free(lSQ->spl);
      }
      safe_free(lSQ);
   }
}

void
set_sound_volume(fixed_dumb vol)
{
   /* Never called ? */
   /* set_volume() ; */
}

/* What to do with volume ? Currently just guessing one

 * Somebody has to explain the SoundBalance struct to me.
 */
void
play_sound(const unsigned char *sample, int len,
	   SoundBalance balance, int myspeed)
{
   SampQueue *lSQ;

   lSQ = (SampQueue *) safe_malloc(sizeof(SampQueue));
   lSQ->spl = make_sample(sample, len, myspeed);
   lSQ->timestamp = biostime(0, 0);
   lSQ->next = NULL;
   SQt->next = lSQ;
   SQt = lSQ;

   play_sample(SQt->spl, 200, 128, 1000, FALSE);
}

/* Have to do this to avoid memory leak. UGLY!
 * Waits for 100 ticks (about 5s) before and still memory leaks
 * around midnight .....
 * How about preloading sounds ?
 */
void
poll_sound(void)
{
   SampQueue *lSQ;

   while ((SQh->next != NULL) &&
	  (SQh->next->timestamp < (biostime(0, 0) - 100))) {
      lSQ = SQh->next;
      SQh->next = SQh->next->next;
      if (lSQ == SQt)
	 SQt = SQh;

      stop_sample(lSQ->spl);

      safe_free(lSQ->spl);
      safe_free(lSQ);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
