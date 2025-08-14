/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/gettableinwad.c: Encoding and decoding Gettables in WAD.
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

#include <config.h>

#include <assert.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/align.h"
#include "libdumbutil/endiantypes.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "gettableinwad.h"
#include "lumpver.h"


/* The GETTABLE lump consists of a Gettables_inwad header followed by
   blocks of varying length.  */

struct Gettables_inwad {
   LumpVer_inwad lumpver;
   /* struct Gettable_inwad getts[] */
};

/* Each block begins with a header which says how long the block is,
   including the header.  When DUMB starts up, it reads the blocks and
   converts them to the internal format defined in gettablestruct.h.

   Each block can also contain other information referred to by the
   header.  The location of each such piece of information is saved as
   a byte count from the beginning of the block.  Thus the meaning of
   a block does not depend on its position in the lump.

   The length of each block should be a multiple of 8.  */

struct Gettable_inwad {
   LE_int32 block_length;
   LE_flags32 flags;
   LE_int32 xo, yo;		/* IconPos <x:integer> <y:integer> */
   LE_int16 initial;		/* Initial <integer> */
   LE_int16 defaultmax;		/* DefaultMaximum <integer> */
   LE_int16 backpackmax;	/* WithBackpack <integer>, or ==defaultmax */
   LE_int16 key;
   LE_int16 ngets;		/* how many gettables this will give */
   LE_int16 getsofs;		/* where the Gets_inwad list begins */
   LE_int16 special;		/* see ../dumb/dumbdefs.pt */
   LE_int16 bogotype;
   LE_int16 powered_bogotype;
   LE_int16 weaponnumber;	/* which key to press; 0=none, 1...10 */
   LE_int16 replaceweapon;	/* gettable number to replace/disable */
   LE_int16 decay;
   char iconname[10];		/* FIXME: why 10? */
   char iconanim, _spare;
   LE_int16 timing;
   LE_int16 usesound;
};

static const LumpVer ok_lumpver = { "DUMB gettbl", 2 };

struct Gets_inwad {
   LE_int32 gtid;		/* Gettable ID */
   LE_int32 change;		/* >0 gives, <0 takes */
   LE_int32 maximum;		/* 0 means use default */
};


/* Decode */

const Gettable_inwad *
first_gettableinwad(const Gettables_inwad *lumpdata,
		    size_t *lengthp, const char *lumpname)
{
   if (*lengthp < sizeof(Gettables_inwad)) {
      if (lumpname != NULL)
	 logprintf(LOG_ERROR, 'G', _("Lump %s is too short"), lumpname);
      return NULL;
   }
   if (!is_lumpver_ok(&lumpdata->lumpver, &ok_lumpver, lumpname))
      return NULL;
   *lengthp -= sizeof(Gettables_inwad);
   return (Gettable_inwad *) (lumpdata + 1);
}

int
is_gettableinwad(const Gettable_inwad *data, size_t length)
{
   if (length == 0)
      return 0;
   else {
      size_t blk_length = data->block_length;
      if (blk_length < sizeof(Gettable_inwad) || blk_length > length) {
	 logprintf(LOG_ERROR, 'G', _("Gettable block chain corrupted"));
	 return 0;
      } else
	 return 1;
   }
}

void
next_gettableinwad(const Gettable_inwad **datap, size_t *lengthp)
{
   size_t blk_length = (*datap)->block_length;
   *datap = (const Gettable_inwad *)
      (((const char *) *datap) + blk_length);
   *lengthp -= blk_length;
}

void
decode_gettableinwad(Gettable *dest, const Gettable_inwad *src)
{
   /* make sure none of the old stuff remains; the assignments below
      should overwrite everything, though.  */
   memset(dest, 0, sizeof(*dest));
   /* endianness and/or width may be converted in the process, so we
      can't use memcpy() */
   dest->flags            = src->flags;
   dest->xo               = src->xo;
   dest->yo               = src->yo;
   dest->initial          = src->initial;
   dest->defaultmax       = src->defaultmax;
   dest->backpackmax      = src->backpackmax;
   dest->key              = src->key;
   dest->ngets            = src->ngets;
   dest->gets             = decode_getsinwad_array((const Gets_inwad *)
						   (((const char *) src) 
						    + src->getsofs),
						   src->ngets);
   dest->special          = src->special;
   dest->bogotype         = src->bogotype;
   dest->powered_bogotype = src->powered_bogotype;
   dest->weaponnumber     = src->weaponnumber;
   dest->replaceweapon    = src->replaceweapon;
   dest->decay            = src->decay;
   strncpy(dest->iconname, src->iconname, LUMPNAMELEN);
   dest->iconname[LUMPNAMELEN] = '\0';
   dest->iconanim         = src->iconanim;
   dest->timing           = src->timing;
   dest->usesound         = src->usesound;
}

size_t
decode_gettableinwad_array(Gettable **destp,
			   const Gettables_inwad *hdr, size_t length,
			   const char *lumpname)
{
   const Gettable_inwad *src, *cur_src;
   size_t cur_length;
   size_t count = 0;

   /* first parse the header, modifying the LENGTH parameter */
   src = first_gettableinwad(hdr, &length, lumpname);
   if (src == NULL) {
      if (destp != NULL)
	 *destp = NULL;
      return 0;
   }

   for (cur_src = src, cur_length = length;
	is_gettableinwad(cur_src, cur_length);
	next_gettableinwad(&cur_src, &cur_length))
      count++;
   if (destp != NULL) {
      Gettable *dest = (Gettable *)
	 safe_malloc(count * sizeof(Gettable));
      int i = 0;
      for (cur_src = src, cur_length = length;
	   is_gettableinwad(cur_src, cur_length);
	   next_gettableinwad(&cur_src, &cur_length), i++)
	 decode_gettableinwad(&dest[i], cur_src);
      *destp = dest;
   }
   return count;
}


/* Encode */

Gettables_inwad *
begin_gettablesinwad(size_t *lengthp)
{
   Gettables_inwad *hdr = (Gettables_inwad *)
      safe_malloc(sizeof(Gettables_inwad));
   set_lumpver(&hdr->lumpver, &ok_lumpver);
   *lengthp = sizeof(Gettables_inwad);
   return hdr;
}

Gettable_inwad *
encode_gettableinwad(const Gettable *src, size_t *lengthp)
{
   Gettable_inwad *dest;
   size_t length = sizeof(Gettable_inwad);
   size_t getsofs = 0;
   size_t getsinwad_len;
   Gets_inwad *getsinwad = encode_getsinwad_array(src->gets, src->ngets,
						  &getsinwad_len);
   /* compute locations of parts and increment length as needed */
   if (getsinwad != NULL) {
      /* align the structure nicely */
      getsofs = ALIGN(length, ALIGN_GETS);
      length = getsofs + getsinwad_len;
   }
   length = ALIGN(length, ALIGN_GETTABLE);
   /* Now allocate.  Clear the area so the WAD looks neater and
      compresses better.  */
   dest = (Gettable_inwad *) safe_calloc(length, 1);
   dest->block_length = length;
   /* commence copying */
   dest->flags            = src->flags;
   dest->xo               = src->xo;
   dest->yo               = src->yo;
   dest->initial          = src->initial;
   dest->defaultmax       = src->defaultmax;
   dest->backpackmax      = src->backpackmax;
   dest->key              = src->key;
   dest->ngets            = src->ngets;
   dest->getsofs          = getsofs;
   if (getsinwad != NULL) {
      memcpy(((char *) dest) + getsofs, getsinwad, getsinwad_len);
      safe_free(getsinwad);
   }
   dest->special          = src->special;
   dest->bogotype         = src->bogotype;
   dest->powered_bogotype = src->powered_bogotype;
   dest->weaponnumber     = src->weaponnumber;
   dest->replaceweapon    = src->replaceweapon;
   dest->decay            = src->decay;
   assert(src->iconname[LUMPNAMELEN] == '\0');
   strcpy(dest->iconname, src->iconname);
   dest->iconanim         = src->iconanim;
   dest->timing           = src->timing;
   dest->usesound         = src->usesound;
   /* whoa, finished! */
   *lengthp = length;
   return dest;
}


/* Gets */

Gets *
decode_getsinwad_array(const Gets_inwad getsinwad[], size_t ngets)
{
   if (ngets == 0)
      return NULL;
   else {
      Gets *gets = (Gets *) safe_malloc(ngets * sizeof(Gets));
      size_t i;
      for (i = 0; i < ngets; i++) {
	 gets[i].gtid    = getsinwad[i].gtid;
	 gets[i].change  = getsinwad[i].change;
	 gets[i].maximum = getsinwad[i].maximum;
      }
      return gets;
   }
}

Gets_inwad *
encode_getsinwad_array(const Gets gets[], size_t ngets, size_t *lengthp)
{
   if (ngets == 0) {
      *lengthp = 0;
      return NULL;
   } else {
      Gets_inwad *getsinwad = (Gets_inwad *)
	 safe_malloc(ngets * sizeof(Gets_inwad));
      size_t i;
      for (i = 0; i < ngets; i++) {
	 getsinwad[i].gtid = gets[i].gtid;
	 getsinwad[i].change = gets[i].change;
	 getsinwad[i].maximum = gets[i].maximum;
      }
      *lengthp = ngets * sizeof(Gets_inwad);
      return getsinwad;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
