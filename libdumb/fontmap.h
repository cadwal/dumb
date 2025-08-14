/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/fontmap.h: Mapping font usage codes to lump names.
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

#ifndef LIBDUMB_FONTMAP_H
#define LIBDUMB_FONTMAP_H

#include "libdumb/font.h"	/* Font */
#include "dumb/dumbdefs.pt"	/* FONTMAP_xxx */

typedef struct FontMap FontMap;
extern FontMap *game_fontmap;

FontMap *new_fontmap(void);

/* pointer will be valid until the mapping is changed */
const char *fontmapping(const FontMap *, unsigned mapping_id);

void set_fontmapping(FontMap *, unsigned mapping_id, const char *fontname);

/* Return NUM such that font_mapping(fm, id)==NULL for every ID>=NUM.  */
unsigned fontmap_length(const FontMap *fm);

Font *load_mapped_font(const FontMap *, unsigned mapping_id);

#endif /* LIBDUMB_FONTMAP_H */

// Local Variables:
// c-basic-offset: 3
// End:
