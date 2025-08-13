#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef NO_MMAP
#include <sys/mman.h>
#endif

#include "log.h"
#include "safeio.h"
#include "safem.h"


const void *safe_mmap(const char *name,int fd,unsigned int offset,size_t len) {
#ifndef NO_MMAP
   caddr_t p=mmap(NULL,len,PROT_READ,MAP_SHARED,fd,offset);
   if(p==(caddr_t)-1) {
      if (name) 
	 logprintf(LOG_FATAL,'S', "Error %d on mmap of %lu bytes for %s",
		   errno, (unsigned long)len, name);
      else
	 logprintf(LOG_FATAL,'S', "Error %d on mmap of %lu bytes",
		   errno, (unsigned long)len);
   }
   return p;
#else
   void *p=safe_malloc(len);
   size_t l;
   if(name==NULL) name="???";
   if(lseek(fd,offset,SEEK_SET)==-1)
     logprintf(LOG_ERROR,'S',
	       "Bad offset (%d) in map attempt for %s (errno=%d)",
	       offset,name,errno);
   if((l=read(fd,p,len))<len)
     logprintf(LOG_ERROR,'S',
	       "Bad length (%lu) in map attempt for %s (errno=%d) (got %lu)",
	       (unsigned long)len,name,errno,(unsigned long)l);
   return p;
#endif
}

void safe_munmap(const char *name,const void *ptr,size_t len) {
#ifndef NO_MMAP
   if(munmap((caddr_t)ptr,len)==-1) 
      logprintf(LOG_FATAL,'S',"Error %d munmapping %s",
		errno, name?name:"?\?\?");
#else
   safe_free((void *)ptr);
#endif
}

void safe_read(const char *name,int fd,void *buf,size_t len) {
   int r=read(fd,buf,len);
   if(r != len) 
     logprintf(LOG_FATAL,'S',"Error %d reading %s, expecting %lu, got %d",
	       errno,name?name:"???",(unsigned long)len,r);
}

int safe_open(const char *fname,int omode,int fail_lvl) {
   int fd=open(fname,omode);
   if(fd==-1&&fail_lvl!=-1) 
     logprintf(fail_lvl,'S',"Error %d opening %s",errno,fname);
   /*else
     logprintf(LOG_DEBUG,'S',"Successfully opened %s",fname);*/
   return fd;
}
void safe_close(const char *name,int fd) {
   if(close(fd))
     logprintf(LOG_ERROR,'S',"Error %d closing %s",errno,name?name:"???");
}

// Local Variables:
// c-basic-offset: 3
// End:
