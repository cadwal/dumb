/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/fontmap.c: Mapping font usage codes to lump names.
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

#include <assert.h>
#include <string.h>

#include "libmissing/libmissing.h"
#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "fontinwad.h"
#include "fontmap.h"

typedef struct {
   char *lumpname;
} FontMapping;

/* typedef in fontmap.h */
struct FontMap {
   unsigned nmappings;
   FontMapping *mappings;
};

FontMap *game_fontmap = NULL;


FontMap *
new_fontmap(void)
{
   FontMap *fm = (FontMap *) safe_malloc(sizeof(FontMap));
   fm->nmappings = 0;
   fm->mappings = NULL;
   return fm;
}

const char *
fontmapping(const FontMap *fm, unsigned mapping_id)
{
   if (mapping_id < fm->nmappings)
      return fm->mappings[mapping_id].lumpname;
   else
      return NULL;
}

void
set_fontmapping(FontMap *fm, unsigned mapping_id, const char *fontname)
{
   if (mapping_id >= fm->nmappings) {
      unsigned inited = fm->nmappings;
      fm->nmappings = mapping_id + 1;
      fm->mappings = (FontMapping *)
	 safe_realloc(fm->mappings, fm->nmappings * sizeof(FontMapping));
      for (; inited < fm->nmappings; inited++)
	 fm->mappings[inited].lumpname = NULL;
   }
   if (fm->mappings[mapping_id].lumpname != NULL)
      safe_free(fm->mappings[mapping_id].lumpname);
   fm->mappings[mapping_id].lumpname = safe_strdup(fontname);
}

unsigned
fontmap_length(const FontMap *fm)
{
   return fm->nmappings;
}

Font *
load_mapped_font(const FontMap *fm, unsigned mapping_id)
{
   const char *fontname = fontmapping(fm, mapping_id);
   if (fontname == NULL) {
      logprintf('F', LOG_ERROR, _("No font defined for FontMapping %d"),
		mapping_id);
      return NULL;
   } else
      return load_font(fontname);
}

// Local Variables:
// c-basic-offset: 3
// End:
