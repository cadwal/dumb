/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/dos_input.c: MS-DOS input driver.
 * Copyright (C) 1998 by Ulf Axelsson <ulf@ore.ims.se>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include <libintl.h>
#define _(str) gettext(str)
#define N_(str) gettext_noop(str)

#include "libdumbutil/log.h"
#include "input.h"

ConfItem input_conf[] =
{
   CONFB("no-mouse", NULL, 0, N_("disable mouse support")),
   /* FIXME: It shouldn't be necessary to put default values in the string.  */
   CONFI("mx-sens", NULL, 0, N_("mouse sensitivity x-axis (4)"), 4),
   CONFI("my-sens", NULL, 0, N_("mouse sensitivity y-axis (2)"), 2),
   {NULL}
};

#define cnf_no_mouse (input_conf[0].intval)
#define cnf_mx_sens (input_conf[1].intval)
#define cnf_my_sens (input_conf[2].intval)

static int mouse_dx = 0, mouse_dy = 0;

static void
update_dumb_mouse(void)
{
   get_mouse_mickeys(&mouse_dx, &mouse_dy);
   mouse_dx = -mouse_dx;
   mouse_dy = -mouse_dy;
}


#define NK(x) if (key[KEY_##x]) in->select[x] = 1

void
get_input(PlayerInput *in)
{
   int speed = UNIT_SPEED;

   if (!cnf_no_mouse)
      update_dumb_mouse();

   memset(in, 0, sizeof(PlayerInput));

   if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
      speed += UNIT_SPEED;

   /* movement */
   if (key[KEY_ALT] || key[KEY_ALTGR]) {
      if (key[KEY_LEFT])
	 in->sideways += speed;
      if (key[KEY_RIGHT])
	 in->sideways -= speed;
      in->sideways += mouse_dx * speed / 60;
   } else {
      if (key[KEY_LEFT])
	 in->rotate += speed;
      if (key[KEY_RIGHT])
	 in->rotate -= speed;
      in->rotate += mouse_dx * speed / 60;
   }

   if (key[KEY_PGUP] || key[KEY_A])
      in->lookup += UNIT_SPEED;
   if (key[KEY_PGDN] || key[KEY_Z])
      in->lookup -= UNIT_SPEED;

   if (key[KEY_UP] || (mouse_b & 2))
      in->forward += speed;

   if (key[KEY_DOWN])
      in->forward -= speed;

   in->forward += mouse_dy * speed / 60;

   /* bang bang */
   if (key[KEY_TAB])
      in->w_sel = 1;

   if (key[KEY_ENTER] || (mouse_b & 1) ||
       key[KEY_LCONTROL] || key[KEY_RCONTROL])
      in->shoot = 1;

   if (key[KEY_HOME] || key[KEY_5_PAD])
      in->jump = UNIT_SPEED;

   if (key[KEY_SPACE])
      in->action = 1;

   if (key[KEY_ESC])
      in->quit = 1;

   /* Weapon selection */
   NK(0);
   NK(1);
   NK(2);
   NK(3);
   NK(4);
   NK(5);
   NK(6);
   NK(7);
   NK(8);
   NK(9);
}

#undef NK

void
init_input(void)
{
   install_keyboard();

   if (!cnf_no_mouse) {
      if (install_mouse() < 0) {
	 logprintf(LOG_ERROR, 'I',
		   _("Can't initialize mouse! Missing driver?"));
	 cnf_no_mouse = TRUE;
      } else {
	 /* Not really necessary */
	 set_mouse_range(-15000, -15000, 15000, 15000);
	 position_mouse(0, 0);

	 set_mouse_speed(cnf_mx_sens, cnf_my_sens);
      }
   }
}

void
reset_input(void)
{
   if (!cnf_no_mouse)
      remove_mouse();
   remove_keyboard();
}

// Local Variables:
// c-basic-offset: 3
// End:
