/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/font.h: Creating and using fonts.
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

#ifndef LIBDUMB_FONT_H
#define LIBDUMB_FONT_H

/* Get wchar_t.  In C++, it is a built-in type.  */
#ifndef __cplusplus
# include <stddef.h>
#endif

#include "libdumb/texture.h"	/* typedef Texture */

typedef struct Font Font;	/* opaque */


/* Construct and destruct */

/* Create a new font.  */
Font *new_font(void);

/* Free FONT and the character shapes to which it refers.  */
void free_font(Font *font);


/* Examine */

/* See set_font_delimlumps().  Neither AFTERP nor BEFOREP may be NULL.
   The pointers saved in *AFTERP and *BEFOREP will be valid until the
   font is freed or the delimiters are changed.  This means you cannot
   do the following:

   const char *after, *before;
   get_font_delimlumps(font, &after, &before);
   set_font_delimlumps(font, after, before);

   because set_font_delimlumps will free the old delimiters before it
   installs the new ones.  */
void get_font_delimlumps(const Font *font,
			 const char **afterp, const char **beforep);

/* Return the texture of WC.  If DESCENTP is non-NULL, save in
   *DESCENTP the number of pixels the texture should be drawn below
   the baseline.  Return NULL if the character doesn't exist.  */
Texture *font_wchar_tex(const Font *, wchar_t wc, int *descentp);

/* Return the name of the texture of WC.  If DESCENTP is non-NULL,
   save in *DESCENTP the number of pixels the texture should be drawn
   below the baseline.  Return NULL if the character doesn't exist.

   Unlike font_wchar_tex(), this does not require the texture lump to
   exist.  This function exists mainly to let ptcomp build fonts and
   then save them.

   The returned pointer will be valid until the font is freed or the
   character is modified.  */
const char *font_wchar_texref(const Font *, wchar_t wc, int *descentp);

/* Return how many empty pixels there should be between horizontally
   adjacent characters.  The number may be negative, meaning the
   characters should overlap.  */
int font_separation(const Font *);

/* Return how many empty pixels a space character is worth.  The usual
   separation pixels should also be added in both sides.  */
int font_space_width(const Font *);

/* To sum two widths returned by these functions, you must add the
   separation between them.  */
int font_utf8_text_width(const Font *font, const char text[], size_t len);
int font_wc_text_width(const Font *font, const wchar_t text[], size_t len);
int font_utf8_str_width(const Font *font, const char *str);

/* Call FUNC once for each existing character in FONT.  This is much
   more efficient than looping from WCHAR_MIN to WCHAR_MAX.  */
void font_foreach_wchar(const Font *font,
			void (*func)(void *cookie, wchar_t),
			void *cookie);


/* Modify */

/* All lump names passed to these functions must be null-terminated!  */

/* Copy the strings AFTER and BEFORE as the names of FONT's delimiter
   lumps.  This means the glyph lumps must be after AFTER and before
   BEFORE.  If a pointer is NULL, that end isn't delimited.  */
void set_font_delimlumps(Font *font, const char *after, const char *before);

/* Add character WC in FONT.  It will use the texture in lump TEXNAME
   and hang DESCENT pixels below the baseline.  FONT must not already
   contain WC.  */
void set_font_wchar_texref(Font *font, wchar_t wc,
			   const char *texname, int descent);

/* Change the descent of character WC which must already have been
   added in FONT.  */
void set_font_wchar_descent(Font *font, wchar_t wc, int descent);

/* Set the number returned by font_separation().  */
void set_font_separation(Font *font, int separation);

/* Set the number returned by font_space_width().  */
void set_font_space_width(Font *font, int space_width);


#endif /* LIBDUMB_FONT_H */

// Local Variables:
// c-basic-offset: 3
// End:
