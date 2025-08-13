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

/* Require a string parameter and save it in BUF which is N characters
   long.  Because of the null terminator, the string can have at most
   N-1 characters.  */
void parm_str(char *buf, size_t n);

/* Require a string parameter of any length and return it in a buffer
   which must free()d when no longer needed.  A null terminator is
   automatically added.  */
char *parm_strdup(void);

/* Require a name parameter of any length and return it in a static
   buffer.  If the name isn't there, print ERRMSG and exit.  */
const char *parm_name(const char *errmsg);

/* If the next token is KEYWORD, eat it and return true.  Otherwise,
   return false.  */
int parm_keyword_opt(const char *keyword);

char parm_ch(void);
int parm_num(void);
double parm_dbl(void);
int parm_time(void);
int parm_speed(void);
fixed parm_arc(void);
fixed parm_arc_opt(fixed def);

extern fixed default_speed;

void change_time_units(void);
void change_default_speed(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
