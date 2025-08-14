/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/msgdom.h: MessageDomain handling via MSGDOM lump.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef DUMB_MSGDOM_H
#define DUMB_MSGDOM_H

void init_msgdom(void);
void reset_msgdom(void);
const char *get_msgdom(void);

#endif /* DUMB_MSGDOM_H */

// Local Variables:
// c-basic-offset: 3
// End:
