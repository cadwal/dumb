/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/sdl_sound.c: SDL3 sound driver.  Mixing is done by
 * sound_mix.c in the game's own thread; this only keeps an SDL audio
 * stream topped up.
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

#include <SDL3/SDL.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "sound.h"
#include "sound_mix.h"

ConfItem sound_conf[] =
{
   CONFB("mono", NULL, 0, N_("Force monaural sound")),
   CONFI("latency", NULL, 0, N_("Sound buffer length in milliseconds (60)"),
	 60),
   CONFITEM_END
};

#define cnf_mono    (sound_conf[0].intval)
#define cnf_latency (sound_conf[1].intval)

/* Mix this many samples at a time.  */
#define FRAGMENT_SAMPLES 256

static SDL_AudioStream *stream = NULL;
static void *fragbuf = NULL;
static size_t frag_bytes;	/* FRAGMENT_SAMPLES * sample size * channels */
static int target_bytes;	/* keep about this much queued */
static fixed volume = FIXED_ONE_HALF;

void
init_sound(int speed)
{
   SDL_AudioSpec spec;
   int channels = cnf_mono ? 1 : 2;

   if (stream)
      reset_sound();
   if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
      logprintf(LOG_ERROR, 'S', _("Can't initialize SDL audio: %s"),
		SDL_GetError());
      return;
   }
   spec.format = SDL_AUDIO_S16;
   spec.channels = channels;
   spec.freq = speed;
   stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
				      &spec, NULL, NULL);
   if (stream == NULL) {
      logprintf(LOG_ERROR, 'S', _("Can't open the sound device: %s"),
		SDL_GetError());
      SDL_QuitSubSystem(SDL_INIT_AUDIO);
      return;
   }
   sndmix_init(speed, SNDMIX_FMT_S16, channels);
   frag_bytes = FRAGMENT_SAMPLES * sndmix_sample_size(SNDMIX_FMT_S16) * channels;
   fragbuf = safe_calloc(1, frag_bytes);
   target_bytes = (int) ((long) speed * cnf_latency / 1000
			 * sndmix_sample_size(SNDMIX_FMT_S16) * channels);
   SDL_ResumeAudioStreamDevice(stream);
   logprintf(LOG_INFO, 'S', _("Sound: SDL %s, %d Hz, %d channel(s), %d ms"),
	     SDL_GetCurrentAudioDriver(), speed, channels, cnf_latency);
}

/* stop playing now, and throw away anything you were about to play */
void
purge_sound(void)
{
   sndmix_purge();
   if (stream)
      SDL_ClearAudioStream(stream);
}

/* do all of the above, and shut down the sound device, clean up, etc */
void
reset_sound(void)
{
   if (!stream)
      return;
   purge_sound();
   sndmix_reset();
   SDL_DestroyAudioStream(stream);
   stream = NULL;
   if (fragbuf)
      safe_free(fragbuf);
   fragbuf = NULL;
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void
set_sound_volume(fixed vol)
{
   volume = vol;		/* FIXME: use it too */
}

void
play_sound(const unsigned char *buf, int count,
	   SoundBalance bal, int myspeed)
{
   if (!stream)
      return;
   sndmix_play_sound(buf, count, bal, myspeed + 300 * bal.bend);
}

void
poll_sound(void)
{
   if (!stream)
      return;
   while (SDL_GetAudioStreamQueued(stream) < target_bytes) {
      sndmix_calc_frag(fragbuf, FRAGMENT_SAMPLES);
      if (!SDL_PutAudioStreamData(stream, fragbuf, (int) frag_bytes))
	 break;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
