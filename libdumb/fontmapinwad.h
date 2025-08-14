/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/fontmapinwad.h: Loading and saving font maps.
 * Copyright (C) 1999 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMB_FONTMAPINWAD_H
#define LIBDUMB_FONTMAPINWAD_H

#include "libdumbwad/wadwr.h"
#include "libdumb/fontmap.h"

FontMap *load_fontmap(void);
void save_fontmap(WADWR *, const FontMap *);

#endif /* LIBDUMB_FONTMAPINWAD_H */

// Local Variables:
// c-basic-offset: 3
// End:
