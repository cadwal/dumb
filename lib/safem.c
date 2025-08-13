#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef NO_MMAP
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif

#include "log.h"
#include "safem.h"


/*#define DEV_ZERO "/dev/zero"*/

#if !defined(NO_MMAP)
void *safe_vcalloc(size_t size) {
   void *p;
#ifdef DEV_ZERO
   static int fd=-1;
   if(fd<0) {
      fd=open(DEV_ZERO,O_RDONLY);
      if(fd<0)
	 logfatal('S',"Error %d opening %s.",errno,DEV_ZERO);
   };
   p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
   if(p==NULL) 
      logfatal('S',"Error %d mmapping %lu bytes from %s.",
	       errno,(unsigned long)size,DEV_ZERO);
#else
   /* this should get us an all-zero area */
   p=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
   if(p==NULL) 
      logfatal('S',"Error %d mmapping %lu zero bytes.",
	       errno,(unsigned long)size);
#endif
   return p;
};
void safe_vfree(void *ptr,size_t size) {
   if(munmap(ptr,size))
      logfatal('S',"Error %d munmapping %lu bytes @ %lx.",
		errno,(unsigned long)size,(unsigned long)ptr);
};
#endif

void *safe_realloc(void *p1,size_t l) {
   void *p=realloc(p1,l);
   if(p==NULL) 
     logfatal('S',"Error %d reallocating %lu bytes (old=%lx).",errno,(unsigned long)l,(unsigned long)p1);
   return p;
}

void *safe_malloc(size_t l) {
   void *p=malloc(l);
   if(p==NULL) 
     logfatal('S',"Error %d allocating %lu bytes.",errno,(unsigned long)l);
   return p;
}
void *safe_calloc(size_t l,size_t c) {
   void *p=calloc(l,c);
   if(p==NULL) 
     logfatal('S',"Error %d allocating %lu bytes.",errno,(unsigned long)l*c);
   return p;
}

void safe_free(void *p) {
   if(p==NULL) logprintf(LOG_ERROR,'S',"Attempt to free NULL.");
   /*else logprintf(LOG_DEBUG,'S',"Freeing memory at %x",p);*/
   /* *((char *)p)=0;*/ /* force a crash here, if p is bogus */
   free(p);
}
