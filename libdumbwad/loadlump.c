#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <limits.h>

#include "libdumbutil/safeio.h"
#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "wadstruct.h"
#include "wadio.h"

#define WDEBUG(x) logprintf(LOG_DEBUG,'W',x)

typedef struct {
   const void *data;
   size_t len;
} LumpRec;  

static LumpRec **tbl=NULL;

static void llinit(void) {
   int i,num_wads;
   if(tbl!=NULL) return;
   num_wads=get_num_wads();
   tbl=safe_calloc(num_wads,sizeof(LumpRec *));
   for(i=0;i<num_wads;i++) 
     tbl[i]=safe_calloc(get_num_lumps(i),sizeof(LumpRec));
}

const void *load_lump(LumpNum ln) {
   LumpRec *lr;
   const void *p;
   if(!LUMPNUM_OK(ln)) 
      logprintf(LOG_FATAL,'W',"Bad lumpnum in %s","load_lump");
   p=get_lump_map(ln);
   if(p!=NULL) return p;
   llinit();
   lr=tbl[LUMP_WADNUM(ln)]+LUMP_DIRNUM(ln);
   if(lr->data==NULL||lr->len==0) {
      lr->len=get_lump_len(ln);
      if(lr->data==NULL)
	lr->data=safe_mmap("a lump",get_lump_fd(ln),get_lump_ofs(ln),lr->len);
   }
   return lr->data;
}

void *copy_lump(LumpNum ln) {
   size_t l=get_lump_len(ln);
   void *p=safe_malloc(l);
   memcpy(p,load_lump(ln),l);
   return p;
}

void free_lump(LumpNum ln) {
   LumpRec *lr;
   if(!LUMPNUM_OK(ln)) 
      logprintf(LOG_FATAL,'W',"Bad lumpnum in %s","free_lump");
   if(tbl==NULL) return;
   lr=tbl[LUMP_WADNUM(ln)]+LUMP_DIRNUM(ln);
   if(lr->data) {
      safe_munmap("a lump",lr->data,lr->len);
      lr->data=NULL;
      lr->len=0;
   }
}
void release_lump(LumpNum ln) {
   LumpRec *lr;
   if(tbl==NULL) return;
   lr=tbl[LUMP_WADNUM(ln)]+LUMP_DIRNUM(ln);
   if(lr->data) lr->len=0;
}

void free_dead_lumps(void) {
   int i,num_wads;
   if(tbl==NULL) return;
   num_wads=get_num_wads();
   for(i=0;i<num_wads;i++) {
      int j,num_lumps=get_num_lumps(i);
      for(j=0;j<num_lumps;j++) {
	 if(tbl[i][j].data&&tbl[i][j].len==0) 
	    safe_munmap("a lump",tbl[i][j].data,tbl[i][j].len);
      }
   }
}

void free_all_lumps(void) {
   int i,num_wads;
   if(tbl==NULL) return;
   num_wads=get_num_wads();
   for(i=0;i<num_wads;i++) {
      int j,num_lumps=get_num_lumps(i);
      for(j=0;j<num_lumps;j++)
	 if(tbl[i][j].data) 
	    safe_munmap("a lump",tbl[i][j].data,tbl[i][j].len);
      safe_free(tbl[i]);
   }
   safe_free(tbl);
   tbl=NULL;
}

// Local Variables:
// c-basic-offset: 3
// End:
