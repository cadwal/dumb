/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/draw.c: Drawing on the framebuffer without scaling.
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

#include <string.h>
#include <wchar.h>

#include "libdumbutil/utf8.h"
#include "draw.h"

static int fb_height = 200, fb_width = 320, fb_bpp = 1, real_width = 320;

/*** drawtext ***/

void
draw_utf8_text(void *fb, const Font *font, const char *text, size_t textlen,
	       int x, int y)
{
   utf8_mbstate_t state;
   memset(&state, 0, sizeof(state));
   for (;;) {
      wchar_t wc;
      int descent;
      Texture *tex;
      size_t mblen = utf8_mbrtowc(&wc, text, textlen, &state);
      if (mblen == (size_t) -2	/* incomplete multibyte char */
	  || mblen == (size_t) -1 /* invalid multibyte char (errno=EILSEQ) */
	  || mblen == 0)	/* successful end */
	 break;
      tex = font_wchar_tex(font, wc, &descent);
      if (tex != NULL) {
	 load_texels(tex, fb_bpp);
	 draw(fb, tex, x, y+descent);
	 x += tex->width;
      } else
	 x += font_space_width(font);
      x += font_separation(font);
      text += mblen;
      textlen -= mblen;
   }
}

void
draw_wc_text(void *fb, const Font *font, const wchar_t *text, size_t textlen,
	     int x, int y)
{
   size_t i;
   for (i = 0; i < textlen; i++) {
      int descent;
      Texture *tex = font_wchar_tex(font, text[i], &descent);
      if (tex != NULL) {
	 load_texels(tex, fb_bpp);
	 draw(fb, tex, x, y+descent);
	 x += tex->width;
      } else
	 x += font_space_width(font);
      x += font_separation(font);
   }
}

void
draw_utf8_str(void *fb, const Font *font, const char *str,
	      int x, int y)
{
   draw_utf8_text(fb, font, str, strlen(str), x, y);
}

/*** draw ***/

void
init_draw(int w, int h, int b, int r)
{
   fb_height = h;
   fb_width = w;
   fb_bpp = b;
   real_width = r;
}

#define DRAWSPEC(drawfunc,Pixel,T)					\
static inline void							\
drawfunc(void *fb, Texture *t, int xo, int yo)				\
{									\
   int x, y, dx, dy, ix=0, iy=0;					\
   dx = t->width;							\
   dy = t->height;							\
   if (fb_width-xo < dx) dx = fb_width-xo;				\
   if (fb_height-yo < dy) dy = fb_height-yo;				\
   if (xo < 0) { ix=-xo; }						\
   if (yo < 0) { iy=-yo; yo=0; }					\
   for (x = ix; x < dx; x++) {						\
      const Pixel *tpix = (const Pixel *) t->texels;			\
      Pixel *fpix = (Pixel *) fb;					\
      tpix += ((t->width - 1 - x) << t->log2height) + t->height;	\
      fpix += x + xo + yo*real_width;					\
      for (y = iy; y < dy; y++) {					\
	 tpix--;							\
	 if (*tpix != -1)						\
	    *fpix = T;							\
	 fpix += real_width;						\
      }									\
   }									\
}

DRAWSPEC(draw1, signed char, *tpix)
DRAWSPEC(draw2, short, *tpix)
DRAWSPEC(draw4, int, *tpix)
DRAWSPEC(drawoutl1, signed char, 0)
DRAWSPEC(drawoutl2, short, 0)
DRAWSPEC(drawoutl4, int, 0)

void
draw(void *fb, Texture *t, int xo, int yo)
{
   load_texels(t, fb_bpp);
   switch (fb_bpp) {
   case (1):
      draw1(fb, t, xo, yo);
      break;
   case (2):
      draw2(fb, t, xo, yo);
      break;
   case (4):
      draw4(fb, t, xo, yo);
      break;
   }
}
void
draw_outline(void *fb, Texture *t, int xo, int yo)
{
   load_texels(t, fb_bpp);
   switch (fb_bpp) {
   case (1):
      drawoutl1(fb, t, xo, yo);
      break;
   case (2):
      drawoutl2(fb, t, xo, yo);
      break;
   case (4):
      drawoutl4(fb, t, xo, yo);
      break;
   }
}

void
draw_center(void *fb, Texture *t)
{
   int x = (fb_width - t->width) / 2, y = (fb_height - t->height) / 2;
   draw(fb, t, x, y);
}

// Local Variables:
// c-basic-offset: 3
// End:
