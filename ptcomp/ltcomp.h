/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/ltcomp.h: LineType/SectorType compiler.
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

#ifndef LTCOMP_H
#define LTCOMP_H

#include "libdumbwad/wadwr.h"

void init_ltcomp(void);
void ltcomp(int issect);
void wrlts(WADWR *w);
void wrsts(WADWR *w);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
