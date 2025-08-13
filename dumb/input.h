/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/input.h: Prototypes for functions to be defined in each input driver.
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

#ifndef INPUT_H
#define INPUT_H

#include "libdumbutil/confdef.h"
#include "libdumbutil/endiantypes.h"

extern ConfItem input_conf[];

/* The PlayerInput structure contains a series of fields each of which
 * (rougly) corresponds to a command key.  The fields are integers so that
 * the input module can report the intensity with which the command was
 * issued.  This comes in handy for mice and joysticks.  Fields like
 * rotate, forward, sideways, and jump permit negative values.
 */

#define UNIT_SPEED 16

typedef struct {
   LE_int32 forward;
   LE_int32 rotate;
   LE_int32 sideways;
   LE_int32 lookup;
   LE_int32 jump;
   char shoot;
   char action;
   char quit;
   char w_sel;
   char s_sel;
   char use;
   char dummy1, dummy2;
   char select[16];
} PlayerInput;

void get_input(PlayerInput *in);

void init_input(void);
void reset_input(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
