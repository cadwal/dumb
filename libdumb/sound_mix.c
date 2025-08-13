/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/sound_mix.c: Mixing sounds to one stream.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <string.h>
#include <assert.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/fixed.h"
#include "libdumbutil/log.h"
#include "sound_mix.h"

/* how many samples we can play at maximum volume without distortion */
#define VOLUME_DIVISOR 4

typedef unsigned char UINT8;
typedef signed char INT8;
typedef signed short INT16;

typedef INT8 MONO_8;
typedef INT16 MONO_16;
typedef struct {
   INT8 left, right;
} STEREO_8;
typedef struct {
   INT16 left, right;
} STEREO_16;

/* sound queue stuff */
typedef struct {
   const UINT8 *data;
   size_t count;		/* count == 0 if slot unused */
   int step;			/* 1/256 fixed-point */
   fixed lvol, rvol;		/* only lvol used if mono */
} SQEnt;

#define SQMAX 64

static SQEnt sq[SQMAX];

static int sndmix_inited = 0;

static unsigned samplerate;
static enum sndmix_fmt format;
static unsigned channels;

static size_t calc_mono_8(MONO_8 *buf, size_t len, const SQEnt *ent);
static size_t calc_mono_16(MONO_16 *buf, size_t len, const SQEnt *ent);
static size_t calc_stereo_8(STEREO_8 *buf, size_t len, const SQEnt *ent);
static size_t calc_stereo_16(STEREO_16 *buf, size_t len, const SQEnt *ent);

void
sndmix_init(unsigned rate, enum sndmix_fmt fmt, unsigned chn)
{
   if (sndmix_inited)
      sndmix_reset();

   samplerate = rate;
   format = fmt;
   channels = chn;

   /* clear queue */
   memset(&sq, 0, sizeof(sq));

   sndmix_inited = 1;
}

void
sndmix_purge(void)
{
   /* clear queue */
   memset(&sq, 0, sizeof(sq));
}

void
sndmix_reset(void)
{
   sndmix_inited = 0;
}

size_t
sndmix_sample_size(enum sndmix_fmt fmt)
{
   switch (fmt) {
   case SNDMIX_FMT_S16:
      return 2;
   case SNDMIX_FMT_S8:
   case SNDMIX_FMT_U8:
      return 1;
   default:
      logprintf(LOG_ERROR, 'S', _("sndmix_sample_size: strange format %d"),
		fmt);
      return 0;
   }
}

void
sndmix_play_sound(const unsigned char *buf, size_t len,
		  SoundBalance bal, int freq)
{
   int i;
   /* If sndmix hasn't been initialized, this function could just
    * return with no error, but sndmix_calc_frag() can't.  For
    * consistency, both functions assert(sndmix_inited).  */
   assert(sndmix_inited);
   for (i = 0; i < SQMAX; i++) {
      if (sq[i].count == 0)
	 break;
   }
   if (i == SQMAX) {
      logprintf(LOG_ERROR, 'S',
		_("sndmix_play_sound: out of slots (sound discarded)"));
      return;
   }
   sq[i].data = buf;
   sq[i].count = len;
   if (channels == 1)
      sq[i].lvol = (bal.lfvol + bal.rfvol + bal.lbvol + bal.rbvol) / 4;
   else {			/* 2 */
      sq[i].lvol = (bal.lfvol + bal.lbvol) / 2;
      sq[i].rvol = (bal.rfvol + bal.rbvol) / 2;
   }
   sq[i].step = freq * 0x100 / samplerate;
}

void
sndmix_calc_frag(void *buf, unsigned samples)
{
   int i;
   /* If sndmix has not been initialized, this function aborts the
    * program.  The alternative would be to zero the buffer, but that
    * is impossible, as the number of bits and channels is not known.
    */
   assert(sndmix_inited);
   switch (format) {
   case SNDMIX_FMT_S16:
      memset(buf, 0, samples * channels * 2);
      break;
   case SNDMIX_FMT_S8:
      memset(buf, 0, samples * channels);
      break;
   case SNDMIX_FMT_U8:
      memset(buf, 0x80, samples * channels);
      break;
   default:
      logprintf(LOG_ERROR, 'S', _("sndmix_calc_frag: strange format %d"),
		format);
      return;
   }
   for (i = 0; i < SQMAX; i++) {
      if (sq[i].count != 0) {
	 size_t bytes;
	 switch (format) {
	 case SNDMIX_FMT_S16:
	    if (channels == 1)
	       bytes = calc_mono_16((MONO_16 *) buf, samples, &sq[i]);
	    else
	       bytes = calc_stereo_16((STEREO_16 *) buf, samples, &sq[i]);
	    break;
	 case SNDMIX_FMT_S8:
	 case SNDMIX_FMT_U8:	/* this assumes a 2's complement system */
	    if (channels == 1)
	       bytes = calc_mono_8((MONO_8 *) buf, samples, &sq[i]);
	    else
	       bytes = calc_stereo_8((STEREO_8 *) buf, samples, &sq[i]);
	    break;
	 default:
	    abort();		/* someone's changing format behind our back */
	 }
	 if (bytes >= sq[i].count)
	    sq[i].count = 0;
	 else {
	    sq[i].count -= bytes;
	    sq[i].data += bytes;
	 }
      }				/* if count */
   }				/* for i */
}

static size_t
calc_mono_8(MONO_8 *buf, size_t len, const SQEnt *ent)
{
   unsigned ns = 0, nb = 0;
   while (ns < len && (nb >> 8) < ent->count) {
      int scaled = (ent->data[nb >> 8] - 0x80) / VOLUME_DIVISOR;
      buf[ns] += fixmul(scaled, ent->lvol);
      ns++;
      nb += ent->step;
   }
   return nb >> 8;
}

static size_t
calc_stereo_8(STEREO_8 *buf, size_t len, const SQEnt *ent)
{
   unsigned ns = 0, nb = 0;
   while (ns < len && (nb >> 8) < ent->count) {
      int scaled = (ent->data[nb >> 8] - 0x80) / VOLUME_DIVISOR;
      buf[ns].left += fixmul(scaled, ent->lvol);
      buf[ns].right += fixmul(scaled, ent->rvol);
      ns++;
      nb += ent->step;
   }
   return nb >> 8;
}

static size_t
calc_mono_16(MONO_16 *buf, size_t len, const SQEnt *ent)
{
   unsigned ns = 0, nb = 0;
   while (ns < len && (nb >> 8) < ent->count) {
      int scaled = (ent->data[nb >> 8] - 0x80) * (256 / VOLUME_DIVISOR);
      buf[ns] += fixmul(scaled, ent->lvol);
      ns++;
      nb += ent->step;
   }
   return nb >> 8;
}

static size_t
calc_stereo_16(STEREO_16 *buf, size_t len, const SQEnt *ent)
{
   unsigned ns = 0, nb = 0;
   while (ns < len && (nb >> 8) < ent->count) {
      int scaled = (ent->data[nb >> 8] - 0x80) * (256 / VOLUME_DIVISOR);
      buf[ns].left += fixmul(scaled, ent->lvol);
      buf[ns].right += fixmul(scaled, ent->rvol);
      ns++;
      nb += ent->step;
   }
   return nb >> 8;
}

// Local Variables:
// c-basic-offset: 3
// End:
