// DUMB: A Doom-like 3D game engine.
//
// libdumbwad/wadchain-wrapper.cc: Wadio emulation via WadNode.
// Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111, USA.

#include "wadio.h"
#include "wadchain.hh"

static WadPtr wads = NULL;
static unsigned num_wads = 0;

void
reset_wad(void)
{
   delete wads;
   wads = NULL;
   num_wads = 0;
}

void
init_iwad(const char *fname, const char *const *path)
{
   wads = new WadNode(fname, path, wads);
   num_wads++;
}

void
init_pwad(const char *fname, const char *const *path)
{
   init_iwad(fname, path);
}

void
init_wadhashing(void)
{
   // automatic
}

void
reset_wadhashing(void)
{
   // automatic
}

LumpNum
lumpnext(LumpNum l, int crosswad)
{
   FIXME;
}

LumpNum
lumplook(LumpNum l, const char *name)
{
   FIXME;
}

int
count_lumps_between(const char *after, const char *before)
{
   FIXME;
}

LumpNum
safe_lookup_lump(const char *name, const char *after, const char *before, int lvl)
{
   LumpRef lump;
   if (wads)
      lump = wads->lookup_lump(name, after, before);
   if (!lump)
      logprintf(lvl, 'W', _("Can't find lump %s"), name);
   return lump;
}

unsigned int
get_lump_ofs(LumpNum ln)
{
   return ln.offset();
}

unsigned int
get_lump_len(LumpNum ln)
{
   return ln.length();
}

char *
get_lump_name(LumpNum ln, char buf[LUMPNAMELEN+1])
{
   char *name = ln.lumpname();
   strncpy(buf, name, LUMPNAMELEN);
   buf[LUMPNAMELEN] = '\0';
   free(name);
   return buf;
}

const char *
get_lump_filename(LumpNum ln)
{
   FIXME;			// when is the name freed?
}

int
get_num_wads(void)
{
   return num_wads;
}

int
get_num_lumps(unsigned int wadnum)
{
   FIXME;			// "unsigned int" isn't the right type
}

const void *
load_lump(LumpNum ln)
{
   return ln.addr();		// FIXME: this may free the lump too early
}

void *
copy_lump(LumpNum ln)
{
   size_t size = ln.size();
   void *mem = safe_malloc(size);
   memcpy(mem, ln.addr(), size);
   return mem;
}

void *
release_lump(LumpNum ln)
{
   // automatic
}

void *
free_lump(LumpNum ln)
{
   // automatic
}

void *
free_all_lumps(void)
{
   // automatic
}

// Local Variables:
// c-basic-offset: 3
// End:
