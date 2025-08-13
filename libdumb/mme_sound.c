/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/mme_sound.c: Digital Unix MME sound driver.
 * Copyright (C) 1998 by Marcus Sundberg <e94_msu@e.kth.se>
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <mme/mme_api.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "sound.h"
#include "sound_mix.h"

/* configuration */

ConfItem sound_conf[] =
{
   CONFI("bits", NULL, 0, N_("Bits per channel"), 8),
   CONFB("stereo", NULL, 0, N_("Stereo sound instead of mono")),
   CONFITEM_END
};
#define cnf_bits   (sound_conf[0].intval)
#define cnf_stereo (sound_conf[1].intval)

/* size of window in hundredths of seconds */
#define WINDOW_IN_HSEC 20

/* fragment size in bytes */
#define FRAGSIZE 4096
#define NUMBUF 4

/* a buffer */
static void *fragbuf = NULL;

/* some control data */
static int speed;
static fixed volume = FIXED_ONE_HALF;

/* if driver has more than this many frags free, it's time to send more data */
static int slush_fragments;

/* how big a fragment is in samples (not bytes) */
static size_t fragment_size;

/* how big a sample is in bytes, taking in account the number of channels */
static size_t sample_size;

/* descriptor amd buffers for the sounddevice */
static HWAVEOUT mms_device_handle = 0;
static int mms_nextbuf = 0;
static LPSTR mms_audio_buffer = NULL;
static LPWAVEHDR WaveHeader = NULL;

/* callbackfunction used by mme */
static void
my_callback(HANDLE hWaveOut,
	    UINT wMsg,
	    DWORD dwInstance,
	    LPARAM lParam1,
	    LPARAM lParam2)
{
   switch (wMsg) {
   case WOM_OPEN:
   case WOM_CLOSE:
      /* Ignore these */
      break;
   case WOM_DONE:
      slush_fragments--;
      break;
   default:
      break;
   }
}

void
init_sound(int s)
{
   MMRESULT status;
   LPPCMWAVEFORMAT lpWaveFormat;

   int bitness = cnf_bits, channels = cnf_stereo + 1;
   int window, outdevs;
   enum sndmix_fmt sm_format;

   /* check, are we already inited? */
   if (mms_device_handle != 0)
      reset_sound();

   /* check if sounddevice is available */
   if ((outdevs = waveOutGetNumDevs()) < 1) {
      logprintf(LOG_ERROR, 'S', _("init_sound: no sounddevice present"));
      return;
   }
   /* configure and open it */

   if ((lpWaveFormat = (LPPCMWAVEFORMAT)
	mmeAllocMem(sizeof(PCMWAVEFORMAT))) == NULL) {
      logprintf(LOG_ERROR, 'S',
		_("Failed to allocate PCMWAVEFORMAT struct"));
      return;
   }
   lpWaveFormat->wf.nSamplesPerSec = s;
   lpWaveFormat->wf.nChannels = channels;
   lpWaveFormat->wBitsPerSample = bitness;
   lpWaveFormat->wf.wFormatTag = WAVE_FORMAT_PCM;

   lpWaveFormat->wf.nBlockAlign = lpWaveFormat->wf.nChannels *
       ((lpWaveFormat->wBitsPerSample + 7) / 8);
   lpWaveFormat->wf.nAvgBytesPerSec = lpWaveFormat->wf.nBlockAlign *
       lpWaveFormat->wf.nSamplesPerSec;

   /* Open the audio device in the appropriate rate/format */
   status = waveOutOpen(&mms_device_handle,
			WAVE_MAPPER,
			(LPWAVEFORMAT) lpWaveFormat,
			(void (*)()) my_callback,
			(unsigned int) NULL,
			CALLBACK_FUNCTION | WAVE_OPEN_SHAREABLE);
   mmeFreeMem(lpWaveFormat);

   if (status != MMSYSERR_NOERROR) {
      logprintf(LOG_ERROR, 'S',
		_("waveOutOpen failed - status = %d\n"), status);
      return;
   }
   /* Allocate wave header for use in write */
   if ((WaveHeader = (LPWAVEHDR)
	mmeAllocMem(sizeof(WAVEHDR))) == NULL) {
      logprintf(LOG_ERROR, 'S', _("Failed to allocate WAVEHDR struct"));
      return;
   }
   /* figure out window size */
   fragment_size = FRAGSIZE / sample_size;
   window = (WINDOW_IN_HSEC * speed) / (100 * fragment_size);
   if (window < 2)
      window = 2;

   /* initialize sound mixer */
   sm_format = (bitness == 8) ? SNDMIX_FMT_U8 : SNDMIX_FMT_S16;
   sndmix_init(s, sm_format, channels);
   sample_size = sndmix_sample_size(sm_format) * channels;

   /* allocate buffer */
   if ((fragbuf = (Sample *) mms_audio_buffer
	= mmeAllocBuffer(FRAGSIZE * NUMBUF))
       == NULL) {
      logprintf(LOG_ERROR, 'S', _("Failed to allocate shared audio buffer"));
      mmeFreeMem(WaveHeader);
      sndmix_reset();
      return;
   }
   /* say what we've done */
   logprintf(LOG_INFO, 'S',
	     _("init_sound: mme_handle=%d fragsize=%d (%ld bytes)"),
	     mms_device_handle, fragment_size,
	     fragment_size * sample_size);
   logprintf(LOG_INFO, 'S',
	     _("init_sound: speed=%d window=%d slush=%d"),
	     s, window, slush_fragments);
}

/* stop playing now, and throw away anything you were about to play */
void
purge_sound(void)
{
   sndmix_purge();
   /* TODO: purge any buffers the kernel might have */
}

/* do all of the above, and shut down the sound device, clean up, etc */
void
reset_sound(void)
{
   purge_sound();
   sndmix_reset();
   if (mms_audio_buffer)
      mmeFreeBuffer(mms_audio_buffer);
   if (mms_device_handle != 0) {
      mmeFreeMem(WaveHeader);
      waveOutClose(mms_device_handle);
   }
   mms_device_handle = 0;
   fragbuf = NULL;
}

void
set_sound_volume(fixed vol)
{
   volume = vol;		/* FIXME: use this too */
}

void
play_sound(const unsigned char *buf, int count,
	   SoundBalance bal, int myspeed)
{
   int i;
   /*logprintf(LOG_DEBUG, 'S', _("play_sound: %d bytes: %f seconds"),
      count, ((float) count) / ((float) myspeed)); */
   if (mms_device_handle == 0)
      return;
   sndmix_play_sound(buf, count, bal, myspeed + 300 * bal.bend);
}

void
poll_sound()
{
   if (mms_device_handle == 0)
      return;
   while (1) {
      int i;
      if (mmeCheckForCallbacks())
	 mmeProcessCallbacks();
      /* if there is <=slush space free, don't stuff in any more */
      if (slush_fragments >= NUMBUF)
	 break;
      /* prepare a new fragment */
      sndmix_calc_frag(fragbuf, fragment_size);
      /* write it */
      WaveHeader->lpData = (LPSTR) fragbuf;
/*      printf(_("Slush: %d, nextbuf: %d\n"), slush_fragments, mms_nextbuf); */
      mms_nextbuf++;
      if (mms_nextbuf == NUMBUF)
	 mms_nextbuf = 0;
      fragbuf = (Sample *) (mms_audio_buffer + mms_nextbuf * FRAGSIZE);
      WaveHeader->dwBufferLength = FRAGSIZE;
      if (waveOutWrite(mms_device_handle, WaveHeader,
		       sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
	 logprintf(LOG_INFO, 'S', _("waveOutWrite failed"));
      } else
	 slush_fragments++;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
