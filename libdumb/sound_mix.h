/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/sound_mix.h: Mixing sounds to one stream.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMB_SOUND_MIX_H
#define LIBDUMB_SOUND_MIX_H

#include "libdumb/sound.h"

/* We currently don't support U16. */
enum sndmix_fmt {
   SNDMIX_FMT_S16,
   SNDMIX_FMT_S8,
   SNDMIX_FMT_U8
};

void sndmix_init(unsigned samplerate, enum sndmix_fmt format,
		 unsigned channels);
void sndmix_purge(void);
void sndmix_reset(void);

size_t sndmix_sample_size(enum sndmix_fmt fmt);

/* This does not use bal.bend */
void sndmix_play_sound(const unsigned char *buf, size_t len,
		       SoundBalance bal, int freq);

/* The actual size of buf is samples * sndmix_sample_size(fmt) * channels */
void sndmix_calc_frag(void *buf, unsigned samples);

#endif /* LIBDUMB_SOUND_MIX_H */

// Local Variables:
// c-basic-offset: 3
// End:
