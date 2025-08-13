/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/pastepic.h: Pasting patches in textures.
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

/* this file included in texture.c */

/* ppot, Pixel, and pixcvt should be defined in parent file */

static void
jpot(Texture *t, const AltPictData *pict, int xo, int yo)
{
   Pixel *out = (Pixel *) t->texels;
   int x, y;
   int dx = pict->width, dy = pict->height;
   for (x = 0; x < dx; x++)
      for (y = 0; y < dy; y++) {
	 int xt = x + (t->width) - pict->width - xo;
	 int yt = y + (t->height) - pict->height - yo;
	 unsigned char tpix;
	 if (xt < 0 || yt < 0 || xt >= t->width || yt >= t->height)
	    continue;
	 tpix = pict->data[(x << pict->log2height) + y];
	 if (tpix == 0xff)
	    continue;
	 out[(xt << t->log2height) + yt] = pixcvt(tpix);
      }
}

static void
ppot(Texture *t, const PictData *pict, int xo, int yo)
{
   int col, row, endrow;
   for (col = 0; col < pict->UMEMB(hdr).width; col++) {
      const unsigned char *pd = pict->data + pict->UMEMB(hdr).idx[col];
      Pixel *out = (Pixel *) t->texels;
      out += (t->width - (xo + col + 1)) << t->log2height;
      out += t->height - 1;
      while (*pd != 0xff) {
	 row = yo + (*pd++);
	 endrow = row + (*pd++);
	 pd++;			/* dummy byte */
	 while (row < endrow) {
	    if (row >= 0
		&& row < t->height
		&& (xo + col) >= 0
		&& (xo + col) < t->width)
	       out[-row] = pixcvt(*pd);
	    row++;
	    pd++;
	 }
	 pd++;			/* dummy byte */
      }
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
