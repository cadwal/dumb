/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/safeio.c: mmap() emulation, file operations and path search.
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

#include "libdumbutil/dumb-nls.h"

#include "log.h"
#include "safeio.h"
#include "safem.h"

static int fname_has_directory(const char *fname);

#ifdef HAVE_MMAP

const void *
safe_mmap(const char *name, int fd, unsigned int offset, size_t len)
{
   caddr_t p = (caddr_t) mmap(NULL, len, PROT_READ, MAP_SHARED, fd, offset);
   if (p == (caddr_t) -1) {
      if (name)
	 logprintf(LOG_FATAL, 'S', _("%s: mmap %lu bytes: %s"),
		   name, (unsigned long) len, strerror(errno));
      else
	 logprintf(LOG_FATAL, 'S', _("mmap %lu bytes: %s"),
		   (unsigned long) len, strerror(errno));
   }
   return p;
}

void
safe_munmap(const char *name, const void *ptr, size_t len)
{
   if (munmap((caddr_t) ptr, len) == -1)
      logprintf(LOG_FATAL, 'S', _("%s: munmapping %lu bytes: %s"),
		name ? name : "?\?\?",
		(unsigned long) len, strerror(errno));
}

#else  /* !HAVE_MMAP */

const void *
safe_mmap(const char *name, int fd, unsigned int offset, size_t len)
{
   void *p = safe_malloc(len);
   size_t l;
   if (name == NULL)
      name = "?\?\?";
   if (lseek(fd, offset, SEEK_SET) == -1)
      logprintf(LOG_ERROR, 'S', _("%s: seeking to %u: %s"),
		name, offset, strerror(errno));
   /* FIXME: read() might return -1 */
   /* FIXME: interrupted system call */
   if ((l = read(fd, p, len)) < len)
      logprintf(LOG_ERROR, 'S', _("%s: short read (%lu<%lu): %s"),
		name, (unsigned long) l, (unsigned long) len,
		strerror(errno));
   return p;
}

void
safe_munmap(const char *name, const void *ptr, size_t len)
{
   safe_free((void *) ptr);
}

#endif /* !HAVE_MMAP */

void
safe_read(const char *name, int fd, void *buf, size_t len)
{
   if (!name)
      name = _("<unknown file>");
   /* Signals may interrupt read(), so call it as many times as necessary.  */
   while (len > 0) {
      ssize_t got = read(fd, buf, len);
      if (got == -1) {
	 if (errno != EINTR)
	    logprintf(LOG_FATAL, 'S', "%s: %s", name, strerror(errno));
      } else if (got == 0) {
	 logprintf(LOG_FATAL, 'S', _("%s: unexpected EOF"), name);
      } else {
	 /* It may not be necessary to cast GOT to size_t,
	    but it doesn't hurt. */
	 len -= (size_t) got;
	 buf = (void *) ((char *) buf + (size_t) got);
      }
   }
}

int
safe_open(const char *fname, int omode, int fail_lvl)
{
   /* If the file is created (omode includes O_CREAT), give everyone
    * read and write permission if allowed by umask.  */
   int fd = open(fname, omode,
	      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
   if (fd == -1 && fail_lvl != -1)
      logprintf(fail_lvl, 'S', _("%s: opening: %s"),
		fname, strerror(errno));
   /*else
      logprintf(LOG_DEBUG, 'S', _("Successfully opened %s"),fname); */
   return fd;
}

void
safe_close(const char *name, int fd)
{
   if (close(fd))
      logprintf(LOG_ERROR, 'S', _("%s: closing: %s"),
		name ? name : "?\?\?",
		strerror(errno));
}

static int
fname_has_directory(const char *fname)
{
   if (strchr(fname, '/'))
      return 1;
#ifdef __MSDOS__
   /* Actually, the colon is valid in fname[1] only.  */
   if (strpbrk(fname, "\\:"))
      return 1;
#endif /* __MSDOS__ */
   return 0;
}

int
safe_open_path(const char *fname, int omode, int fail_lvl,
	       const char *const *path, char **realname)
{
   static const char *const default_path[] = {".", NULL};
   if (fname_has_directory(fname)) {	/* don't search path */
      int ret = safe_open(fname, omode, fail_lvl);
      if (realname && ret != -1)
	 *realname = safe_strdup(fname);
      return ret;
   }
   if (!path || path[0] == NULL)
      path = default_path;
   for (; *path; path++) {
      const char *dir = *path;
      size_t dirlen = strlen(dir);
      /* Variable-length automatic arrays are a GCC extension.  */
      char tryname[dirlen + 1 + strlen(fname) + 1];
      int ret;
      if (!dirlen || dir[dirlen - 1] == '/')
	 sprintf(tryname, "%s%s", dir, fname);
      else
	 sprintf(tryname, "%s/%s", dir, fname);
      /* Don't ever give up if there's another chance.  */
      ret = safe_open(tryname, omode,
		      path[1] ? -1 : fail_lvl);
      if (ret != -1) {
	 if (realname)
	    *realname = safe_strdup(tryname);
	 return ret;
      }
   }
   /* We tried every directory in the path but couldn't find the file,
    * and fail_lvl wasn't LOG_FATAL.  */
   return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
