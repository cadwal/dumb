/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/game.c: Input processing.
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

#include <config.h>

#include <stdarg.h>
#include <stdio.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "things.h"
#include "gettable.h"
#include "netplay.h"
#include "render.h"		/* for renderer_debug_flags */
#include "game.h"

/* game message stuff */

/* this will eventually get moved here from dumb.c */
void gmsg(int pl, const char *s);

void
game_vmessage(int pl, const char *fmt, va_list argl)
{
   char buf[256]; /* better not ever print out a message bigger than this! */
   vsprintf(buf, fmt, argl);
   /*logprintf(LOG_DEBUG, 'D', _("game_message: `%s'"), buf); */
   gmsg(pl, buf);
}

void
game_message(int pl, const char *fmt,...)
{
   va_list ap;
   va_start(ap, fmt);
   game_vmessage(pl, fmt, ap);
   va_end(ap);
}


/* process player input stuff */

ConfItem playconf[] =
{
   CONFI("timer", NULL, 0, N_("processor timing overcompensation tweak"), 5),
   CONFI("forward", NULL, 0, N_("forward speed"), 18),
   CONFI("strafe", NULL, 0, N_("strafe speed"), 20),
   CONFI("turn", NULL, 0, N_("turning speed"), 100),
   CONFI("jump", NULL, 0, N_("jumping speed"), 20),
   CONFITEM_END
};
#define timer_tweak (playconf[0].intval)
#define forward_tweak (playconf[1].intval)
#define strafe_tweak (playconf[2].intval)
#define turn_tweak (playconf[3].intval)
#define jump_tweak (playconf[4].intval)

static void
process_input_ms(LevData *ld,
		 const PlayerInput *in,
		 int tickspassed, int pl)
{
   /* things that need to be done both on slave and master */
   if (in->quit)
      game_want_quit(1);
}

void
slave_input(LevData *ld, const PlayerInput *in, int tickspassed)
{
   /* this function called only on slave, so pl is always localplayer */
   process_input_ms(ld, in, tickspassed, ld->localplayer);
   send_input(ld, in, tickspassed);
   if (in->quit)
      net_bufflush();
}

void
process_input(LevData *ld, const PlayerInput *in, int tickspassed, int pl)
{
   int cheated = 0;
   /* this function called only on master */
   ThingDyn *td = ldthingd(ld) + ld->player[pl];

   /* cheats and special functions */
   if (in->select[1]) {
      cheated++;
      if (!(ld->plflags[pl] & PLF_CHEAT)) {
	 game_message(-1, _("PLAYER %d WARPED"), pl + 1);
	 game_want_newlvl(1);
      }
   }
   if (in->select[2]) {
      cheated++;
      if (!(ld->plflags[pl] & PLF_CHEAT)) {
	 game_message(-1, _("PLAYER %d CHEATED"), pl + 1);
	 cheat_gettables(ld, pl);
      }
   }
   if (in->select[3]) {
      cheated++;
      if (!(ld->plflags[pl] & PLF_CHEAT)) {
	 game_message(-1, _("PLAYER %d WIBBLED LOUDLY"), pl + 1);
	 do_tag666(ld);
      }
   }
   if (in->select[4]) {
      cheated++;
      if (!(ld->plflags[pl] & PLF_CHEAT)) {
	 game_message(-1, _("PLAYER %d RETURNED"), pl + 1);
	 thing_send_sig(ld, ld->player[pl], TS_ANIMATE);
      }
   }
   if (in->select[5]) {
      cheated++;
      if (!(ld->plflags[pl] & PLF_CHEAT)) {
	 game_message(-1, _("PLAYER %d SMASHES PUMPKINS"), pl + 1);
	 /* This toggles the flag all the time if you keep the button
	  * down... but who cares?
	  * In fact, it would be the input side's job to prevent this.
	  */
	 td->flags ^= THINGDF_NOCLIP;
      }
   }
   if (in->select[6]) {
      /* not a cheat */
      renderer_debug_flags++;
   }
   if (cheated)
      ld->plflags[pl] |= PLF_CHEAT;
   else
      ld->plflags[pl] &= ~PLF_CHEAT;

   /* only allow movement if player isn't dead */
   process_input_ms(ld, in, tickspassed, pl);
   if (td->proto == NULL || td->hits <= 0)
      return;

   /* the point of this is to discount the processor-speed independent
      timing a little.  The update code tends to overcompensate for
      fast machines a little */
   tickspassed += timer_tweak;

   /* movement */
   player_apply_lookup(ld, ld->player[pl], in->lookup / UNIT_SPEED);
   if (in->sideways)
      thingd_apply_sideways(td,
			    INT_TO_FIXED(in->sideways * tickspassed) /
			    (strafe_tweak * UNIT_SPEED));
   if (in->forward)
      thingd_apply_forward(td,
			   INT_TO_FIXED(in->forward * tickspassed) /
			   (forward_tweak * UNIT_SPEED));
   if (in->rotate)
      thingd_apply_torque(td,
			  INT_TO_FIXED(in->rotate * tickspassed) /
			  (turn_tweak * UNIT_SPEED));
   if (in->jump)
      thingd_apply_up(td, INT_TO_FIXED(in->jump * tickspassed) /
		      (jump_tweak * UNIT_SPEED));

   /* action */
   if (in->action)
      thing_activate(ld, ld->player[pl], 3 * FIXED_ONE);
   if (in->shoot)
      use_selection(0, ld, pl);

   /* selection */
   if (in->w_sel && !(ld->plflags[pl] & PLF_WSEL))
      rotate_selection(ld, pl, 0, in->w_sel > 0 ? +1 : -1);
   if (in->s_sel && !(ld->plflags[pl] & PLF_SSEL))
      rotate_selection(ld, pl, 1, in->w_sel > 0 ? +1 : -1);

   if (in->w_sel)
      ld->plflags[pl] |= PLF_WSEL;
   else
      ld->plflags[pl] &= ~PLF_WSEL;
   if (in->s_sel)
      ld->plflags[pl] |= PLF_SSEL;
   else
      ld->plflags[pl] &= ~PLF_SSEL;

}

// Local Variables:
// c-basic-offset: 3
// End:
