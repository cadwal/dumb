/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbwad/wadio.h: Loading WADs and looking up lumps.
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

#ifndef WADIO_H
#define WADIO_H

#include "libdumbutil/log.h"	/* LOG_FATAL used in #define load_lump() */

/* I grepped for "8" and replaced it with this where it seemed
   appropriate.  But there are still null-terminated buffers which
   have some other length like char[10].  Those should be changed to
   [LUMPNAMELEN+1], but be careful not to break anything.

   Also... we might some day check where this number is mandated by
   Doom WADs, and make those use DOOM_LUMPNAMELEN or some such.  Then
   we could increase the number elsewhere and see what happens.
       - 1999-04-04 Kalle Niemitalo <tosi@stekt.oulu.fi> */
#define LUMPNAMELEN 8

void reset_wad(void);
void init_iwad(const char *fname, const char *const *path);
void init_pwad(const char *fname, const char *const *path);

void init_wadhashing(void);
void reset_wadhashing(void);

typedef unsigned int LumpNum;

#define LUMP_WADNUM(x) (((LumpNum)x)>>16)
#define LUMP_DIRNUM(x) (((LumpNum)x)&0xffff)
#define LUMPNUM(w,d) ( (((LumpNum)w)<<16) | (d&0xffff) )

#define BAD_LUMPNUM ((LumpNum)0xffffffff)
#define LUMPNUM_OK(l) (l!=BAD_LUMPNUM)

LumpNum lumpnext(LumpNum l, int crosswad);
LumpNum lumplook(LumpNum l, const char *name);
int count_lumps_between(const char *after, const char *before);

LumpNum safe_lookup_lump(const char *name, const char *after, const char *before, int lvl);
#define lookup_lump(x,y,z) safe_lookup_lump(x,y,z,-1)
#define getlump(n) safe_lookup_lump(n,NULL,NULL,LOG_FATAL)
#define have_lump(n) LUMPNUM_OK(lookup_lump(n,NULL,NULL))

int get_lump_fd(LumpNum ln);
unsigned int get_lump_ofs(LumpNum ln);
unsigned int get_lump_len(LumpNum ln);
char *get_lump_name(LumpNum ln, char buf[LUMPNAMELEN+1]);

/* Return the filename of the WAD containing LN.  The pointer isn't
   guaranteed to stay valid for long. */
const char *get_lump_filename(LumpNum ln);

/* If LN is mapped along with the WAD, return its address.  If this
   returns NULL, the lump will have to be read with conventional
   methods.  */
const void *get_lump_map(LumpNum ln);

int get_num_wads(void);
int get_num_lumps(unsigned int wadnum);

const void *load_lump(LumpNum ln);
void *copy_lump(LumpNum ln);
void release_lump(LumpNum ln);
void free_lump(LumpNum ln);
/* free_lump is for when we don't think we'll be wanting this one again */
void free_all_lumps(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
