/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/message.h: Shoving messages to players.
 * Copyright (C) 1999 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef DUMB_MESSAGE_H
#define DUMB_MESSAGE_H

#include <stdarg.h>
#include "levdata.h"

/* these args should really be global variables */
void init_message(int screen_width, int screen_height, int crowd_flag,
		  LevData *ld, int slave);

/* this doesn't really belong here, but it needs message_font */
Texture *get_xhair_texture(void);

void game_vmessage(int pl, const char *fmt, va_list argl);
void game_message(int pl, const char *fmt, ...);

#endif /* DUMB_MESSAGE_H */

// Local Variables:
// c-basic-offset: 3
// End:
