/* DUMB: A Doom-like 3D game engine.
 *
 * libmissing/hstrerror.c: hstrerror() for systems lacking it.
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <errno.h>
#include <netdb.h>
#include <string.h>		/* strerror */

#include "libdumbutil/dumb-nls.h"

/* If hstrerror() exists, this file is not compiled at all.
   So don't add #ifndef HAVE_HSTRERROR.  */

const char *
hstrerror(int err_num)
{
   switch (err_num) {
#ifdef NETDB_INTERNAL
   case NETDB_INTERNAL:
      return strerror(errno);
#endif
#ifdef NETDB_SUCCESS
   case NETDB_SUCCESS:
      return _("Success");
#endif
   case HOST_NOT_FOUND:
      return _("Host not found (Authoritative)");
   case TRY_AGAIN:
      return _("Host not found (Unauthoritative)");
   case NO_RECOVERY:
      return _("Unrecoverable error");
   case NO_ADDRESS:
      return _("Name is valid but has no address");
   default:
      return _("Unknown error");
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
