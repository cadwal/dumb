/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/copyright.h: Printing copyright info
 * Copyright (C) 1998 by Kalle Olavi Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LIBDUMBUTIL_COPYRIGHT_H
#define LIBDUMBUTIL_COPYRIGHT_H

struct copyright
{
   const char *years;
   const char *holder;
};

#define COPYRIGHT_END { NULL, NULL }

void print_copyright(const struct copyright *info);
void print_copyrights(const struct copyright info[]);

#endif /* LIBDUMBUTIL_COPYRIGHT_H */

// Local Variables:
// c-basic-offset: 3
// End:
