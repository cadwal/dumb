/* Written by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
 *
 * I changed x11_input.c to use enum ctlkey as an intermediate step
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

const char *const ctlkey_name[CTLKEY_ARRAY_SIZE] = {
   "Quit",
   "Move forward", "Move backward",
   "Turn left", "Turn right",
   "Turn around",
   "Move left", "Move right",
   "Jump / Swim/Fly up", "Duck / Swim/Fly down",
   "Look up", "Look down",
   "Aim up", "Aim down", "Center view",
   "Run", "Strafe",
   "Activate",
   "Shoot",
   "Shoot special",
   "Select next weapon", "Select previous weapon",
   "Select weapon 0", "Select weapon 1", "Select weapon 2", 
   "Select weapon 3", "Select weapon 4", "Select weapon 5",
   "Select weapon 6", "Select weapon 7", "Select weapon 8",
   "Select weapon 9",
   "Use item", "Next item", "Previous item"
};

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
   int left, right, i,x,y;
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
   input.sideways = (left - right) * UNIT_SPEED;
   if (keystate[CTLKEY_STRAFE])
      input.rotate = 0;
   else
      input.rotate = (keystate[CTLKEY_TURN_LEFT]
			  - keystate[CTLKEY_TURN_RIGHT]) * UNIT_SPEED;
   input.forward = (keystate[CTLKEY_MOVE_FORWARD]
			- keystate[CTLKEY_MOVE_BACKWARD]) * UNIT_SPEED;
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
      input.sideways *= 2;
      input.rotate *= 2;
      input.forward *= 2;
   }
   for (i = 0; i <= 9; i++)
      input.select[i] = keystate[CTLKEY_WEAPON_0 + i];
   /* The variables aren't actually fixed-point but this works anyway!  */
   /* the to-ing and fro-ing with x and y is necessary on bigendian machines */
   x=input.sideways;y=input.forward;
   compress_square_to_circle(&x, &y);
   input.sideways=x;input.forward=y;
}

static void
compress_square_to_circle(fixed *x, fixed *y)
{
   /* Old coordinates: x, y
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

// Local Variables:
// c-basic-offset: 3
// End:
