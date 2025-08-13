/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/levinfo.h: LEVINFO lump handling.
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

#ifndef LEVINFO_H
#define LEVINFO_H

#include "libdumb/levinfostruct.h"
#include "levdata.h"

void init_levinfo(void);
void reset_levinfo(void);

void levinfo_startgame(LevData *ld, int episode, int difficulty, int mplayer);
void levinfo_next(LevData *ld, int secret);

const LevInfo *find_levinfo(LevData *ld);

const char *get_skyname(LevData *ld);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
