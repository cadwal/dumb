/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/fontinwad.h: Loading and saving fonts.
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

#ifndef LIBDUMB_FONTINWAD_H
#define LIBDUMB_FONTINWAD_H

#include "libdumbwad/wadwr.h"
#include "libdumb/font.h"

Font *load_font(const char *lumpname);
void save_font(WADWR *, const Font *, const char *lumpname);

#endif /* LIBDUMB_FONTINWAD_H */

// Local Variables:
// c-basic-offset: 3
// End:
