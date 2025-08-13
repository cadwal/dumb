
#ifndef WADWR_H
#define WADWR_H

#include "libdumbwad/wadstruct.h"

typedef struct {
   int f;
   WadHeader hdr;
   WadDirEntry *dir,*current;
   int maxdir;
   const char *fname;
   char type;
} WADWR;

WADWR *wadwr_open(const char *fname,char wadtype);
void wadwr_close(WADWR *w);

void wadwr_lump(WADWR *w,const char *lumpname);
void wadwr_write(WADWR *w,const void *lump,size_t len);

#endif
