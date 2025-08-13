/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/ctlkey.c: enum ctlkey and conversion from it to PlayerInput.
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

/* I changed x11_video.c to use enum ctlkey as an intermediate step
 * between X keysyms and DUMB input states.  The mechanism seemed
 * generic enough to deserve a file of its own.  Here it is.
 *
 * And now aalib_input.c uses this code too!
 *
 * TO DO: Mouse/joystick support.  Normal mice & joystics have two
 * axes.  Allow the user to assign a meaning to each.  The normal case
 * would be x=sideways and y=forward but he could change it.  At the
 * same time, allow digital thresholds.  For example, shoot when
 * pulling the joystick down.  Separate functions for mouse/joystick
 * button single-clicks and double-clicks.  Like in Doom where middle
 * button strafes when held down and opens doors when double-clicked.
 */

#include <config.h>

#include <string.h>		/* memset() */

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/fixed.h"
#include "libdumbutil/log.h"
#include "ctlkey.h"

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
   memset(&keystate, 0, sizeof(keystate));
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
      return;			/* no error */
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
   input.sideways = FIXED_TO_FLOAT(x) * UNIT_SPEED;
   input.forward = FIXED_TO_FLOAT(y) * UNIT_SPEED;
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

static const char *const ctlkey_pretty_names[CTLKEY_ARRAY_SIZE] =
{
   N_("Quit"),
   N_("Move forward"), N_("Move backward"),
   N_("Turn left"), N_("Turn right"),
   N_("Turn around"),
   N_("Sidestep left"), N_("Sidestep right"),
   N_("Jump / Swim/Fly up"), N_("Duck / Swim/Fly down"),
   N_("Look up"), N_("Look down"),
   N_("Aim up"), N_("Aim down"), N_("Center view"),
   N_("Run"), N_("Strafe"),
   N_("Activate"),
   N_("Shoot"),
   N_("Shoot special"),
   N_("Select next weapon"), N_("Select previous weapon"),
   N_("Select weapon 0"), N_("Select weapon 1"), N_("Select weapon 2"),
   N_("Select weapon 3"), N_("Select weapon 4"), N_("Select weapon 5"),
   N_("Select weapon 6"), N_("Select weapon 7"), N_("Select weapon 8"),
   N_("Select weapon 9"),
   N_("Use item"), N_("Next item"), N_("Previous item")
};

const char *
ctlkey_pretty_name(enum ctlkey key)
{
   return _(ctlkey_pretty_names[key]);
}

// Local Variables:
// c-basic-offset: 3
// End:
