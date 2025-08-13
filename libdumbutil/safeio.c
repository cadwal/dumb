#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

#include "log.h"
#include "safeio.h"
#include "safem.h"


#ifdef HAVE_MMAP

const void *
safe_mmap(const char *name, int fd, unsigned int offset, size_t len)
{
   caddr_t p=mmap(NULL,len,PROT_READ,MAP_SHARED,fd,offset);
   if(p==(caddr_t)-1) {
      if (name)
	 logprintf(LOG_FATAL,'S', "%s: mmap %lu bytes: %s",
		   name, (unsigned long) len, strerror(errno));
      else
	 logprintf(LOG_FATAL,'S', "mmap %lu bytes: %s",
		   (unsigned long) len, strerror(errno));
   }
   return p;
}

void
safe_munmap(const char *name, const void *ptr, size_t len)
{
   if(munmap((caddr_t)ptr,len)==-1)
      logprintf(LOG_FATAL,'S', "%s: munmapping %lu bytes: %s",
		name?name:"?\?\?", (unsigned long) len, strerror(errno));
}

#else  /* !HAVE_MMAP */

const void *
safe_mmap(const char *name, int fd, unsigned int offset, size_t len)
{
   void *p=safe_malloc(len);
   size_t l;
   if(name==NULL) name="?\?\?";
   if(lseek(fd,offset,SEEK_SET)==-1)
      logprintf(LOG_ERROR,'S', "%s: seeking to %d: %s",
	       name, offset, strerror(errno));
   if((l=read(fd,p,len))<len)
      logprintf(LOG_ERROR,'S', "%s: short read (%d<%d): %s",
		name, l, len, strerror(errno));
   return p;
}

void
safe_munmap(const char *name, const void *ptr, size_t len)
{
   safe_free((void *)ptr);
}

#endif /* !HAVE_MMAP */

void
safe_read(const char *name, int fd, void *buf, size_t len)
{
   int r=read(fd,buf,len);
   if(r != len)
      logprintf(LOG_FATAL,'S', "%s: short read (%d<%lu): %s",
		name?name:"?\?\?", r, (unsigned long) len, strerror(errno));
}

int
safe_open(const char *fname, int omode, int fail_lvl)
{
   /* If the file is created (omode includes O_CREAT), give everyone
    * read and write permission if allowed by umask.  */
   int fd=open(fname, omode, 
	       S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH);
   if(fd==-1&&fail_lvl!=-1)
      logprintf(fail_lvl,'S', "%s: opening: %s",
		fname, strerror(errno));
   /*else
      logprintf(LOG_DEBUG,'S',"Successfully opened %s",fname);*/
   return fd;
}

void
safe_close(const char *name, int fd)
{
   if (close(fd))
      logprintf(LOG_ERROR,'S', "%s: closing: %s", 
		name?name:"?\?\?", strerror(errno));
}

int
safe_open_path(const char *fname, int omode, int fail_lvl,
	       const char *const *path, char **realname)
{
   static const char *const default_path[] = { ".", NULL };
   if (fname[0] == '/') {	/* Absolute filename, don't search path */
#ifdef __MSDOS__
#warn "MS-DOS absolute filenames not recognized"
#endif
      int ret = safe_open(fname, omode, fail_lvl);
      if (realname && ret!=-1)
	 *realname = safe_strdup(fname);
      return ret;
   }
   if (!path || path[0]==NULL)
      path = default_path;
   for (; *path; path++) {
      const char *dir = *path;
      size_t dirlen = strlen(dir);
      char tryname[dirlen + 1 + strlen(fname) + 1];
      int ret;
      if (!dirlen || dir[dirlen-1]=='/')
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
