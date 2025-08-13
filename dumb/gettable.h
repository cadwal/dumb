/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/gettable.c: Gettables.
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

#ifndef GETTABLE_H
#define GETTABLE_H

#include "libdumb/gettablestruct.h"
#include "levdata.h"

void init_gettables(void);
void reset_gettables(void);
void reset_local_gettables(LevData *ld);

void cheat_gettables(LevData *ld, int plnum);

int gettable_chk_key(const LevData *ld, int plnum, int keytype);

void draw_gettables(LevData *ld, int pl, void *fb, int width, int height);
void update_gettables(LevData *ld, int ticks);

void pickup_gettable(LevData *ld, int plnum, int type, int num);

void rotate_selection(LevData *ld, int plnum, int type, int dir);
void use_selection(int type, LevData *ld, int plnum);

/* return 0 if use failed */
int use_item(LevData *ld, int player, const Gettable *gt);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
