/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/fontcomp.c: Font compiler.
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

#include "libdumbutil/dumb-nls.h"

#include "libmissing/libmissing.h"
#include "libdumbutil/safem.h"
#include "libdumb/fontinwad.h"
#include "token.h"
#include "parm.h"
#include "fontcomp.h"

typedef struct FontLink {
   struct FontLink *next;
   char name[LUMPNAMELEN+1];
   Font *font;
} FontLink;

static FontLink *first_fontlink;
static unsigned nfonts;


void
init_fontcomp(void)
{
   first_fontlink = NULL;
   nfonts = 0;
}

void
fontcomp(void)
{
   const char *token;
   Font *font = new_font();
   {
      FontLink *newlink = (FontLink *) safe_malloc(sizeof(FontLink));
      newlink->next = first_fontlink;
      first_fontlink = newlink;
      nfonts++;
      parm_str(newlink->name, LUMPNAMELEN+1);
      newlink->font = font;
   }
   while (1) {
      token = next_token();
      if (class_of_token(token) == TOKENCL_EOF)
	 return;
      else if (class_of_token(token) == TOKENCL_NEWLINE)
	 ;
      else if (!strcasecmp(token, "SpaceCharWidth"))
	 set_font_space_width(font, parm_int());
      else if (!strcasecmp(token, "Separation"))
	 set_font_separation(font, parm_int());
      else if (!strcasecmp(token, "Delimiters")) {
	 char *after = parm_strdup();
	 char *before = parm_strdup();
	 set_font_delimlumps(font, after, before);
	 safe_free(after);
	 safe_free(before);
      } else if (!strcasecmp(token, "CodingRange")) {
	 unsigned long first = parm_ulong();
	 unsigned long last = parm_ulong();
	 char *format = parm_strdup();
	 int arg = parm_int_opt(0);
	 unsigned long c;
	 for (c = first; c <= last; c++, arg++) {
	    char texname[LUMPNAMELEN+1];
	    /* FIXME: parse_printf_format() */
	    if (snprintf(texname, sizeof(texname), format, arg)
		> LUMPNAMELEN)
	       err(_("Formatted string is too long at char %#04lx"), c);
	    set_font_wchar_texref(font, (wchar_t) c, texname,
				  /* descent: */ 0);
	 }
	 safe_free(format);
      } else if (!strcasecmp(token, "Descent")) {
	 unsigned long wc = parm_ulong();
	 int descent = parm_int();
	 set_font_wchar_descent(font, (wchar_t) wc, descent);
      } else
	 break;
   }
   unget_token();
}

void
wrfonts(WADWR *w)
{
   const FontLink *p;
   printf(_("%5u fonts:"), nfonts);
   fflush(stdout);
   for (p = first_fontlink; p != NULL; p = p->next) {
      save_font(w, p->font, p->name);
      printf(" %s", p->name);
      fflush(stdout);
   }
   putchar('\n');
}

// Local Variables:
// c-basic-offset: 3
// End:
