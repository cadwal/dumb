#include <config.h>

#include <stdarg.h>
#include <stdio.h>

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "libdumbwad/wadwr.h"
#include "libdumbwad/wadstruct.h"

/* a quick hack, if ever there was one */

#define NVERS 4
VertexData ver[NVERS]={
   {0,0},
   {0,512},
   {512,512},
   {512,0}
};

#define NLINES 4 
LineData line[NLINES]={
   {0,1,LINE_IMPASSIBLE,0,-1,{0,-1}},
   {1,2,LINE_IMPASSIBLE,0,-1,{0,-1}},
   {2,3,LINE_IMPASSIBLE,0,-1,{0,-1}},
   {3,0,LINE_IMPASSIBLE,0,-1,{0,-1}}
};

#define NSIDES 4
SideData side[NSIDES]={
   {0,0,"","","STARG3",0},
   {0,0,"","","STARG3",0},
   {0,0,"","","STARG3",0},
   {0,0,"","","STARG3",0}
};

#define NSECTS 1
SectorData sect[NSECTS]={
   {0,128,"FLAT19","FLAT19",256,0,-1}
};

#define NTHINGS 1
ThingData thing[NTHINGS]={
   {128,128,0,1,7}
};

BlockMapHdr bmh={0,0,4,4,{0}};

int main(int argc,char **argv) {
   WADWR *w;
   short *bm;
   int i,bmsize;
   unsigned short j;
   if(argc!=3) {
      printf("Usage: mknulmap <filename> <mapname>\n");
      return 0;
   }
   log_stdout();
   w=wadwr_open(argv[1],'P');
   wadwr_lump(w,argv[2]);
   wadwr_lump(w,"THINGS");
   wadwr_write(w,thing,sizeof(ThingData)*NTHINGS);
   wadwr_lump(w,"LINEDEFS");
   wadwr_write(w,line,sizeof(LineData)*NLINES);
   wadwr_lump(w,"SIDEDEFS");
   wadwr_write(w,side,sizeof(SideData)*NSIDES);
   wadwr_lump(w,"VERTEXES");
   wadwr_write(w,ver,sizeof(VertexData)*NVERS);
   wadwr_lump(w,"SECTORS");
   wadwr_write(w,sect,sizeof(SectorData)*NSECTS);
   /* make a bogus blockmap */
   wadwr_lump(w,"BLOCKMAP");
   bm=safe_malloc(bmsize=(2*(NLINES+2)));
   bm[0]=0;
   bm[NLINES+1]=-1;
   for(i=0;i<NLINES;i++) bm[i+1]=i;
   wadwr_write(w,&bmh,8);
   j=4+bmh.numx*bmh.numy;
   for(i=0;i<bmh.numx*bmh.numy;i++) wadwr_write(w,&j,2);
   wadwr_write(w,bm,bmsize);
   /* write it all */
   wadwr_close(w);
   return 0;
}
