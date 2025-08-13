/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/bugaddr.c: Where to report bugs.
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

#include <config.h>		/* may define const */

#include "libdumbutil/dumb-nls.h"

#include "bugaddr.h"

/* This can be changed by each program.  */
const char *program_bug_address = "dumb-list@schlick.anu.edu.au";

void
print_bugaddr_message(FILE *dest)
{
   fprintf(dest, _("Report bugs to <%s>.\n"), program_bug_address);
}

// Local Variables:
// c-basic-offset: 3
// End:
