/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/ctlkey.h: enum ctlkey and conversion from it to PlayerInput.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef CTLKEY_INPUT_H
#define CTLKEY_INPUT_H

#include "input.h"

enum ctlkey {
   CTLKEY_QUIT,			/* Immediately quit the game program */
   CTLKEY_MOVE_FORWARD, CTLKEY_MOVE_BACKWARD,
   CTLKEY_TURN_LEFT, CTLKEY_TURN_RIGHT,
   CTLKEY_TURN_180,		/* Not supported (yet) */
   CTLKEY_MOVE_LEFT, CTLKEY_MOVE_RIGHT,		/* Strafe */
   CTLKEY_MOVE_UP, CTLKEY_MOVE_DOWN,
   CTLKEY_LOOK_UP, CTLKEY_LOOK_DOWN,
   CTLKEY_AIM_UP, CTLKEY_AIM_DOWN, CTLKEY_CENTER_VIEW,
   CTLKEY_RUN, CTLKEY_STRAFE,	/* These affect what other keys do */
   CTLKEY_ACTIVATE,		/* For doors etc. */
   CTLKEY_SHOOT,
   CTLKEY_SHOOT_SPECIAL,	/* Not used (yet) */
   CTLKEY_NEXT_WEAPON, CTLKEY_PREVIOUS_WEAPON,
   /* CTLKEY_WEAPON_0 to CTLKEY_WEAPON_9 must be contiguous because
    * they are accessed as CTLKEY_WEAPON_0+i */
   CTLKEY_WEAPON_0, CTLKEY_WEAPON_1, CTLKEY_WEAPON_2,
   CTLKEY_WEAPON_3, CTLKEY_WEAPON_4, CTLKEY_WEAPON_5,
   CTLKEY_WEAPON_6, CTLKEY_WEAPON_7, CTLKEY_WEAPON_8,
   CTLKEY_WEAPON_9,
   /* For inventory items */
   CTLKEY_USE_ITEM, CTLKEY_NEXT_ITEM, CTLKEY_PREVIOUS_ITEM,
   CTLKEY_ARRAY_SIZE, CTLKEY_NONE = -1
};

void ctlkey_init(void);
void ctlkey_reset(void);

/* pressed_flag must be either 0 or 1 */
/* Calling this with KEY==CTLKEY_NONE is not considered an error.
 * ctlkey_press() will then just return immediately.  */
void ctlkey_press(enum ctlkey key, int pressed_flag);

void ctlkey_calc_tick(void);

void ctlkey_get_player_input(PlayerInput *dest);

const char *ctlkey_pretty_name(enum ctlkey);	/* translated */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
