// DUMB: A Doom-like 3D game engine.
//
// libdumbwad/wadchain.cc: Reference-counted chain of WADs.
// Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
// Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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

#include <config.h>

#include <assert.h>
#include <ctype.h>		// toupper
#include <stdlib.h>		// free
#include <string.h>		// strchr

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "wadchain.hh"

#define FIXME


class Wad::Hash_entry
{
public:
   Hash_entry *next;
   // The Wad containing this hash already has a reference to the Wad
   // containing the lump.
   Raw_Lump_ptr lump;
};

class Wad::Hash_bucket
{
public:
   Hash_entry *first, **endp;

   Hash_bucket(void): first(NULL), endp(&first)
      {
      }

   ~Hash_bucket(void)
      {
	 while (first) {
	    Hash_entry *next = first->next;
	    delete first;
	    first = next;
	 }
	 // endp is invalid here; doesn't matter
      }

   void add(Wad *wad, Lump_index ln)
      {
	 Hash_entry *entry = new Hash_entry;
	 entry->next = NULL;
	 entry->lump.wad = wad;
	 entry->lump.lump_index = ln;
	 // Add it to the end of the chain.  If the chain was empty,
	 // endp == &first.
	 *endp = entry;
	 endp = &entry->next;
      }

private:			// forbidden
   Hash_bucket(const Hash_bucket &);
   Hash_bucket &operator =(const Hash_bucket &);
};

class Wad::Lump
{
public:
   unsigned refcnt;
   off_t offset;
   size_t size;
   char *name;
   const void *data;
   // No destructor here; its job is done by Wad::~Wad() which can
   // check whole_map without a Wad pointer in Lump.
};

const char *
Wad::Raw_Lump_ptr::lumpname(void) const
{
   assert(valid());
   assert(lump_index < wad->nlumps);
   return wad->lumps[lump_index].name;
}


Wad::Wad(const char *fname, const char *const *path,
	 Counted_Wad_ptr next_param):
   next_wad(next_param), depth(next_wad ? next_wad->depth-1 : 0), refcnt(0),
   filename(NULL),
   lumps(NULL), nlumps(0), hash(NULL)
{
   FIXME;
}
 
Wad::~Wad()
{
   delete[] hash;
   for (Lump_index i = 0; i < nlumps; i++) {
      assert(lumps[i].refcnt == 0);
      free(lumps[i].name);
      if (!whole_map)
	 free(const_cast<void *>(lumps[i].data));
   }
   delete[] lumps;
}

Counted_Lump_ptr
Wad::find_lump_between(const char *name, const char *after, const char *before,
		       int err_level)
{
   Raw_Lump_ptr after_ptr, before_ptr;
   if (after) {
      after_ptr = find_lump_between(after, Raw_Lump_ptr(), Raw_Lump_ptr());
      if (!after_ptr.valid()) {
	 logprintf(err_level, 'W',
		   _("Can't find lump marker %s (lump %s)"),
		   after, name);
	 return Counted_Lump_ptr();
      }
   }
   do {
      if (before) {
	 before_ptr = find_lump_between(before, after_ptr, Raw_Lump_ptr());
	 if (!before_ptr.valid()) {
	    logprintf(err_level, 'W',
		      _("Can't find end-marker %s"), before);
	    return Counted_Lump_ptr();
	 }
      }
      Raw_Lump_ptr sought_ptr = find_lump_between(name, after_ptr, before_ptr);
      if (sought_ptr.valid())
	 return sought_ptr;
      // Try to find another AFTER-BEFORE pair.
   } while (after && before
	    && (after_ptr = find_lump_between(after,
					      before_ptr, Raw_Lump_ptr())
		).valid());
   if (after)
      logprintf(err_level, 'W', _("Can't find lump %s:%s"),
		after, name);
   else
      logprintf(err_level, 'W', _("Can't find lump %s"), name);
   return Counted_Lump_ptr();
}

void
Wad::add_ref(void)
{
   ++refcnt;
   assert(refcnt > 0);		// overflow check
}

void
Wad::del_ref(void)
{
   assert(refcnt > 0);
   if (--refcnt == 0)
      delete this;
}

void
Wad::add_lump_ref(Wad::Lump_index lump_index)
{
   assert(lump_index < nlumps);
   ++lumps[lump_index].refcnt;
   assert(lumps[lump_index].refcnt > 0);
}

void
Wad::del_lump_ref(Wad::Lump_index lump_index)
{
   assert(lump_index < nlumps);
   assert(lumps[lump_index].refcnt > 0);
   if (--lumps[lump_index].refcnt == 0) {
      if (!whole_map)
	 free(const_cast<void *>(lumps[lump_index].data));
      // else lumps[lumpindex].data points to the mmapped area.
   }
}

unsigned
Wad::hashfunc(const char *s)
{
   // TODO: Use some algorithm which doesn't require the fixed-length
   // garble string.
   const char *end = s + LUMPNAMELEN;
   const char *garble = "garble1^";
   unsigned int sum = 0;
   while (*s && s < end)
      sum += *garble++ * toupper(*s++);
   return sum % hashsize;
}

void
Wad::build_hash(void)
{
   assert(!hash);
   hash = new Hash_bucket[hashsize];
   // No need to use Counted_Wad_ptr here, since this Wad already has
   // a reference to the next one, etc.
   for (Wad *wad = this; wad; wad = wad->next_wad) {
      for (Lump_index i = 0; i < wad->nlumps; i++) {
	 hash[hashfunc(wad->lumps[i].name)].add(wad, i);
      }
   }
}

Wad::Raw_Lump_ptr
Wad::find_lump_between(const char *name,
		       Raw_Lump_ptr after_ptr, Raw_Lump_ptr before_ptr)
{
   if (!strchr(name, '?'))
      return hash_find_lump_between(name, after_ptr, before_ptr);
   while (after_ptr.valid() && after_ptr != before_ptr) {
      if (lump_name_matches(after_ptr.lumpname(), name))
	 return after_ptr;
      after_ptr = next_lump(after_ptr);
   }
   return Raw_Lump_ptr();
}

Wad::Raw_Lump_ptr
Wad::hash_find_lump_between(const char *name,
			    Raw_Lump_ptr after_ptr,
			    Raw_Lump_ptr before_ptr)
{
   if (!hash)
      build_hash();
   Hash_entry *entry = hash[hashfunc(name)].first;
   // The hash entries are in ascending order.  Take advantage of that.
   if (after_ptr.valid()) {
      // Skip all entries which are before AFTER_PTR.
      while (entry && entry->lump < after_ptr)
	 entry = entry->next;
   }
   for (;;) {
      if (!entry || (before_ptr.valid() && !(entry->lump < before_ptr)))
	 return Raw_Lump_ptr();
      if (!strcmp(entry->lump.lumpname(), name))
	 return entry->lump;
      entry = entry->next;
   }
}

bool
Wad::lump_name_matches(const char *lumpname, const char *pattern)
{
   for (; *lumpname != '\0'; ++lumpname, ++pattern) {
      // This catches the pattern ending first.
      if (*pattern != '?' && toupper(*lumpname) != toupper(*pattern))
	 return false;
   }
   // End of lumpname.  Did the pattern end too?
   return (*pattern == '\0');
}

// Local Variables:
// c-basic-offset: 3
// End:
