/* ctlkey_input.c: enum ctlkey and conversion from it to PlayerInput.
 *
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
 * 02139, USA.
 */

/* I changed x11_input.c to use enum ctlkey as an intermediate step
 * between X keysyms and DUMB input states.  The mechanism seemed
 * generic enough to deserve a file of its own.  Here it is.
 *
 * TO DO: Mouse/joystick support.  Normal mice & joystics have two
 * axes.  Allow the user to assign a meaning to each.  The normal case
 * would be x=sideways and y=forward but he could change it.  At the
 * same time, allow digital thresholds.  For example, shoot when
 * pulling the joystick down.  Separate functions for mouse/joystick
 * button single-clicks and double-clicks.  Like in Doom where middle
 * button strafes when held down and opens doors when double-clicked.
 */

#include <string.h>		/* memset() */
#include "plat/ctlkey_input.h"
#include "lib/fixed.h"
#include "lib/log.h"

/* Define these to separate looking from aiming */
/* #define COMPLEX_LOOKING */
/* #define MAX_LOOK_ANGLE 64 */

static unsigned char keystate[CTLKEY_ARRAY_SIZE];
static PlayerInput input;
static int need_recalc;

static void recalc_input(void);
static void compress_square_to_circle(fixed *x, fixed *y);

void 
ctlkey_init(void)
{
   memset(&keystate, 0, sizeof (keystate));
   /* memset(&input, 0, sizeof (input)); recalced anyway */
   need_recalc = 1;
}

void
ctlkey_reset(void)
{
}

void
ctlkey_press(enum ctlkey key, int pressed_flag)
{
   if (key == CTLKEY_NONE)
      return; /* no error */
   if (keystate[key] == pressed_flag)
      return;
   keystate[key] = pressed_flag;
   need_recalc = 1;
#ifdef COMPLEX_LOOKING   
   if (key == CTLKEY_CENTER_VIEW && pressed_flag)
      input.lookup = 0;
   if ((key == CTLKEY_LOOK_UP || key == CTLKEY_LOOK_DOWN) && !pressed_flag)
      input.lookup = 0;
#endif
}

void
ctlkey_calc_tick(void)
{
#ifdef COMPLEX_LOOKING
   int aim_speed = ((keystate[CTLKEY_AIM_UP] 
		     || keystate[CTLKEY_LOOK_UP])
		    - (keystate[CTLKEY_AIM_DOWN] 
		       || keystate[CTLKEY_LOOK_DOWN])) * 3;
   if (keystate[CTLKEY_RUN])
      aim_speed *= 3;
   input.lookup += aim_speed;
   if (input.lookup < -MAX_LOOK_ANGLE)
      input.lookup = -MAX_LOOK_ANGLE;
   else if (input.lookup > MAX_LOOK_ANGLE)
      input.lookup = MAX_LOOK_ANGLE;
#endif
}

void 
ctlkey_get_player_input(PlayerInput *dest)
{
   if (need_recalc) {
      recalc_input();
      need_recalc = 0;
   }
   *dest = input;
}

static void
recalc_input(void)
{
   int left, right, i;
   fixed x, y;
   input.quit = keystate[CTLKEY_QUIT];
   input.jump = (keystate[CTLKEY_MOVE_UP] 
		 - keystate[CTLKEY_MOVE_DOWN]) * UNIT_SPEED;
   input.action = keystate[CTLKEY_ACTIVATE];
   input.shoot = keystate[CTLKEY_SHOOT];
   input.w_sel = (keystate[CTLKEY_NEXT_WEAPON] 
		  - keystate[CTLKEY_PREVIOUS_WEAPON]);
   input.use = keystate[CTLKEY_USE_ITEM];
   input.s_sel = (keystate[CTLKEY_NEXT_ITEM]
		  - keystate[CTLKEY_PREVIOUS_ITEM]);
   left = (keystate[CTLKEY_MOVE_LEFT]
	   || (keystate[CTLKEY_STRAFE] 
	       && keystate[CTLKEY_TURN_LEFT]));
   right = (keystate[CTLKEY_MOVE_RIGHT]
	    || (keystate[CTLKEY_STRAFE] 
		&& keystate[CTLKEY_TURN_RIGHT]));
   x = INT_TO_FIXED(left - right);
   if (keystate[CTLKEY_STRAFE])
      input.rotate = 0;
   else
      input.rotate = (keystate[CTLKEY_TURN_LEFT]
		      - keystate[CTLKEY_TURN_RIGHT]) * UNIT_SPEED;
   y = INT_TO_FIXED(keystate[CTLKEY_MOVE_FORWARD]
		    - keystate[CTLKEY_MOVE_BACKWARD]);
#ifdef COMPLEX_LOOKING
   /* input.lookup is handled in ctlkey_calc_tick() */
#else
   input.lookup = (((keystate[CTLKEY_LOOK_UP] 
		     || keystate[CTLKEY_AIM_UP])
		    - (keystate[CTLKEY_LOOK_DOWN] 
		       || keystate[CTLKEY_AIM_DOWN]))
		   * UNIT_SPEED);
#endif   
   if (keystate[CTLKEY_RUN]) {
      x = FIXED_SCALE(x, 2);
      y = FIXED_SCALE(y, 2);
      input.rotate *= 2;
   }
   compress_square_to_circle(&x, &y);
#if UNIT_SPEED == INT_TO_FIXED(1)
   /* PlayerInput uses regular fixed-point */
   input.sideways = x;
   input.forward = y;
#else
   /* PlayerInput has its own scaling */
   input.sideways = FIXED_TO_FLOAT(x)*UNIT_SPEED;
   input.forward = FIXED_TO_FLOAT(y)*UNIT_SPEED;
#endif
   for (i = 0; i <= 9; i++)
      input.select[i] = keystate[CTLKEY_WEAPON_0 + i];
}

static void
compress_square_to_circle(fixed *x, fixed *y)
{
   /* +-------*******-------+ (C,C)
    * |    ***       **     |
    * |  **             **  |
    * | *          (x',y')*_+ (x,y)
    * |*                _-+*|
    * |*             _--   *|
    * *           _--       *
    * *          -          *
    * *        (0,0)        *
    * |*                   *|
    * |*                   *|
    * | *                 * |
    * |  **             **  |
    * |    ***       ***    |
    * +-------*******-------+ (C,-C)
    *
    * Old coordinates: x, y
    * New coordinates: x', y'
    * Must find scale factor k such that x'=k*x and y'=k*y.
    * x=0 or y=0 ==> k=1 ==> x'=x and y'=y.
    * Original distance: r = sqrt(x^2+y^2)
    * New distance: r' = sqrt(x'^2+y'^2)
    * Original comparison distance in that direction:
    *   if abs(y) < abs(x) < C:
    *     R = sqrt(C^2 + (y*C/x)^2)
    *       = sqrt((x*C/x)^2 + (y*C/x)^2)
    *       = sqrt((C/x)^2 * (x^2 + y^2))
    *       = abs(C/x) * sqrt(x^2+y^2)
    *       = abs(C/x) * r
    *       = r * C / abs(x)
    *   if abs(x) < abs(y) < C:
    *     R = r * C / abs(y)
    *   so: R = r * C / max(abs(x), abs(y))
    * New comparison distance in that direction:
    *   R' = C
    *   R' = k*R
    *   ==> k = R'/R = C/R = max(abs(x), abs(y)) / r
    * Now, if x=0 then k = r / max(0, abs(y)) = abs(y)/abs(y) = 1, great.
    */
   fixed maxabs, r, k;
   if (*x == FIXED_ZERO || *y == FIXED_ZERO)
      return;
   maxabs = MAX(FIXED_ABS(*x), FIXED_ABS(*y));
   r = fix_pythagoras(*x, *y);
   k = fixdiv(maxabs, r);
   *x = fixmul(*x, k);
   *y = fixmul(*y, k);
}

/* Ctlkey naming support */

struct ctlkey_namepair {
   const char *ugly;
   const char *pretty;
};

static const struct ctlkey_namepair ctlkey_names[CTLKEY_ARRAY_SIZE] = {
   { "quit", "Quit" },
   { "move_forward", "Move forward" },
   { "move_backward", "Move backward" },
   { "turn_left", "Turn left" },
   { "turn_right", "Turn right" },
   { "turn_180", "Turn around" },
   { "move_left", "Sidestep left" },
   { "move_right", "Sidestep right" },
   { "move_up", "Jump / Swim/Fly up" },
   { "move_down", "Duck / Swim/Fly down" },
   { "look_up", "Look up" },
   { "look_down", "Look down" },
   { "aim_up", "Aim up" },
   { "aim_down", "Aim down" },
   { "center_view", "Center view" },
   { "run", "Run" },
   { "strafe", "Strafe" },
   { "activate", "Activate" },
   { "shoot", "Shoot" },
   { "shoot_special", "Shoot special" },
   { "next_weapon", "Select next weapon" },
   { "previous_weapon", "Select previous weapon" },
   { "weapon_0", "Select weapon 0" },
   { "weapon_1", "Select weapon 1" },
   { "weapon_2", "Select weapon 2" },
   { "weapon_3", "Select weapon 3" },
   { "weapon_4", "Select weapon 4" },
   { "weapon_5", "Select weapon 5" },
   { "weapon_6", "Select weapon 6" },
   { "weapon_7", "Select weapon 7" },
   { "weapon_8", "Select weapon 8" },
   { "weapon_9", "Select weapon 9" },
   { "use_item", "Use item" },
   { "next_item", "Next item" },
   { "previous_item", "Previous item" }
};

const char *ctlkey_ugly_name(enum ctlkey key)
{
   return ctlkey_names[key].ugly;
}

const char *ctlkey_pretty_name(enum ctlkey key)
{
   return ctlkey_names[key].pretty;
}

// Local Variables:
// c-basic-offset: 3
// End:
