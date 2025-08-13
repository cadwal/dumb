/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/linetype.h: Linetypes.
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LINETYPE_H
#define LINETYPE_H

#include "libdumb/linetypestruct.h"
#include "levdata.h"

void init_linetypes(void);
void reset_linetypes(void);

const LineType *lookup_linetype(int id);
const SectorType *lookup_sectortype(int id);

fixed get_term_type(const LevData *ld, LT_TermType ltt, int sector);

void linetype_ticks_passed(int tickspassed);

struct LineTypeOffsets {
   int xofs, yofs;
};
extern struct LineTypeOffsets *ltofs;

/* this is actually in thinghit.c, any suggestions on a better .h for it? */
void perform_lta(LevData *ld, const LT_Action *lta, int on, int targ);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
