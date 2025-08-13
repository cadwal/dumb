/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/parm.h: Reading parameters of simple types.
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

#ifndef PARM_H
#define PARM_H

#include "libdumbutil/fixed.h"

void parm_str(char *buf, size_t n);
char parm_ch(void);
int parm_num(void);
double parm_dbl(void);
int parm_time(void);
int parm_speed(void);
fixed parm_arc(void);
fixed parm_arc_opt(fixed def);
void parm_msg(char *buf, size_t n);

extern fixed default_speed;

void change_time_units(void);
void change_default_speed(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
