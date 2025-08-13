/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/keyname.h: Names of ASCII characters for keymapping.
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

#ifndef KEYNAME_H
#define KEYNAME_H

/* Return the name of the ASCII character CH.  The name returned is in
 * static storage and valid until this function is called again.  (So
 * this is not reentrant.)
 *
 * For graphic characters (isgraph(ch)), the name is the character
 * itself.  */
const char *keyname_of_char(char ch);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
