/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/fontinwad.c: Loading and saving fonts.
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

#include "libdumbutil/dumb-nls.h"

#include "libdumbwad/wadio.h"
#include "fontinwad.h"
#include "lumpver.h"

typedef struct {
   LE_uint32 charid;
   LE_int16 descent;
   LE_int16 spare;
   char lumpname[LUMPNAMELEN];	/* not null-terminated */
} FontChar_inwad;

static const LumpVer ok_lumpver = { "DUMB font", 2 }; /* v1 was never released */

typedef struct {
   LumpVer_inwad lumpver;
   LE_int16 separation;
   LE_int16 space_width;
   /* names of lumps after and before which the character shapes are;
      not null-terminated */
   char after[LUMPNAMELEN], before[LUMPNAMELEN];
   /* FontChar_inwad chars[]; */
} Font_inwad;

struct save_font_context {
   const Font *font;
   WADWR *wadwr;
};

static void save_font_wchar(void *cookie, wchar_t);


Font *
load_font(const char *fontlumpname)
{
   Font *font;
   const Font_inwad *font_inwad;
   size_t nchars;
   char after[LUMPNAMELEN+1], before[LUMPNAMELEN+1];
   const FontChar_inwad *fciw;
   LumpNum font_ln = safe_lookup_lump(fontlumpname, NULL, NULL, LOG_WARNING);
   if (!LUMPNUM_OK(font_ln))
      goto error;
   if (get_lump_len(font_ln) < sizeof(Font_inwad)) {
      logprintf('F', LOG_ERROR,
		_("Font lump %s is too short for header"), fontlumpname);
      goto error;
   }

   /* first resource allocation: load_lump() */
   font_inwad = (const Font_inwad *) load_lump(font_ln);
   if (!is_lumpver_ok(&font_inwad->lumpver, &ok_lumpver, fontlumpname))
      goto error_freelump;
   nchars = ((get_lump_len(font_ln) - sizeof(Font_inwad))
	     / sizeof(FontChar_inwad));
   /* copy the char arrays to null-terminated strings */
   strncpy(after, font_inwad->after, LUMPNAMELEN);
   after[LUMPNAMELEN] = '\0';
   strncpy(before, font_inwad->before, LUMPNAMELEN);
   before[LUMPNAMELEN] = '\0';

   /* second resource allocation: new_font() */
   font = new_font();
   set_font_separation(font, font_inwad->separation);
   set_font_space_width(font, font_inwad->space_width);
   set_font_delimlumps(font,
		       after[0] ? after : NULL,
		       before[0] ? before : NULL);
   logprintf('F', LOG_INFO, _("init %lu characters in font %s"),
	     (unsigned long) nchars, fontlumpname);

   for (fciw = (const FontChar_inwad *) (font_inwad + 1);
	nchars > 0;
	nchars--, fciw++) {
      /* fciw->lumpname is not null-terminated, so copy it to a local array */
      char glyphlumpname[sizeof(fciw->lumpname) + 1];
      strncpy(glyphlumpname, fciw->lumpname, sizeof(fciw->lumpname));
      glyphlumpname[sizeof(fciw->lumpname)] = '\0';
      set_font_wchar_texref(font, fciw->charid,
			    glyphlumpname, fciw->descent);
   }

   release_lump(font_ln);
   return font;

 error_freelump:
   free_lump(font_ln);
 error:
   return NULL;
}

void
save_font(WADWR *wadwr, const Font *font, const char *lumpname)
{
   wadwr_lump(wadwr, lumpname);
   /* header */
   {
      Font_inwad fiw;
      const char *after, *before;
      memset(&fiw, 0, sizeof(fiw)); /* for after[], before[] */
      set_lumpver(&fiw.lumpver, &ok_lumpver);
      fiw.separation = font_separation(font);
      fiw.space_width = font_space_width(font);
      get_font_delimlumps(font, &after, &before);
      if (after != NULL)
	 strncpy(fiw.after, after, sizeof(fiw.after));
      if (before != NULL)
	 strncpy(fiw.before, before, sizeof(fiw.before));
      wadwr_write(wadwr, &fiw, sizeof(fiw));
   }
   /* body */
   {
      struct save_font_context context;
      context.font = font;
      context.wadwr = wadwr;
      font_foreach_wchar(font, save_font_wchar, (void *) &context);
   }
}

static void
save_font_wchar(void *cookie, wchar_t wc)
{
   const struct save_font_context *context
      = (struct save_font_context *) cookie;
   const char *glyphlumpname;
   int descent;
   FontChar_inwad fciw;
   glyphlumpname = font_wchar_texref(context->font, wc, &descent);
   assert(glyphlumpname != NULL); /* since we got wc from font_foreach_char() */
   memset(&fciw, 0, sizeof(fciw)); /* for fciw.spare */
   fciw.charid = wc;
   fciw.descent = descent;
   strncpy(fciw.lumpname, glyphlumpname, sizeof(fciw.lumpname));
   wadwr_write(context->wadwr, &fciw, sizeof(fciw));
}

// Local Variables:
// c-basic-offset: 3
// End:
