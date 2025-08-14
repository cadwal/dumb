/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/font.c: Creating and using fonts.
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

/* See comment before dumb_towlower().
   This macro may be undefined by <config.h>.  */
#define POSSIBLY_BAD_TOWLOWER 1

#include <config.h>

#include <assert.h>
#if POSSIBLY_BAD_TOWLOWER
# include <ctype.h>
#endif
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/endiantypes.h"
#include "libdumbutil/safem.h"
#include "libdumbutil/utf8.h"

#include "font.h"

typedef struct {
   /* If tex.name[0]=='\0', the entire FontChar is invalid and
      font_wchar_nofallback() will return NULL when asked for this
      character.

      Else, if tex.lumpnum==BAD_LUMPNUM, the texture hasn't been
      located yet -- perhaps we don't even have the WAD it is in.  In
      this case, only tex.name and descent are valid.  Note especially
      that tex.width, tex.texels and tex.type can contain garbage.

      Else, the texture has been located and the validity rules of
      Texture apply.  */
   Texture tex;
   int descent;
} FontChar;

/* typedef is in font.h */
struct Font {
   int separation;
   int space_width;
   char *after, *before;
   FontChar ****p4;		/* see font_wchar_nofallback() */
};

static int font_wchar_width(const Font *font, wchar_t wc);
static FontChar *add_font_wchar(Font *, wchar_t);
static FontChar *font_wchar_nofallback(const Font *, wchar_t);
static FontChar *font_wchar(const Font *, wchar_t);
static wint_t dotless(wchar_t);
#if POSSIBLY_BAD_TOWLOWER
static wint_t dumb_towlower(wint_t);
#endif


/* Construct and destruct */

Font *
new_font(void)
{
   Font *font = (Font *) safe_malloc(sizeof(Font));
   font->separation = 0;
   font->space_width = 0;
   font->after = NULL;
   font->before = NULL;
   font->p4 = NULL;
   return font;
}

void
free_font(Font *font)
{
   FontChar ****p4 = font->p4;
   if (p4 != NULL) {
      int i4;
      for (i4 = 0; i4 < 256; i4++) {
	 FontChar ***p3 = p4[i4];
	 if (p3 != NULL) {
	    int i3;
	    for (i3 = 0; i3 < 256; i3++) {
	       FontChar **p2 = p3[i3];
	       if (p2 != NULL) {
		  int i2;
		  for (i2 = 0; i2 < 256; i2++) {
		     FontChar *p1 = p2[i2];
		     if (p1 != NULL) {
			int i1;
			for (i1 = 0; i1 < 256; i1++) {
			   FontChar *fc = &p1[i1];
			   if (fc->tex.name[0] != '\0'
			       && LUMPNUM_OK(fc->tex.lumpnum))
			      free_texels(&fc->tex);
			   /* else the character is invalid */
			} /* for i1 */
			safe_free(p1);
		     } /* if p1 */
		  } /* for i2 */
		  safe_free(p2);
	       } /* if p2 */
	    } /* for i3 */
	    safe_free(p3);
	 } /* if p3 */
      }	/* for i4 */
      safe_free(p4);
   } /* if p4 */
   if (font->after)
      safe_free(font->after);
   if (font->before)
      safe_free(font->before);
   safe_free(font);
}


/* Examine */

void
get_font_delimlumps(const Font *font,
		    const char **afterp, const char **beforep)
{
   *afterp = font->after;
   *beforep = font->before;
}

Texture *
font_wchar_tex(const Font *font, wchar_t wc, int *descentp)
{
   FontChar *fc = font_wchar(font, wc);
   if (fc == NULL)
      return NULL;
   if (!LUMPNUM_OK(fc->tex.lumpnum)) {
      /* The texture hasn't been located yet.  Find it.  */
      fc->tex.lumpnum = safe_lookup_lump(fc->tex.name, font->after, font->before,
					 LOG_WARNING);
      if (!LUMPNUM_OK(fc->tex.lumpnum)) {
	 /* TODO: remember we already tried */
	 return NULL;
      }
      /* memset(&fc->tex, 0, sizeof(fc->tex));
	 but that would lose tex.name and tex.lumpnum.  So we zero the
	 texture in add_font_wchar() instead.  */
      fc->tex.type = TT_FONT;
      guess_sprite_size(&fc->tex);
   }
   if (descentp != NULL)
      *descentp = fc->descent;
   return &fc->tex;
}

const char *
font_wchar_texref(const Font *font, wchar_t wc, int *descentp)
{
   FontChar *fc = font_wchar_nofallback(font, wc);
   if (fc == NULL)
      return NULL;
   if (descentp != NULL)
      *descentp = fc->descent;
   return fc->tex.name;
}

int
font_separation(const Font *font)
{
   return font->separation;
}

int
font_space_width(const Font *font)
{
   return font->space_width;
}

static int
font_wchar_width(const Font *font, wchar_t wc)
{
   Texture *tex = font_wchar_tex(font, wc, NULL);
   if (tex != NULL)
      return tex->width;
   else
      return font_space_width(font);
}

int
font_utf8_text_width(const Font *font, const char *text, size_t textlen)
{
   int width = 0;
   int first = 1;
   utf8_mbstate_t state;
   memset(&state, 0, sizeof(state));
   for (;;) {
      wchar_t wc;
      size_t mblen = utf8_mbrtowc(&wc, text, textlen, &state);
      if (mblen == (size_t) -2	/* incomplete multibyte char */
	  || mblen == (size_t) -1 /* invalid multibyte char (errno=EILSEQ) */
	  || mblen == 0)	/* successful end */
	 break;
      if (!first)
	 width += font_separation(font);
      width += font_wchar_width(font, wc);
      text += mblen;
      textlen -= mblen;
      first = 0;
   }
   return width;
}

int
font_wc_text_width(const Font *font, const wchar_t *text, size_t textlen)
{
   int width = 0;
   int first = 1;
   while (textlen--) {
      if (!first)
	 width += font_separation(font);
      width += font_wchar_width(font, *text++);
      first = 0;
   }
   return width;
}

int
font_utf8_str_width(const Font *font, const char *str)
{
   return font_utf8_text_width(font, str, strlen(str));
}

void
font_foreach_wchar(const Font *font,
		   void (*func)(void *cookie, wchar_t),
		   void *cookie)
{
   FontChar ****p4 = font->p4;
   if (p4 != NULL) {
      int i4;
      for (i4 = 0; i4 < 256; i4++) {
	 FontChar ***p3 = p4[i4];
	 if (p3 != NULL) {
	    int i3;
	    for (i3 = 0; i3 < 256; i3++) {
	       FontChar **p2 = p3[i3];
	       if (p2 != NULL) {
		  int i2;
		  for (i2 = 0; i2 < 256; i2++) {
		     FontChar *p1 = p2[i2];
		     if (p1 != NULL) {
			int i1;
			for (i1 = 0; i1 < 256; i1++) {
			   if (p1[i1].tex.name[0] != '\0')
			      (*func)(cookie, ((i4 << 24) | (i3 << 16)
					       | (i2 << 8) | i1));
			} /* for i1 */
		     } /* if p1 */
		  } /* for i2 */
	       } /* if p2 */
	    } /* for i3 */
	 } /* if p3 */
      }	/* for i4 */
   } /* if p4 */
}


/* Modify */

void
set_font_delimlumps(Font *font, const char *after, const char *before)
{
   if (font->after != NULL) {
      safe_free(font->after);
      font->after = NULL;
   }
   if (font->before != NULL) {
      safe_free(font->before);
      font->before = NULL;
   }
   
   if (after != NULL)
      font->after = safe_strdup(after);
   if (before != NULL)
      font->before = safe_strdup(before);
}

void
set_font_wchar_texref(Font *font, wchar_t wc,
		      const char *texname, int descent)
{
   FontChar *fc = add_font_wchar(font, wc);
   /* We currently cannot change a texture once it has been chosen.
      Implement that later if it seems useful.  */
   assert(fc->tex.name[0] == '\0');
   strncpy(fc->tex.name, texname, sizeof(fc->tex.name)-1);
   fc->tex.name[sizeof(fc->tex.name)-1] = '\0';
   /* Now that there's a name, lumpnum may no longer contain garbage.  */
   fc->tex.lumpnum = BAD_LUMPNUM;
   fc->descent = descent;
}

void
set_font_wchar_descent(Font *font, wchar_t wc, int descent)
{
   FontChar *fc = font_wchar_nofallback(font, wc);
   /* FIXME: Print a proper error message, since this may be caused by
      an invalid .pt file.  */
   assert(fc);
   fc->descent = descent;
}

void
set_font_separation(Font *font, int separation)
{
   font->separation = separation;
}

void
set_font_space_width(Font *font, int space_width)
{
   font->space_width = space_width;
}


/* Internals */

/* Extend the font pointer arrays of FONT so that character C has a
   slot, and return its address.  */
static FontChar *
add_font_wchar(Font *font, wchar_t wc)
{
   /* were this C++, these four would be references */
   FontChar *****p4p, ****p3p, ***p2p, **p1p, *fc;
   p4p = &font->p4;
   if (!*p4p) {
      int i;
      *p4p = (FontChar ****) safe_malloc(256 * sizeof(FontChar ***));
      /* don't assume NULL has an all-zero representation */
      for (i = 0; i < 256; i++)
	 (*p4p)[i] = NULL;
   }
   p3p = &(*p4p)[(((unsigned long) wc) >> 24) & 0xFF];
   if (!*p3p) {
      int i;
      *p3p = (FontChar ***) safe_malloc(256 * sizeof(FontChar **));
      for (i = 0; i < 256; i++)
	 (*p3p)[i] = NULL;
   }
   p2p = &(*p3p)[(((unsigned long) wc) >> 16) & 0xFF];
   if (!*p2p) {
      int i;
      *p2p = (FontChar **) safe_malloc(256 * sizeof(FontChar *));
      for (i = 0; i < 256; i++)
	 (*p2p)[i] = NULL;
   }
   p1p = &(*p2p)[(((unsigned long) wc) >> 8) & 0xFF];
   if (!*p1p) {
      /* Actually we shouldn't clear the structure until in
         font_wchar_tex(), but it's easier to do it here.  */
      *p1p = (FontChar *) safe_calloc(256, sizeof(FontChar));
      /* If we didn't clear the entire array above, we'd do:
	 int i;
	 for (i = 0; i < 256; i++)
	    (*p1p)[i].tex.name[0] = '\0';  */
   }
   fc = &(*p1p)[(((unsigned long) wc) >> 0) & 0xFF];
   return fc;
}

static FontChar *
font_wchar_nofallback(const Font *font, wchar_t wc)
{
   FontChar ****p4, ***p3, **p2, *p1, *fc;
   p4 = font->p4;
   if (!p4)
      return NULL;
   p3 = p4[(((unsigned long) wc) >> 24) & 0xFF];
   if (!p3)
      return NULL;
   p2 = p3[(((unsigned long) wc) >> 16) & 0xFF];
   if (!p2)
      return NULL;
   p1 = p2[(((unsigned long) wc) >> 8) & 0xFF];
   if (!p1)
      return NULL;
   fc = &p1[(((unsigned long) wc) >> 0) & 0xFF];
   if (fc->tex.name[0] == '\0')
      return NULL;
   return fc;
}

/* Like font_wchar_nofallback(), but if the character isn't found, try
   to replace it with a related character.

   This is called by font_wchar_tex() and indirectly by functions
   related to drawing with the font.  font_wchar_texref() and
   set_font_wchar_descent() use font_wchar_nofallback() instead.  */
static FontChar *
font_wchar(const Font *font, wchar_t wc)
{
   FontChar *fc = font_wchar_nofallback(font, wc);
   if (fc != NULL)
      return fc;
   if (iswlower(wc)) {
      fc = font_wchar_nofallback(font, towupper(wc));
      if (fc != NULL)
	 return fc;
   }
   if (iswupper(wc)) {
#if POSSIBLY_BAD_TOWLOWER
      fc = font_wchar_nofallback(font, dumb_towlower(wc));
#else
      fc = font_wchar_nofallback(font, towlower(wc));
#endif
      if (fc != NULL)
	 return fc;
   }
   {
      wint_t replacement = dotless(wc);
      if (replacement != WEOF)
	 return font_wchar(font, replacement); /* recurse */
   }      
   return NULL;
}

/* font_wchar() calls this if it can't find a character in a font.
   This function returns a replacement character with diacriticals
   removed, or WEOF.  This way, we can get at least some output.

   It would make sense to convert e.g. U+00C6 (LATIN CAPITAL LETTER
   AE) to the sequence U+0041 U+0045, but that would require changes
   elsewhere.  */
static wint_t
dotless(wchar_t wc)
{
   /* Be careful not to set up replacement loops.  */
   switch (wc) {
      /* We do not use L'x' here, since that could depend on the
	 character set of the compiler.  We know the values are in
	 Unicode.  */
   case 0x00C4:			/* LATIN CAPITAL LETTER A WITH DIAERESIS */
      return 0x0041;		/* LATIN CAPITAL LETTER A */
   case 0x00C5:			/* LATIN CAPITAL LETTER A WITH RING ABOVE */
      return 0x0041;		/* LATIN CAPITAL LETTER A */
   case 0x00D6:			/* LATIN CAPITAL LETTER O WITH DIAERESIS */
      return 0x004F;		/* LATIN CAPITAL LETTER O */
   case 0x00E4:			/* LATIN SMALL LETTER A WITH DIAERESIS */
      return 0x0061;		/* LATIN SMALL LETTER A */
   case 0x00E5:			/* LATIN SMALL LETTER A WITH RING ABOVE */
      return 0x0061;		/* LATIN SMALL LETTER A */
   case 0x00F6:			/* LATIN SMALL LETTER O WITH DIAERESIS */
      return 0x006F;		/* LATIN SMALL LETTER O */
   default:
      return WEOF;
   }
}

#if POSSIBLY_BAD_TOWLOWER
/* In glibc-2.1.1, towlower() does the same as towupper().  So we
   route the calls via dumb_towlower(), which detects and circumvents
   this bug.  */
static wint_t
dumb_towlower(wint_t wc)
{
   static int inited = 0;
   static int buggy;
   /* This test shouldn't be moved to the configure script, since the
      user may update libc without recompiling DUMB.  */
   if (!inited) {
      inited = 1;
      /* Parentheses around towlower avoid a macro version which
         doesn't have the bug.  */
      if ((towlower)(L'x') == L'X') {
	 buggy = 1;
	 /* Don't call logprintf() until both INITED and BUGGY have
            been set... this should avoid infinite recursion in case
            logprintf() is some day changed to use fonts.  */
	 logprintf(LOG_WARNING, 'F', "Your towlower() is buggy!");
      } else
	 buggy = 0;
   }
   if (buggy) {
      /* This hack assumes wchar_t 1 to 126 are the same as char */
      if (wc > 0 && wc < 127)
	 return (wint_t) tolower((char) wc);
      else
	 return wc;
   } else
      return towlower(wc);
}
#endif /* POSSIBLY_BAD_TOWLOWER */
      
// Local Variables:
// c-basic-offset: 3
// End:
