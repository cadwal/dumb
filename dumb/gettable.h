/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/gettable.c: Gettables.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

enum gettable_pickup {
   GETT_PU_NOTENOUGH = -2,	/* (-) Not enough ammo */
   GETT_PU_USELESS   = -1,	/* (+) Wouldn't gain anything by taking it */
   GETT_PU_ENOUGH    =  0,	/* (-) It's a loss we can live with */
   GETT_PU_GOT       =  1,	/* (+) Had some before */
   GETT_PU_GOTFIRST  =  2	/* (+) Didn't have any before */
};
/* Return true if PU means the player picked the object up.  */
#define GETT_PU_IS_OK(pu)	((pu) >= 0)
/* Return true if PU means the player gained something. */
#define GETT_PU_IS_USEFUL(pu)	((pu) > 0)

enum gettable_pickup pickup_gettable(LevData *ld, int pl,
				     const Gets *gets,
				     int dry_run);

/* You must call this with dry_run=1 first!  Otherwise, the player may
   get gettables he doesn't deserve.  */
enum gettable_pickup pickup_gettables(LevData *ld, int pl,
				      const Gets gets[], size_t ngets,
				      int dry_run);

void rotate_selection(LevData *ld, int plnum, int type, int dir);
void use_selection(int type, LevData *ld, int plnum);

/* return 0 if use failed */
/* (this is in useitem.c) */
int use_item(LevData *ld, int player, const Gettable *gt);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
