/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/protoinwad.c: Encoding and decoding ProtoThings in WAD.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

/* This enables the strnlen() prototype in glibc2 <string.h>.  If
   there's no strnlen() in libc, HAVE_STRNLEN is undefined and the
   definition in ../libmissing/ is used.  */
#define _GNU_SOURCE

#include <config.h>

#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libmissing/libmissing.h"
#include "libdumbutil/align.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "protoinwad.h"
#include "gettableinwad.h"	/* encoder/decoder for Gets */

static char *protoinwad_strdup(const ProtoThing_inwad *src, int ofs);


/* The PROTOS lump consists of blocks of varying length.  Each block
   begins with a header which says how long the block is, including
   the header.  When DUMB starts up, it reads the blocks and converts
   them to the internal format defined in prothingstruct.h.

   Each block can also contain other information referred to by the
   header.  The location of each such piece of information is saved as
   a byte count from the beginning of the block.  Thus the meaning of
   a block does not depend on its position in the lump, and lumps can
   be concatenated.

   The length of each block should be a multiple of 8.

   Strings are null-terminated.  If the offset is zero, it means the
   string is undefined.  */

/* the typedef was already in protoinwad.h */
struct ProtoThing_inwad {
   LE_int16 block_length;
   LE_int16 id;			/* id referred to by THINGS lump */
   LE_int16 hits;
   LE_int16 damage;	/* how much damage to do in melee, or when exploding */
   LE_int32 /* fixed */ realmass, friction;
   LE_int32 /* fixed */ radius, height;
   LE_int32 sound;
   LE_flags32 flags;
   LE_int32 /* fixed */ speed;
   LE_int32 /* fixed */ shootarc_h, shootarc_v, see_arc, aim_arc;
   LE_int16 shootnum;
   LE_int16 spawn1, spawn2;	/* prothing id to be spawned by TPH_SPAWN */
   LE_int16 spawnmax;
   LE_int16 become1, become2;
   LE_int16 bloodtype;
   LE_int16 phase_id;
   LE_int16 signals[NUM_THINGSIGS];
   char sprite[6];
   /* The rest of the members say what happens when the player tries
      to pick this up.  */
   LE_int16 pickup_sound;
   LE_int16 ngets;		/* how many gettables this will give */
   LE_int16 getsofs;		/* where the Gets_inwad list begins */
   LE_int16 firstpickupmsgofs;	/* "Suddenly you can't see yourself." */
   LE_int16 pickupmsgofs;	/* "You found another backpack." */
   LE_int16 ignoremsgofs;	/* "You can't carry any more ammo." */
};


/* Decode */

int
is_protoinwad(const ProtoThing_inwad *data, size_t length)
{
   if (length == 0)
      return 0;
   else {
      size_t blk_length = data->block_length;
      if (blk_length < sizeof(ProtoThing_inwad) || blk_length > length) {
	 logprintf(LOG_ERROR, 'O', _("ProtoThing block chain corrupted"));
	 return 0;
      } else
	 return 1;
   }
}

void
next_protoinwad(const ProtoThing_inwad **datap, size_t *lengthp)
{
   size_t blk_length = (*datap)->block_length;
   *datap = (const ProtoThing_inwad *)
      (((const char *) *datap) + blk_length);
   *lengthp -= blk_length;
}

void
decode_protoinwad(ProtoThing *dest, const ProtoThing_inwad *src)
{
   /* endianness and/or width may be converted in the process, so we
      can't use memcpy() */
   int i;
   dest->id         = src->id;
   dest->hits       = src->hits;
   dest->damage     = src->damage;
   dest->realmass   = src->realmass;
   dest->friction   = src->friction;
   dest->radius     = src->radius;
   dest->height     = src->height;
   dest->sound      = src->sound;
   dest->flags      = src->flags;
   dest->speed      = src->speed;
   dest->shootarc_h = src->shootarc_h;
   dest->shootarc_v = src->shootarc_v;
   dest->see_arc    = src->see_arc;
   dest->aim_arc    = src->aim_arc;
   dest->shootnum   = src->shootnum;
   dest->spawn1     = src->spawn1;
   dest->spawn2     = src->spawn2;
   dest->spawnmax   = src->spawnmax;
   dest->become1    = src->become1;
   dest->become2    = src->become2;
   dest->bloodtype  = src->bloodtype;
   dest->phase_id   = src->phase_id;
   for (i = 0; i < NUM_THINGSIGS; i++)
      dest->signals[i] = src->signals[i];
   strncpy(dest->sprite, src->sprite, sizeof(dest->sprite));
   dest->ngets = src->ngets;
   dest->gets  = decode_getsinwad_array((const Gets_inwad *)
					(((const char *) src) + src->getsofs),
					src->ngets);
   dest->firstpickupmsg = protoinwad_strdup(src, src->firstpickupmsgofs);
   dest->pickupmsg      = protoinwad_strdup(src, src->pickupmsgofs);
   dest->ignoremsg      = protoinwad_strdup(src, src->ignoremsgofs);
   dest->pickup_sound = src->pickup_sound;
}

size_t
decode_protoinwad_array(ProtoThing **destp,
			const ProtoThing_inwad *src, size_t length)
{
   const ProtoThing_inwad *cur_src;
   size_t cur_length;
   size_t count = 0;
   for (cur_src = src, cur_length = length;
	is_protoinwad(cur_src, cur_length);
	next_protoinwad(&cur_src, &cur_length))
      count++;
   if (destp != NULL) {
      ProtoThing *dest = (ProtoThing *)
	 safe_malloc(count * sizeof(ProtoThing));
      int i = 0;
      for (cur_src = src, cur_length = length;
	   is_protoinwad(cur_src, cur_length);
	   next_protoinwad(&cur_src, &cur_length), i++)
	 decode_protoinwad(&dest[i], cur_src);
      *destp = dest;
   }
   return count;
}

/* Return a copy of a string embedded in a ProtoThing_inwad block.
 * Return NULL if there is no such string.  */
static char *
protoinwad_strdup(const ProtoThing_inwad *src, int ofs)
{
   if (ofs == 0)
      return NULL;
   else {
      const char *str = ((const char *) src) + ofs;
      size_t maxbytes = src->block_length - ofs;
      /* If BLOCK_LENGTH is 3 and OFS is 1, the string can span at
         most 2 bytes so MAXBYTES is 2.  But the length of the string
         must be less, since '\0' needs one byte.  */
      if (strnlen(str, maxbytes) == maxbytes) {
	 logprintf(LOG_ERROR, 'O', _("Unterminated string in PROTOS lump"));
	 return NULL;
      } else
	 return safe_strdup(str);
   }
}


/* Encode */

ProtoThing_inwad *
encode_protoinwad(const ProtoThing *src, size_t *lengthp)
{
   ProtoThing_inwad *dest;
   size_t length = sizeof(ProtoThing_inwad);
   size_t getsofs = 0;
   size_t getsinwad_len;
   Gets_inwad *getsinwad = encode_getsinwad_array(src->gets, src->ngets,
						  &getsinwad_len);
   size_t firstpickupmsgofs = 0;
   size_t pickupmsgofs = 0;
   size_t ignoremsgofs = 0;
   int i;
   /* compute locations of parts and increment length as needed */
   if (getsinwad != NULL) {
      /* align the structure nicely */
      getsofs = ALIGN(length, ALIGN_GETS);
      length = getsofs + getsinwad_len;
   }
   if (src->firstpickupmsg != NULL) {
      /* strings don't need alignment */
      firstpickupmsgofs = length;
      length = firstpickupmsgofs + strlen(src->firstpickupmsg) + 1;
   }
   if (src->pickupmsg != NULL) {
      pickupmsgofs = length;
      length = pickupmsgofs + strlen(src->pickupmsg) + 1;
   }
   if (src->ignoremsg != NULL) {
      ignoremsgofs = length;
      length = ignoremsgofs + strlen(src->ignoremsg) + 1;
   }
   length = ALIGN(length, ALIGN_PROTOTHING); /* to align the next block */
   /* now allocate.  clear the area so the WAD looks neater and compresses better */
   dest = (ProtoThing_inwad *) safe_calloc(length, 1);
   dest->block_length = length;
   /* commence copying */
   dest->id         = src->id;
   dest->hits       = src->hits;
   dest->damage     = src->damage;
   dest->realmass   = src->realmass;
   dest->friction   = src->friction;
   dest->radius     = src->radius;
   dest->height     = src->height;
   dest->sound      = src->sound;
   dest->flags      = src->flags;
   dest->speed      = src->speed;
   dest->shootarc_h = src->shootarc_h;
   dest->shootarc_v = src->shootarc_v;
   dest->see_arc    = src->see_arc;
   dest->aim_arc    = src->aim_arc;
   dest->shootnum   = src->shootnum;
   dest->spawn1     = src->spawn1;
   dest->spawn2     = src->spawn2;
   dest->spawnmax   = src->spawnmax;
   dest->become1    = src->become1;
   dest->become2    = src->become2;
   dest->bloodtype  = src->bloodtype;
   dest->phase_id   = src->phase_id;
   for (i = 0; i < NUM_THINGSIGS; i++)
      dest->signals[i] = src->signals[i];
   strncpy(dest->sprite, src->sprite, sizeof(dest->sprite));
   /* then the hard part */
   dest->ngets = src->ngets;
   dest->getsofs = getsofs;
   if (getsinwad != NULL) {
      memcpy(((char *) dest) + getsofs, getsinwad, getsinwad_len);
      safe_free(getsinwad);
   }
   dest->firstpickupmsgofs = firstpickupmsgofs;
   if (firstpickupmsgofs != 0)
      strcpy(((char *) dest) + firstpickupmsgofs,
	     src->firstpickupmsg);
   dest->pickupmsgofs = pickupmsgofs;
   if (pickupmsgofs != 0)
      strcpy(((char *) dest) + pickupmsgofs,
	     src->pickupmsg);
   dest->ignoremsgofs = ignoremsgofs;
   if (ignoremsgofs != 0)
      strcpy(((char *) dest) + ignoremsgofs,
	     src->ignoremsg);
   /* whoa, finished! */
   *lengthp = length;
   return dest;
}

// Local Variables:
// c-basic-offset: 3
// End:
