/* DUMB: A Doom-like 3D game engine.
 *
 * tool/pngtodumb.c: Convert PNG-format images to DUMB formats.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

/* FIXME: more error checking */

#include <config.h>

#include <assert.h>		/* assert() */
#include <stdio.h>		/* fprintf() */
#include <stdlib.h>		/* exit() */
#include <png.h>		/* guess why */

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/exitcode.h"
#include "todumb.h"

const char converter_name[] = "pngtodumb";

void
print_converter_usage(const char *argv0)
{
   printf(_("Usage: %s [OPTION]... <INPUT.png >OUTPUT.lump\n"
	    "Converts graphic files from the Portable Network Graphics (PNG) format\n"
	    "to DUMB's internal formats.\n"
	    "\n"), argv0);
}

Image *
read_image(FILE *f)
{
   Image *image;
   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 width, height;
   int bit_depth, color_type;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				    NULL, NULL, NULL);
   if (png_ptr == NULL) {
      fprintf(stderr, _("PNG library initialisation failed\n"));
      exit(DUMB_EXIT_OTHER);
   }
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);
      fprintf(stderr, _("PNG library initialisation failed\n"));
      exit(DUMB_EXIT_OTHER);
   }
   if (setjmp(png_ptr->jmpbuf) != 0) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
      /* libpng has printed an error message */
      exit(DUMB_EXIT_OTHER);
   }

   png_init_io(png_ptr, f);
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
		&color_type, NULL, NULL, NULL);

   /* We want to convert everything to 8-bit RGBA.  */
   if (bit_depth == 16)
      png_set_strip_16(png_ptr);
   if (bit_depth < 8)
      png_set_packing(png_ptr);
   if (color_type == PNG_COLOR_TYPE_PALETTE
       || (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
       || png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_expand(png_ptr);
   if ((color_type & PNG_COLOR_MASK_ALPHA) == 0
       && !png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER); /* add alpha */

   png_read_update_info(png_ptr, info_ptr);
   assert(png_get_channels(png_ptr, info_ptr) == 4);
   assert(png_get_bit_depth(png_ptr, info_ptr) == 8);
   assert(png_get_rowbytes(png_ptr, info_ptr) == width * sizeof(RGBA));

   image = new_image(width, height);
   png_read_image(png_ptr, (png_bytepp) image->rows);
   
   png_read_end(png_ptr, info_ptr);
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);

   return image;
}

// Local Variables:
// c-basic-offset: 3
// End:
