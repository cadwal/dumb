/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/fontmapcomp.c: FontMap compiler.
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

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumb/fontmap.h"
#include "libdumb/fontmapinwad.h"
#include "parm.h"
#include "fontmapcomp.h"

FontMap *fontmap;

void
init_fontmapcomp(void)
{
   fontmap = new_fontmap();
}

void
fontmapcomp(void)
{
   unsigned id = parm_uint();
   char *fontname = parm_strdup();
   set_fontmapping(fontmap, id, fontname);
   safe_free(fontname);
}

void
wrfontmap(WADWR *w)
{
   printf(_("%5u font mappings\n"), fontmap_length(fontmap));
   save_fontmap(w, fontmap);
}

// Local Variables:
// c-basic-offset: 3
// End:
