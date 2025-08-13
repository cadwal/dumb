/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/linux_input.c: Linux SVGAlib input driver.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1994 by Chris Laurel
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

#include <config.h>

/* Undefine this to get the keys back to what they were before I
 * messed with them.  - Kalle */
#define KALLE_KEYS 1

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vgakeyboard.h>
#include <vgamouse.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "input.h"

static const ConfEnum mousies[] =
{
   {"microsoft", MOUSE_MICROSOFT},
   {"mousesystems", MOUSE_MOUSESYSTEMS},
   {"mmseries", MOUSE_MMSERIES},
   {"logitech", MOUSE_LOGITECH},
   {"busmouse", MOUSE_BUSMOUSE},
   {"ps2", MOUSE_PS2},
   {"logiman", MOUSE_LOGIMAN},
   CONFENUM_END
};

ConfItem input_conf[] =
{
   CONFB("no-mouse", NULL, 0, N_("disable mouse support")),
   CONFE("mouse-type", NULL, 0, N_("mouse type"), 0, mousies),
   CONFS("mouse-dev", NULL, 0, N_("mouse device"), "/dev/mouse", 254),
   CONFITEM_END
};
#define cnf_no_mouse (input_conf[0].intval)
#define cnf_mouse_type (mousies[input_conf[1].intval].value)
#define cnf_mouse_dev (input_conf[2].strval)

static int mouse_dx = 0, mouse_dy = 0, mouse_buts = 0, last_mx = 0,
 last_my = 0;

static void
update_dumb_mouse(void)
{
   int mouse_x, mouse_y;

   mouse_update();

   mouse_x = mouse_getx();
   mouse_y = mouse_gety();
   mouse_buts = mouse_getbutton();

   mouse_dx = -(mouse_x - last_mx);
   mouse_dy = -(mouse_y - last_my);

   last_mx = mouse_x;
   last_my = mouse_y;

   /* normalise dx & dy when wrapping occurs */

   if (mouse_dx < -15000)
      mouse_dx += 30000;
   if (mouse_dx > 15000)
      mouse_dx -= 30000;
   if (mouse_dy < -15000)
      mouse_dy += 30000;
   if (mouse_dy > 15000)
      mouse_dy -= 30000;
}

#define k_k keyboard_keypressed
#define NK(x) if(keyboard_keypressed(SCANCODE_##x)) in->select[x]=1

void
get_input(PlayerInput *in)
{
   int speed = UNIT_SPEED;

   keyboard_update();
   if (!cnf_no_mouse)
      update_dumb_mouse();
   memset(in, 0, sizeof(PlayerInput));

   /* shift */
   if (k_k(SCANCODE_LEFTSHIFT) || k_k(SCANCODE_RIGHTSHIFT))
      speed = 2 * UNIT_SPEED;

   /* movement */
   if (k_k(SCANCODE_RIGHTALT) || k_k(SCANCODE_LEFTALT)) {
      if (k_k(SCANCODE_CURSORLEFT) || k_k(SCANCODE_CURSORBLOCKLEFT))
	 in->sideways += speed;
      if (k_k(SCANCODE_CURSORRIGHT) || k_k(SCANCODE_CURSORBLOCKRIGHT))
	 in->sideways -= speed;
      in->sideways += mouse_dx * speed / 30;
   } else {
      if (k_k(SCANCODE_CURSORLEFT) || k_k(SCANCODE_CURSORBLOCKLEFT))
	 in->rotate += speed;
      if (k_k(SCANCODE_CURSORRIGHT) || k_k(SCANCODE_CURSORBLOCKRIGHT))
	 in->rotate -= speed;
      in->rotate += mouse_dx * speed / 40;
   }
#ifdef KALLE_KEYS
   if (k_k(SCANCODE_KEYPAD1) || k_k(SCANCODE_COMMA))
      in->sideways += speed;
   if (k_k(SCANCODE_KEYPAD3) || k_k(SCANCODE_PERIOD))
      in->sideways -= speed;
   if (k_k(SCANCODE_KEYPAD9) || k_k(SCANCODE_PAGEUP) || k_k(SCANCODE_D))
      in->lookup += UNIT_SPEED;
   if (k_k(SCANCODE_KEYPAD7) || k_k(SCANCODE_PAGEDOWN) || k_k(SCANCODE_C))
      in->lookup -= UNIT_SPEED;
#else  /* !KALLE_KEYS */
   if (k_k(SCANCODE_KEYPAD9) || k_k(SCANCODE_PAGEUP) || k_k(SCANCODE_A))
      in->lookup += UNIT_SPEED;
   if (k_k(SCANCODE_KEYPAD3) || k_k(SCANCODE_PAGEDOWN) || k_k(SCANCODE_Z))
      in->lookup -= UNIT_SPEED;
#endif /* !KALLE_KEYS */

   if (k_k(SCANCODE_CURSORUP) || k_k(SCANCODE_CURSORBLOCKUP)
       || (mouse_buts & MOUSE_RIGHTBUTTON))
      in->forward += speed;
   if (k_k(SCANCODE_CURSORDOWN) || k_k(SCANCODE_CURSORBLOCKDOWN))
      in->forward -= speed;
   in->forward += mouse_dy * speed / 30;

   /* bang bang */
#ifdef KALLE_KEYS
   if (k_k(SCANCODE_Q))
      in->w_sel -= 1;
   if (k_k(SCANCODE_W))
      in->w_sel += 1;
   if ((mouse_buts & MOUSE_LEFTBUTTON)
       || k_k(SCANCODE_LEFTCONTROL) || k_k(SCANCODE_RIGHTCONTROL))
      in->shoot = 1;
   if (k_k(SCANCODE_A))
      in->jump += UNIT_SPEED;
   if (k_k(SCANCODE_Z))
      in->jump -= UNIT_SPEED;
#else  /* !KALLE_KEYS */
   if (k_k(SCANCODE_TAB))
      in->w_sel = 1;
   if (k_k(SCANCODE_ENTER) || (mouse_buts & MOUSE_LEFTBUTTON)
       || k_k(SCANCODE_LEFTCONTROL) || k_k(SCANCODE_RIGHTCONTROL))
      in->shoot = 1;
   if (k_k(SCANCODE_HOME) || k_k(SCANCODE_KEYPAD7))
      in->jump = UNIT_SPEED;
#endif
   if (k_k(SCANCODE_SPACE))
      in->action = 1;
   if (k_k(SCANCODE_ESCAPE))
      in->quit = 1;
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

#undef k_k
#undef NK

void
init_input(void)
{
   if (keyboard_init())
      logprintf(LOG_FATAL, 'I', _("Console open failed"));
   else
      logprintf(LOG_INFO, 'I', _("Console in raw mode"));

   if (!cnf_no_mouse) {
      mouse_init(cnf_mouse_dev, cnf_mouse_type, MOUSE_DEFAULTSAMPLERATE);
      mouse_setxrange(0, 30000);
      mouse_setyrange(0, 30000);
      mouse_setwrap(MOUSE_WRAP);
      mouse_setscale(1);
      mouse_setposition(0, 0);
      last_mx = last_my = 0;
   }
}

void
reset_input(void)
{
   keyboard_close();
}

// Local Variables:
// c-basic-offset: 3
// End:
