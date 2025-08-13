/* DUMB: A Doom-like 3D game engine.
 *
 * tool/dark2trans.c: Converts dark pixels to #000001 which means transparent.
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
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
# include <ppm.h>
}
#else
# include <ppm.h>
#endif

#include "libdumbutil/dumb-nls.h"

int
main(int argc, char **argv)
{
   pixel **pix;
   int cols, rows;
   pixval maxval;
   int x, y, minval;
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   if (argc != 2) {
      fprintf(stderr, _("usage: dark2trans <minimum pixel brightness>\n\n"));
      return 1;
   }
   minval = atoi(argv[1]) * 3;
   pix = ppm_readppm(stdin, &cols, &rows, &maxval);
   for (y = 0; y < rows; y++)
      for (x = 0; x < cols; x++) {
	 pixel *p = pix[y] + x;
	 if (p->r + p->g + p->b < minval) {
	    p->r = p->g = 0;
	    p->b = 1;
	 }
      }
   ppm_writeppm(stdout, pix, cols, rows, maxval, 0);
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
