#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_MMAP
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif

#include "log.h"
#include "safem.h"

#ifdef HAVE_MMAP

/* In Solaris, this flag means that the allocation should succeed even
 * if there actually isn't that much memory.  */
#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif

/* For some reason, Linux doesn't define MAP_FAILED even though the
 * man page for mmap(2) mentions it?  */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#define DEV_ZERO "/dev/zero"

void *
safe_vcalloc(size_t size)
{
   void *p;
#ifdef MAP_ANONYMOUS
   /* this should get us an all-zero area */
   p = mmap(NULL, size, PROT_READ|PROT_WRITE,
	    MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
   if (p==MAP_FAILED)
      logfatal('S',"mmapping %lu zero bytes: %s",
	       (unsigned long) size, strerror(errno));
#else  /* !MAP_ANONYMOUS */
   static int fd = -1;
   if (fd<0) {
      fd = open(DEV_ZERO, O_RDONLY);
      if (fd<0)
	 logfatal('S', "opening %s: %s.", DEV_ZERO, strerror(errno));
   }
   p = mmap(NULL, size, PROT_READ|PROT_WRITE,
	    MAP_PRIVATE | MAP_NORESERVE, fd, 0);
   if (p==MAP_FAILED)
      logfatal('S',"mmapping %lu bytes from %s: %s",
	       (unsigned long) size, DEV_ZERO, strerror(errno));
#endif /* !MAP_ANONYMOUS */
   return p;
}

void
safe_vfree(void *ptr, size_t size)
{
   if(munmap(ptr,size))
      logfatal('S',"munmapping %lu bytes @ %p: %s",
	       (unsigned long) size, ptr,
	       strerror(errno));
}
#endif /* HAVE_MMAP */

void *
safe_realloc(void *p1,size_t l)
{
   void *p=realloc(p1,l);
   if (p==NULL)
      logfatal('S',"reallocating %lu bytes (old=%p): %s",
	       (unsigned long) l, p1, strerror(errno));
   return p;
}

void *
safe_malloc(size_t l)
{
   void *p=malloc(l);
   if (p==NULL)
      logfatal('S',"allocating %lu bytes: %s", (unsigned long) l,
	       strerror(errno));
   return p;
}

void *
safe_calloc(size_t l, size_t c)
{
   void *p=calloc(l, c);
   if (p==NULL)
      logfatal('S',"allocating %lu bytes: %s", (unsigned long) l * c,
	       strerror(errno));
   return p;
}

char *
safe_strdup(const char *s)
{
   char *p=strdup(s);
   if (p==NULL)
      logfatal('S',"allocating %lu bytes: %s", (unsigned long) strlen(s) + 1,
	       strerror(errno));
   return p;
}

void
safe_free(void *p)
{
   /* FIXME: free(NULL) should be a no-op */
   if (p==NULL)
      logprintf(LOG_ERROR,'S', "Attempt to free NULL.");
   /*else logprintf(LOG_DEBUG,'S',"Freeing memory at %p",p);*/
   /* *((char *)p)=0;*/ /* force a crash here, if p is bogus */
   free(p);
}

// Local Variables:
// c-basic-offset: 3
// End:
