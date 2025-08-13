/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/safeio.h: mmap() emulation, file operations and path search.
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

#ifndef SAFEIO_H
#define SAFEIO_H

#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* these mmap fns should only be used for *readonly* mmaps */
const void *safe_mmap(const char *name, int fd, unsigned int offset,
		      size_t len);
void safe_munmap(const char *name, const void *ptr, size_t len);

void safe_read(const char *name, int fd, void *buf, size_t len);
int safe_open(const char *fname, int omode, int fail_lvl);
void safe_close(const char *name, int fd);

/* Like safe_open() except looks in all directories in the
 * NULL-terminated array PATH.  If REALNAME is non-null, saves the
 * real filename in *REALNAME.  The caller is responsible for freeing
 * the name with free() when it's no longer needed.  If the file isn't
 * found and fail_lvl!=LOG_FATAL, returns -1 without changing
 * *REALNAME.  If FNAME contains a slash ('/') or an OS-specific
 * equivalent, doesn't use PATH at all.
 *
 * This won't work sensibly with O_CREAT.  */
int safe_open_path(const char *fname, int omode, int fail_lvl,
		   const char *const path[], char **realname);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
