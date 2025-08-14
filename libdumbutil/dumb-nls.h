/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/dumb-nls.h: _(""), U_("") and N_("") with or without ENABLE_NLS.
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef DUMB_NLS_H
#define DUMB_NLS_H

/* config.h must be included before this */

#if HAVE_LIBINTL_H
# include <libintl.h>
# define _(str) gettext(str)
# define U_(str) dgettext(PACKAGE"-utf8", str)
# define N_(str) str
#else
# define _(str) str
# define U_(str) str
# define N_(str) str
#endif

#endif

// Local Variables:
// c-basic-offset: 3
// End:
