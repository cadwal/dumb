// DUMB: A Doom-like 3D game engine.
//
// libdumbwad/wadchain.h: Reference-counted chain of WADs.
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

#ifndef LIBDUMBWAD_WADCHAIN_H
#define LIBDUMBWAD_WADCHAIN_H

#include <stdio.h>		// off_t
#include "libdumbwad/wadstruct.h" // LUMPNAMELEN

class Wad;
class Counted_Wad_ptr;
class Counted_Lump_ptr;

// Counted_Wad_ptr is like Wad* but it maintains the reference count.
class Counted_Wad_ptr
{
public:
   Counted_Wad_ptr(Wad *wad = NULL);
   Counted_Wad_ptr(const Counted_Wad_ptr &);
   ~Counted_Wad_ptr();
   Counted_Wad_ptr &operator =(const Counted_Wad_ptr &);

   operator Wad *(void) const
      {
	 return wad;
      }
   Wad *operator ->(void) const
      {
	 return wad;
      }

private:
   Wad *wad;
};

// In the future, we might want to support virtual WADs or other
// storage formats.  But currently we don't.
class Wad
{
public:
   Wad(const char *fname, const char *const *path,
       Counted_Wad_ptr next_wad = NULL);
   ~Wad();
   Counted_Lump_ptr find_lump_between(const char *name,
				      const char *after, const char *before,
				      int err_level);

private:			// for class Counted_Wad_ptr
   friend class Counted_Wad_ptr;
   void add_ref(void);
   void del_ref(void);

private:			// for class Wad::Raw_Lump_ptr
   class Raw_Lump_ptr;
   friend class Raw_Lump_ptr;
   // egcs-2.91.66 bug: parameters must be named in the first friend
   // declaration.  This has been fixed in gcc-2.95.
   friend bool operator <(const Raw_Lump_ptr &a,
			  const Raw_Lump_ptr &b);
   int get_depth() const
      {
	 return depth;
      }
   typedef unsigned int Lump_index;
   void add_lump_ref(Lump_index lump_index);
   void del_lump_ref(Lump_index lump_index);

private:			// internal
   Counted_Wad_ptr next_wad;
   int depth;			// <= 0, needed for Raw_Lump_ptr comparison
   unsigned refcnt;
   char *filename;

   class Lump;
   Lump *lumps;
   Lump_index nlumps;
   const void *whole_map;	// !=NULL if the entire WAD has been mapped.
   size_t wadlen;
   char *fname;
   
   class Hash_bucket;
   class Hash_entry;
   static const unsigned hashsize = 401;
   static unsigned hashfunc(const char *s);
   Hash_bucket *hash;
   void build_hash(void);

   Raw_Lump_ptr find_lump_between(const char *name,
				  Raw_Lump_ptr after_ptr,
				  Raw_Lump_ptr before_ptr);
   Raw_Lump_ptr hash_find_lump_between(const char *name,
				       Raw_Lump_ptr after_ptr,
				       Raw_Lump_ptr before_ptr);
   static bool lump_name_matches(const char *lumpname, const char *pattern);
   static Raw_Lump_ptr next_lump(const Raw_Lump_ptr &);

private:			// forbidden
   Wad(const Wad &);
   Wad &operator =(const Wad &);
};


// These inline functions can't be defined before class Wad.

inline
Counted_Wad_ptr::Counted_Wad_ptr(Wad *wad_param):
   wad(wad_param)
{
   if (wad)
      wad->add_ref();
}

inline
Counted_Wad_ptr::Counted_Wad_ptr(const Counted_Wad_ptr &src):
   wad(src.wad)
{
   if (wad)
      wad->add_ref();
}

inline
Counted_Wad_ptr::~Counted_Wad_ptr()
{
   if (wad)
      wad->del_ref();
}

inline Counted_Wad_ptr &
Counted_Wad_ptr::operator =(const Counted_Wad_ptr &src)
{
   if (wad)
      wad->del_ref();
   wad = src.wad;
   if (wad)
      wad->add_ref();
   return *this;
}


class Wad::Raw_Lump_ptr
{
public:
   Wad *wad;
   Lump_index lump_index;

   bool valid(void) const
      {
	 return (wad != NULL);
      }

   friend bool operator ==(const Raw_Lump_ptr &a,
			   const Raw_Lump_ptr &b)
      {
	 return (a.wad == b.wad && a.lump_index == b.lump_index);
      }

   friend bool operator <(const Raw_Lump_ptr &a,
			  const Raw_Lump_ptr &b)
      {
	 assert(a.valid());
	 assert(b.valid());
	 if (a.wad->depth != b.wad->depth)
	    return (a.wad->depth < b.wad->depth);
	 else
	    return (a.lump_index < b.lump_index);
      }

   void add_ref_if_valid(void) const
      {
	 if (wad) {
	    wad->add_ref();
	    wad->add_lump_ref(lump_index);
	 }
      }
   void del_ref_if_valid(void) const
      {
	 if (wad) {
	    wad->del_lump_ref(lump_index);
	    wad->del_ref();
	 }
      }

   const char *lumpname(void) const;
};

// Counted_Lump_ptr is intended to replace LumpNum, which was an
// integer.  So it should be efficient to use.
//
// Wad objects keep a reference count for each lump, and may free the
// lump when the count becomes zero.  So you must always keep the
// Counted_Lump_ptr around as long as you are using the lump.
//
// Even if the reference count is nonzero, the lump is not necessarily
// loaded until you call Counted_Lump_ptr::addr.
class Counted_Lump_ptr
{
private:
   Wad::Raw_Lump_ptr raw;

public:
   Counted_Lump_ptr(void): raw()
      {
      }
   Counted_Lump_ptr(Wad::Raw_Lump_ptr src_raw): raw(src_raw)
      {
	 raw.add_ref_if_valid();
      }
   Counted_Lump_ptr(const Counted_Lump_ptr &src): raw(src.raw)
      {
	 raw.add_ref_if_valid();
      }
   ~Counted_Lump_ptr(void)
      {
	 raw.del_ref_if_valid();
      }
   Counted_Lump_ptr &operator =(const Counted_Lump_ptr &src)
      {
	 if (&src != this) {
	    raw.del_ref_if_valid();
	    raw = src.raw;
	    raw.add_ref_if_valid();
	 }
	 return *this;
      }

   bool valid(void) const
      {
	 return raw.valid();
      }
   friend bool operator ==(const Counted_Lump_ptr &a,
			   const Counted_Lump_ptr &b)
      {
	 return (a.raw == b.raw);
      }
   friend bool operator !=(const Counted_Lump_ptr &a,
			   const Counted_Lump_ptr &b)
      {
	 return !(a==b);
      }

   const void *addr(void) const;
   size_t size(void) const;
   off_t offset(void) const;
   const char *lumpname(void) const; // freed together with Wad
   const char *filename(void) const;
};

#endif // LIBDUMBWAD_WADCHAIN_H

// Local Variables:
// mode: c++
// c-basic-offset: 3
// End:
