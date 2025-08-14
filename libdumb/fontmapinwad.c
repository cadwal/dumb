/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/fontmapinwad.c: Loading and saving font maps.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include "libmissing/libmissing.h"
#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/endiantypes.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "fontinwad.h"
#include "fontmap.h"
#include "fontmapinwad.h"
#include "lumpver.h"

static const LumpVer ok_lumpver = { "DUMBfontmap", 1 };

typedef struct {
   char lumpname[LUMPNAMELEN];
} FontMapping_inwad;

typedef struct {
   LumpVer_inwad lumpver;
   /* FontMapping_inwad entries[]; */
} FontMap_inwad;


FontMap *
load_fontmap(void)
{
   LumpNum fontmap_ln;
   const FontMap_inwad *fmiw;
   const FontMapping_inwad *fmgiw;
   FontMap *fm;
   size_t nmappings, i;

   fontmap_ln = safe_lookup_lump("FONTMAP", NULL, NULL, LOG_WARNING);
   if (!LUMPNUM_OK(fontmap_ln))
      goto error;
   if (get_lump_len(fontmap_ln) < sizeof(FontMap_inwad)) {
      logprintf('F', LOG_ERROR, _("FONTMAP lump is too short for header"));
      goto error;
   }
   
   /* first resource allocation: load_lump() */
   fmiw = (const FontMap_inwad *) load_lump(fontmap_ln);
   if (!is_lumpver_ok(&fmiw->lumpver, &ok_lumpver, "FONTMAP"))
      goto error_freelump;
   nmappings = ((get_lump_len(fontmap_ln) - sizeof(FontMap_inwad))
		/ sizeof(FontMapping_inwad));
   logprintf('F', LOG_INFO,
	     _("init %lu font mappings"), (unsigned long) nmappings);
   fm = new_fontmap();
   fmgiw = (const FontMapping_inwad *) (fmiw + 1);
   for (i = 0; i < nmappings; i++) {
      if (fmgiw[i].lumpname[0] != '\0') {
	 char lumpname[LUMPNAMELEN+1];
	 memcpy(lumpname, fmgiw[i].lumpname, LUMPNAMELEN);
	 lumpname[LUMPNAMELEN] = '\0';
	 set_fontmapping(fm, i, lumpname);
      }
   }
   release_lump(fontmap_ln);
   return fm;

 error_freelump:
   free_lump(fontmap_ln);
 error:
   return NULL;
}

void
save_fontmap(WADWR *w, const FontMap *fm)
{
   unsigned id;
   wadwr_lump(w, "FONTMAP");
   {
      FontMap_inwad fmiw;
      set_lumpver(&fmiw.lumpver, &ok_lumpver);
      wadwr_write(w, &fmiw, sizeof(fmiw));
   }
   for (id = 0; id < fontmap_length(fm); id++) {
      FontMapping_inwad fmgiw;
      const char *fontname = fontmapping(fm, id);
      if (fontname == NULL)
	 fontname = "";
      strncpy(fmgiw.lumpname, fontname, sizeof(fmgiw.lumpname));
      wadwr_write(w, &fmgiw, sizeof(fmgiw));
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
