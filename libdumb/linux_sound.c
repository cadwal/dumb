/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/linux_sound.c: Linux OSS sound driver.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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

#include <linux/soundcard.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbutil/safeio.h"
#include "sound.h"
#include "sound_mix.h"

/* configuration */

ConfItem sound_conf[] =
{
   CONFS("device", NULL, 0, N_("Sound device file to open"), "/dev/dsp"),
   CONFI("bits", NULL, 0, N_("Maximum allowed bits per channel"), 16),
   CONFB("mono", NULL, 0, N_("Force monaural sound")),
   CONFITEM_END
};
#define cnf_device (sound_conf[0].strval)
#define cnf_bits   (sound_conf[1].intval)
#define cnf_mono   (sound_conf[2].intval)

/* size of window in hundredths of seconds */
#define WINDOW_IN_HSEC 20

/* size of fragment to ask sound driver for, in bytes log 2 */
#define FRAGLEN_LOG2 8		/* ie. 256 bytes */

/* Signed 16-bit Host-Endian */
#if WORDS_BIGENDIAN
# define S16HE_FORMAT AFMT_S16_BE
#else  /* !WORDS_BIGENDIAN */
# define S16HE_FORMAT AFMT_S16_LE
#endif /* !WORDS_BIGENDIAN */

static int first_poll;

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

/* the file descriptor for the soundcard device */
static int fd = -1;

static int configure_sound(int s);

void
init_sound(int s)
{
   audio_buf_info abi;
   int window;

   /* check, are we already inited? */
   if (fd >= 0)
      reset_sound();

   /* try to open dsp */
   fd = safe_open(cnf_device, O_WRONLY, LOG_ERROR);
   if (fd < 0)
      return;		/* safe_open() already printed an error message */

   /* and configure it (this will call sndmix_init()) */
   if (!configure_sound(s)) {
      safe_close(cnf_device, fd);
      fd = -1;
      return;
   }
   /* figure out window size */
   ioctl(fd, SNDCTL_DSP_GETOSPACE, &abi);
   fragment_size = abi.fragsize / sample_size;
   window = (WINDOW_IN_HSEC * speed) / (100 * fragment_size);
   if (window < 2)
      window = 2;
   if (window > abi.fragstotal)
      window = abi.fragstotal;

   /* and work out slush frags from that */
   slush_fragments = abi.fragstotal - window;

   /* allocate buffer */
   fragbuf = safe_calloc(fragment_size, sample_size);

   /* say what we've done */
   logprintf(LOG_INFO, 'S',
	     _("init_sound: fd=%d fragsize=%d (%d bytes)"),
	     fd, fragment_size, fragment_size * sample_size);
   logprintf(LOG_INFO, 'S',
	     _("init_sound: speed=%d window=%d (%f seconds) slush=%d"),
	     speed, window,
	     (float) (window * fragment_size) / (float) speed,
	     slush_fragments);

   first_poll = 1;
}

/* this returns 1 if it succeeds */
static int
configure_sound(int s)
{
   int formats;			/* list of formats supported by device */
   int format;			/* OSS format we'll use */
   int fraginfo = 0x7fff0000 | FRAGLEN_LOG2;
   int stereority;
   enum sndmix_fmt sm_format;
   /* format */
   if (ioctl(fd, SNDCTL_DSP_GETFMTS, &formats)) {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: can't get sound format list: %s"),
		strerror(errno));
      return 0;
   }
   if (cnf_bits >= 16 && (formats & S16HE_FORMAT))
      format = S16HE_FORMAT;
   else if (cnf_bits >= 8 && (formats & AFMT_S8))
      format = AFMT_S8;
   else if (cnf_bits >= 8 && (formats & AFMT_U8))
      format = AFMT_U8;
   else {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: no supported formats found"));
      return 0;
   }
   if (ioctl(fd, SNDCTL_DSP_SETFMT, &format)) {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: set format failed: %s"),
		strerror(errno));
      return 0;
   }
   if (format == S16HE_FORMAT)
      sm_format = SNDMIX_FMT_S16;
   else if (format == AFMT_S8)
      sm_format = SNDMIX_FMT_S8;
   else if (format == AFMT_U8)
      sm_format = SNDMIX_FMT_U8;
   else {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: driver changed the format to something "
		  "we don't support"));
      return 0;
   }
   sample_size = sndmix_sample_size(sm_format);
   /* mono/stereo */
   stereority = !cnf_mono;
   if (ioctl(fd, SNDCTL_DSP_STEREO, &stereority)) {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: set stereo failed: %s"),
		strerror(errno));
      return 0;
   }
   if (stereority != !cnf_mono)
      logprintf(LOG_WARNING, 'S',
		stereority
		? _("init_sound: mono not available, using stereo")
		: _("init_sound: stereo not available, using mono"));
   if (stereority)
      sample_size *= 2;
   /* fragments */
   if (ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &fraginfo)) {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: set fragsize failed: %s"),
		strerror(errno));
      return 0;
   }
   /* sample rate */
   speed = s;
   if (ioctl(fd, SNDCTL_DSP_SPEED, &speed)) {
      logprintf(LOG_ERROR, 'S',
		_("init_sound: set speed failed: %s"),
		strerror(errno));
      return 0;
   }
   if (speed != s)
      logprintf(LOG_WARNING, 'S',
	   _("init_sound: sample rate %d Hz not available, using %d Hz"),
		s, speed);
   sndmix_init(speed, sm_format, stereority + 1);
   return 1;
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
   if (fragbuf)
      safe_free(fragbuf);
   if (fd >= 0)
      close(fd);
   fd = -1;
   fragbuf = NULL;
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
   if (fd < 0)
      return;
   sndmix_play_sound(buf, count, bal, myspeed + 300 * bal.bend);
}

void
poll_sound()
{
   if (fd < 0)
      return;
   while (1) {
      audio_buf_info abi;
      /* if there is <=slush space free, don't stuff in any more */
      ioctl(fd, SNDCTL_DSP_GETOSPACE, &abi);
      /*logprintf(LOG_DEBUG, 'S', _("poll_sound: %d frags free"),
         abi.fragments); */
      if (abi.fragments <= slush_fragments)
	 break;
      if (abi.fragments == abi.fragstotal && !first_poll)
	 logprintf(LOG_WARNING, 'S', _("poll_sound: underrun detected"));
      /* prepare a new fragment */
      sndmix_calc_frag(fragbuf, fragment_size);
      /* write it */
      write(fd, fragbuf, fragment_size * sample_size);
   }
   first_poll = 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
