/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/keymapconf.h: Interface between keymap.c and confeng.c.
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

#ifndef KEYMAPCONF_H
#define KEYMAPCONF_H

#include "libdumbutil/confdef.h"

extern ConfItem keymapconf[];

void keymapconf_after_load(void);
void keymapconf_before_save(void);

#endif /* KEYMAPCONF_H */

// Local Variables:
// c-basic-offset: 3
// End:
