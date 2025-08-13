#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libdumbutil/log.h"
#include "wadwr.h"

#define ALLOCBLK 256 /* how many dirents to alloc at a time */

/*typedef struct  {
   int fout;
   WadHeader hdr;
   WadDirEntry *dir,*current;
   int maxdir;
} WADWR;*/

#define tell(fd) lseek(fd,0,SEEK_CUR)

#define sigcpy(x,y) memcpy(x,y,4)

WADWR *wadwr_open(const char *fname,char wadtype) {
   WADWR *w;
   w=(WADWR *)calloc(1,sizeof(WADWR));
   if(w==NULL) return w;
   w->fname=fname;
   w->type=tolower(wadtype);
   if(w->type=='d')
      w->f=-1;
   else {
      w->f=creat(fname,S_IREAD|S_IWRITE);
      if(w->f<0) {
	 logprintf(LOG_ERROR,'W',"wadwr: couldn't open %s, errno=%d",
		   fname,errno);
	 free(w);
	 return NULL;
      }
      if(w->type=='i') sigcpy(w->hdr.sig,"IWAD");
      else sigcpy(w->hdr.sig,"PWAD");
      w->maxdir=ALLOCBLK;
      w->dir=(WadDirEntry *)malloc(sizeof(WadDirEntry)*w->maxdir);
      write(w->f,&w->hdr,sizeof(WadHeader));
   }
   return w;
}

void wadwr_close(WADWR *w) {
   if(w->type!='d') {
      w->hdr.diroffset=tell(w->f);
      write(w->f,w->dir,sizeof(WadDirEntry)*w->hdr.nlumps);
      lseek(w->f,0,SEEK_SET);
      write(w->f,&w->hdr,sizeof(WadHeader));
   }
   if(w->f>=0) close(w->f);
   if(w->dir) free(w->dir);
   free(w);
}

void wadwr_lump(WADWR *w,const char *lumpname) {
   if(w->type=='d') {
      char buf[256];
      if(w->f>=0) close(w->f);
      strcpy(buf,w->fname);
      if(buf[strlen(buf)-1]!='/') strcat(buf,"/");
      strcat(buf,lumpname);
      if(!strstr(buf,".lump")) strcat(buf,".lump");
      w->f=creat(buf,S_IREAD|S_IWRITE);
      if(w->f<0) logprintf(LOG_ERROR,'W',"wadwr: couldn't open %s, errno=%d",
			   buf,errno);
   } else {
      if(w->hdr.nlumps>=w->maxdir)  {
	 w->maxdir+=ALLOCBLK;
	 w->dir=(WadDirEntry *)realloc(w->dir,w->maxdir*sizeof(WadDirEntry));
      }
      w->current=w->dir+w->hdr.nlumps;
      w->hdr.nlumps=w->hdr.nlumps+1;
      memset(w->current,0,sizeof(WadDirEntry));
      strncpy(w->current->name,lumpname,8);
      w->current->offset=tell(w->f);
   }
}

void wadwr_write(WADWR *w,const void *lump,size_t len) {
   if(w->type!='d') {
      if(w->current==NULL) return;
      w->current->size=w->current->size+len;
   }
   if(w->f>=0) write(w->f,lump,len);
}

