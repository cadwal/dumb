/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbwad/wadwr.h: Writing WAD files.
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

#ifndef WADWR_H
#define WADWR_H

#include <stdio.h>

#include "libdumbwad/wadstruct.h"

typedef struct {
   FILE *f;
   WadHeader hdr;
   WadDirEntry *dir, *current;
   unsigned maxdir;
   char *fname;
   char type;
   int error_flag;
} WADWR;

WADWR *wadwr_open(const char *fname, char wadtype);
int wadwr_close(WADWR *w);	/* returns 0 if ok */

void wadwr_lump(WADWR *w, const char *lumpname);
void wadwr_write(WADWR *w, const void *lump, size_t len);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
