/* DUMB: A Doom-like 3D game engine.
 *
 * tool/ppmtodumb.c: Convert ppm-format images to DUMB formats.
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
# include DUMB_CONFIG_PPM_H
}
#else
# include DUMB_CONFIG_PPM_H
#endif

#include "libdumbutil/dumb-nls.h"

#include "todumb.h"

const char converter_name[] = "ppmtodumb";

void
print_converter_usage(const char *argv0)
{
   printf(_("Usage: %s [OPTION]... <INPUT.ppm >OUTPUT.lump\n"
	    "Converts graphic files from the portable pixmap format (PPM) to DUMB's\n"
	    "internal formats.\n"
	    "\n"), argv0);
}

Image *
read_image(FILE *f)
{
   int width, height, x, y;
   pixval maxval;
   pixel **pix;
   Image *image;
   pixel transparent;
   pix = ppm_readppm(f, &width, &height, &maxval);
   if (pix == NULL)
      exit_invalid_data(_("bad ppm"));
   PPM_ASSIGN(transparent, 0,0,1);
   image = new_image(width, height);
   for (y = 0; y < height; y++) {
      const pixel *src = pix[y];
      RGBA *dest = image->rows[y];
      for (x = 0; x < width; x++, src++, dest++) {
	 dest->r = (PPM_GETR(*src) * 255UL) / maxval;
	 dest->g = (PPM_GETG(*src) * 255UL) / maxval;
	 dest->b = (PPM_GETB(*src) * 255UL) / maxval;
	 dest->a = PPM_EQUAL(*src, transparent) ? 0 : 255;
	 /* Alpha channel will be ignored unless enabled in command line.  */
      }
   }
   ppm_freearray(pix, height);
   return image;
}

// Local Variables:
// c-basic-offset: 3
// End:
