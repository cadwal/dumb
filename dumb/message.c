/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/message.c: Shoving messages to players.
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbwad/wadio.h"
#include "libdumb/fontmap.h"
#include "banner.h"
#include "netplay.h"

static int message_banner = -1;
static Font *message_font = NULL;
static int waffle_banner = -1;

static int screen_width, screen_height;
static LevData *ld;
static int running_as_slave;


/* Initialisation */

void
init_message(int scr_width, int scr_height, int crowd_flag,
	     LevData *ld_parm, int slave)
{
   screen_width = scr_width;
   screen_height = scr_height;
   ld = ld_parm;
   running_as_slave = slave;

   message_banner = init_banner(screen_height / 3, 0, screen_width, 10);
   message_font = load_mapped_font(game_fontmap, FONTMAP_MESSAGE);
   if (crowd_flag && have_lump("DUMBLOGO"))
      add_to_banner(message_banner, get_misc_texture("DUMBLOGO"),
		    screen_width, 64);

   waffle_banner = init_banner(screen_height - 12, 0, screen_width, 3);
   if (crowd_flag && have_lump("WAFFLE")) {
      LumpNum waffle_ln = getlump("WAFFLE");
      /* Someone wrote that Digital Unix needs the following (but Linux doesn't):

	 if (waffle_ln == 0)
	   goto skip;

	 However, 0 is a valid lump number and means the first lump
	 in the first WAD!  Some other way must be found.  */
      add_to_banner(waffle_banner, NULL, screen_width, 0);
      add_utf8_text_to_banner(waffle_banner, message_font,
			      (const char *) load_lump(waffle_ln),
			      get_lump_len(waffle_ln));
      free_lump(waffle_ln);
   }
}

/* this doesn't really belong here, but it needs message_font */
Texture *
get_xhair_texture(void)
{
   if (message_font == NULL)
      return NULL;
   else
      return font_wchar_tex(message_font, L'+', NULL);
}


/* Showing messages */

static void
show_local_utf8_message(const char *str)
{
   if (message_font == NULL) {
      /* FIXME: translate to local charset */
      logprintf(LOG_INFO, 'D', _("fontless message: %s"), str);
      return;
   }
   add_to_banner(message_banner, NULL, screen_width, 0);
   add_utf8_str_to_banner(message_banner, message_font, str);
}

static void
dispatch_utf8_message(int pl, const char *s)
{
   if (pl == ld->localplayer)
      show_local_utf8_message(s);
   else {
      if (pl < 0 && !running_as_slave)
	 show_local_utf8_message(s);
      send_message(pl, s);
   }
}

void
game_utf8_vmessage(int pl, const char *fmt, va_list argl)
{
   char buf[256]; /* FIXME: arbitrary limit, and possible buffer overrun */
   vsprintf(buf, fmt, argl);
   /* logprintf(LOG_DEBUG, 'D', _("game_message: `%s'"), buf); */
   dispatch_utf8_message(pl, buf);
}

void
game_utf8_message(int pl, const char *fmt,...)
{
   va_list ap;
   va_start(ap, fmt);
   game_utf8_vmessage(pl, fmt, ap);
   va_end(ap);
}

// Local Variables:
// c-basic-offset: 3
// End:
