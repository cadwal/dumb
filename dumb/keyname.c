/* DUMB: A Doom-like 3D game engine.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * keyname.c: Names of ASCII characters for keymapping.
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <ctype.h>
#include <stdio.h>

#include "keyname.h"

const char *
keyname_of_char(char ch)
{
   static char buf[10];		/* "char_-127" must fit */
   if (isgraph(ch)) {
      buf[0] = ch;
      buf[1] = '\0';
      return buf;
   }
   switch (ch) {
   case '\x08': return "BackSpace";
   case '\x09': return "Tab";
   case '\x0A': return "Linefeed";
   case '\x0D': return "Return";
   case '\x1B': return "Escape";
   case ' ': return "Space";
   case '\x7F': return "Delete";
   default:
      sprintf(buf, "char_%d", (int) ch);
      return buf;
   } /* switch (ch) */
}

  
// Local Variables:
// c-basic-offset: 3
// End:
