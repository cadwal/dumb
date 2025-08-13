#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadwr.h"
#include "libdumbwad/wadstruct.h"
#include "xwad.h"


/* thanks to Colin Reed's BSP for this code fragment */

extern inline int IsLineDefInside(int x1, int y1, int x2, int y2,
				  int xmin, int ymin, int xmax, int ymax )
{
   int outcode1,outcode2,t;

   while(1)
      {
	 outcode1=0;
	 if(y1>ymax) outcode1|=1;
	 if(y1<ymin) outcode1|=2;
	 if(x1>xmax) outcode1|=4;
	 if(x1<xmin) outcode1|=8;
	 outcode2=0;
	 if(y2>ymax) outcode2|=1;
	 if(y2<ymin) outcode2|=2;
	 if(x2>xmax) outcode2|=4;
	 if(x2<xmin) outcode2|=8;
	 if((outcode1&outcode2)!=0) return 0;
	 if(((outcode1==0)&&(outcode2==0))) return 1;
	 
	 if(!(outcode1&15))
	    {
	       t=outcode1;
	       outcode1=outcode2;
	       outcode2=t;
	       t=x1;x1=x2;x2=t;
	       t=y1;y1=y2;y2=t;
	    }
	 if(outcode1&1)
	    {
	       x1=x1+(x2-x1)*(ymax-y1)/(y2-y1);
	       y1=ymax;
	    }
	 else
	    {
	       if(outcode1&2)
		  {
		     x1=x1+(x2-x1)*(ymin-y1)/(y2-y1);
		     y1=ymin;
		  }
	       else
		  {
		     if(outcode1&4)
			{
			   y1=y1+(y2-y1)*(xmax-x1)/(x2-x1);
			   x1=xmax;
			}
		     else
			{
			   if(outcode1&8)
			      {
				 y1=y1+(y2-y1)*(xmin-x1)/(x2-x1);
				 x1=xmin;
			      }
			}
		  }
	    }
      }
}

#define V(i,n) inst->ver[inst->line[i].ver##n]
#define lineinblk(i,x,y,inst) IsLineDefInside( \
V(i,1).x,V(i,1).y,V(i,2).x,V(i,2).y, \
x*BM_BLOCKSIZE+minx,y*BM_BLOCKSIZE+miny, \
(x+1)*BM_BLOCKSIZE+minx,(y+1)*BM_BLOCKSIZE+miny)

static void wrblkmap(WADWR *w,const XWadInstance *inst) {
   int maxx=-65536,maxy=-65536;
   int minx=65536,miny=65536;
   int numx,numy;
   int i,x,y,o;

   /* ugh, but we know this is as big as a blockmap can get */
   BlockMap *bm=safe_vmalloc(65536*2);

   /* find map dimensions */
   for(i=0;i<inst->nvers;i++) {
      const int x=inst->ver[i].x,y=inst->ver[i].y;
      if(x>maxx) maxx=x;
      if(y>maxy) maxy=y;
      if(x<minx) minx=x;
      if(y<miny) miny=y;
   }
   minx-=8;
   miny-=8;
   maxx+=8;
   maxy+=8;
   bm->UMEMB(hdr).minx=minx;
   bm->UMEMB(hdr).miny=miny;
   bm->UMEMB(hdr).numx=numx=(maxx-minx)/BM_BLOCKSIZE+1;
   bm->UMEMB(hdr).numy=numy=(maxy-miny)/BM_BLOCKSIZE+1;
   /*
   logprintf(LOG_INFO,'B',"blockmap (%d,%d)-(%d,%d) : (%d,%d)",
             bm->hdr.minx=minx,bm->hdr.miny=miny,
             maxx,maxy,
	     bm->hdr.numx=numx=(maxx-minx)/BM_BLOCKSIZE+1,
	     bm->hdr.numy=numy=(maxy-miny)/BM_BLOCKSIZE+1);
   */
  
   o=/*index size*/numx*numy+/* header in shortints*/4;
   
   /* start making maps */
   for(x=0;x<numx;x++) for(y=0;y<numy;y++) {
      bm->UMEMB(hdr).idx[x+y*numx]=o;
      bm->data[o++]=0;
      for(i=0;i<inst->nlines;i++)
	 if(lineinblk(i,x,y,inst)) bm->data[o++]=i;
      bm->data[o++]=0xffff;
   }

   /* clean up */
   wadwr_write(w,bm,o*2);
   safe_vfree(bm,65536*2);
}

void save_level(XWadInstance *inst) {
   WADWR *w;
   char buf[16];
   garbage_collect(inst);
#ifndef NO_FORK
   if(!fork())
#endif
   {
      int i;
      strcpy(buf,inst->mapname);
      strcat(buf,".wad");
      w=wadwr_open(buf,'P');
      wadwr_lump(w,inst->mapname);
      wadwr_lump(w,"THINGS");
      wadwr_write(w,inst->thing,sizeof(ThingData)*inst->nthings);
      wadwr_lump(w,"LINEDEFS");
      wadwr_write(w,inst->line,sizeof(LineData)*inst->nlines);
      wadwr_lump(w,"SIDEDEFS");
      wadwr_write(w,inst->side,sizeof(SideData)*inst->nsides);
      wadwr_lump(w,"VERTEXES");
      wadwr_write(w,inst->ver,sizeof(VertexData)*inst->nvers);
      wadwr_lump(w,"SECTORS");
      wadwr_write(w,inst->sect,sizeof(SectorData)*inst->nsects);
      wadwr_lump(w,"BLOCKMAP");
#ifdef NO_FORK
      /* this may take a little while, make sure the map is finished drawing */
      XFlush(dpy);
#endif
      /* do the blockmap */
      wrblkmap(w,inst);
      /* do the reject table */
      wadwr_lump(w,"REJECT");
      for(i=0;i<(inst->nsects/8+1);i++) wadwr_write(w,"",1);
      /* write it all */
      wadwr_close(w);
      exit(0);
   }
}


#define SCOUNT(v) if((v)>=0&&(v)<n) tbl[v]++
#define SCVT(v) if((v)>=0&&(v)<n) (v)=tbl[v]
#define COUNT(v) if((v)<n) tbl[v]++
#define CVT(v) if((v)<n) (v)=tbl[v]

static void gc_vers(XWadInstance *inst) {
   int i,j,n=inst->nvers;
   int *tbl=safe_calloc(n,sizeof(int));
   /* find which vers are referenced */
   for(i=0;i<inst->nlines;i++) {
      COUNT(inst->line[i].ver1);
      COUNT(inst->line[i].ver2);
   }
   /* turn reference table into a map, moving vers as we go */
   j=0;
   for(i=0;i<n;i++) {
      if(tbl[i]>0) {
	 if(j<i) memcpy(inst->ver+j,inst->ver+i,sizeof(VertexData));
	 tbl[i]=j++;
      }
      else tbl[i]=-1;
   }
   /* use map to fixup references */
   for(i=0;i<inst->nlines;i++) {
      CVT(inst->line[i].ver1);
      CVT(inst->line[i].ver2);
   }
   /* tidy up */
   safe_free(tbl);
   inst->nvers=j;
}

static void gc_sides(XWadInstance *inst) {
   int i,j,n=inst->nsides;
   int *tbl=safe_calloc(n,sizeof(int));
   /* find which sides are referenced */
   for(i=0;i<inst->nlines;i++) {
      SCOUNT(inst->line[i].side[0]);
      SCOUNT(inst->line[i].side[1]);
   }
   /* turn reference table into a map, moving sides as we go */
   j=0;
   for(i=0;i<n;i++) {
      if(tbl[i]>0) {
	 if(j<i) memcpy(inst->side+j,inst->side+i,sizeof(SideData));
	 tbl[i]=j++;
      }
      else tbl[i]=-1;
   }
   /* use map to fixup references */
   for(i=0;i<inst->nlines;i++) {
      SCVT(inst->line[i].side[0]);
      SCVT(inst->line[i].side[1]);
   }
   /* tidy up */
   safe_free(tbl);
   inst->nsides=j;
}

static void gc_sects(XWadInstance *inst) {
   int i,j,n=inst->nsects;
   int *tbl=safe_calloc(n,sizeof(int));
   /* find which sects are referenced */
   for(i=0;i<inst->nsides;i++) 
      SCOUNT(inst->side[i].sector);
   /* turn reference table into a map, moving sects as we go */
   j=0;
   for(i=0;i<n;i++) {
      if(tbl[i]>0) {
	 if(j<i) memcpy(inst->sect+j,inst->sect+i,sizeof(SectorData));
	 tbl[i]=j++;
      }
      else tbl[i]=-1;
   }
   /* use map to fixup references */
   for(i=0;i<inst->nsides;i++) 
      SCVT(inst->side[i].sector);
   /* tidy up */
   safe_free(tbl);
   inst->nsects=j;
}

void garbage_collect(XWadInstance *inst) {
   /* 
      collect dud vertices, sides, and sectors.
      since sides reference sectors, they should be done first 
      vertices can be done anytime
   */
   gc_vers(inst);
   gc_sides(inst);
   gc_sects(inst);
   new_selection(-1,inst,0);
}
