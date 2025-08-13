/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/confeng.h: The configuration engine.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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

#ifndef CONFENG_H
#define CONFENG_H

#include "libdumbutil/confdef.h"

void set_conf(ConfItem *ci, char *val, int dirt);
void conf_clear_list(ConfItem *ci);

int conf_greatest_dirtlevel(const ConfModule conf[]);

ConfItem *conf_lookup_longname(const ConfModule conf[], const char *s);
ConfItem *conf_lookup_shortname(const ConfModule conf[], char ch);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
