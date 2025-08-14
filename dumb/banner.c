/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/banner.c: Banners that scroll over the view.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "libdumbutil/utf8.h"
#include "libdumb/font.h"
#include "draw.h"
#include "banner.h"

typedef struct _BanLink {
   struct _BanLink *next;
   Texture *t;
   int len, prog;
   int x, y;
} BanLink;

typedef struct {
   BanLink *list, *last;
   int start, stop, speed, baseline;
   int used;
} Banner;

static BanLink *
enqueue(Banner *b)
{
   BanLink *l = (BanLink *) safe_calloc(1, sizeof(BanLink));
   if (b->list == NULL)
      b->list = b->last = l;
   else {
      b->last->next = l;
      b->last = l;
   }
   return l;
}

static void
dequeue(Banner *b)
{
   BanLink *l = b->list;
   b->list = l->next;
   if (b->list == NULL)
      b->last = NULL;
   safe_free(l);
}


#define MAX_BANNERS 16
Banner bnr[MAX_BANNERS];
int nbnrs = 0;

int
init_banner(int baseline, int start, int stop, int speed)
{
   int i;
   Banner *b;
   for (i = 0; i < nbnrs; i++)
      if (!bnr[i].used)
	 break;
   if (i == nbnrs) {
      if (nbnrs >= MAX_BANNERS)
	 logfatal('B', _("too many banners"));
      nbnrs++;
   }
   b = bnr + i;
   b->last = b->list = NULL;
   b->start = start;
   b->stop = stop;
   b->speed = speed;
   b->baseline = baseline;
   b->used = 1;
   logprintf(LOG_INFO, 'B', _("init banner %d"), i);
   return i;
}

void
reset_banner(int banner)
{
   Banner *b = bnr + banner;
   while (b->list)
      dequeue(b);
   b->used = 0;
}

void
reset_banners(void)
{
   int i;
   for (i = 0; i < nbnrs; i++)
      reset_banner(i);
   nbnrs = 0;
}

void
add_to_banner(int banner, Texture *t, int xoffset, int yoffset)
{
   Banner *b = bnr + banner;
   BanLink *l = enqueue(b);
   l->t = t;
   l->len = xoffset + (t ? t->width : 0);
   l->y = b->baseline + yoffset - (t ? t->height : 0);
   l->x = xoffset;
   l->prog = 0;
}

static void
add_wchar_to_banner(int banner, const Font *font, wchar_t wc)
{
   int descent;
   Texture *tex = font_wchar_tex(font, wc, &descent);
   if (!tex) {
      /* No such character.  Substitute a space.  */
      add_to_banner(banner, NULL,
		    font_separation(font) + font_space_width(font), 0);
   } else
      add_to_banner(banner, tex, font_separation(font), descent);
}

void
add_utf8_text_to_banner(int banner, const Font *font, const char *text, int textlen)
{
   utf8_mbstate_t state;
   memset(&state, 0, sizeof(state));
   for (;;) {
      wchar_t wc;
      size_t mblen = utf8_mbrtowc(&wc, text, textlen, &state);
      if (mblen == (size_t) -2	/* incomplete multibyte char */
	  || mblen == (size_t) -1 /* invalid multibyte char (errno=EILSEQ) */
	  || mblen == 0)	/* successful end */
	 break;
      add_wchar_to_banner(banner, font, wc);
      text += mblen;
      textlen -= mblen;
   }
}

void
add_utf8_str_to_banner(int banner, const Font *font, const char *str)
{
   add_utf8_text_to_banner(banner, font, str, strlen(str));
}

static void
draw_banner(int banner, void *fb)
{
   Banner *b = bnr + banner;
   BanLink *l = b->list;
   int x /*,i=0 */ ;
   if (!l)
      return;
   x = b->start - l->prog;
   while (l && x < b->stop) {
      /*logprintf(LOG_DEBUG, 'B', _("drawing banner %d item %d @ (%d,%d)"),
         banner, i++, x + l->x, l->y); */
      if (l->t)
	 draw(fb, l->t, x + l->x, l->y);
      x += l->len;
      l = l->next;
   }
}

static void
update_banner(int banner, void *fb, int ticks)
{
   Banner *b = bnr + banner;
   BanLink *l = b->list;
   l->prog += (ticks * b->speed + 7) / 8;
   if (l->prog >= l->len)
      dequeue(b);
}

void
update_banners(void *fb, int ticks)
{
   int i;
   for (i = 0; i < nbnrs; i++)
      if (bnr[i].used && bnr[i].list) {
	 draw_banner(i, fb);
	 update_banner(i, fb, ticks);
      }
}

int
banner_queuelen(int banner)
{
   Banner *b = bnr + banner;
   int i = 0;
   BanLink *l = b->list;
   while (l) {
      i++;
      l = l->next;
   }
   return i;
}

// Local Variables:
// c-basic-offset: 3
// End:
