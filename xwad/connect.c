#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "xwad.h"

#define V(i) (inst->ver[i])
#define L(i) (inst->line[i])
#define S(l,i) (inst->side[inst->line[l].side[i]])
#define SE(i) (inst->sect[i])
#define SEL(i) (inst->enttbl[i]&ENT_SELECTED)

static int mkvxy(XWadInstance *inst,int x,int y) {
   int i=inst->nvers++;
   V(i).x=x;
   V(i).y=y;
   return i;
}
#define mkver(i) mkvxy(i,0,0)

static int mksect(XWadInstance *inst) {
   int i=inst->nsects++;
   memset(&SE(i),0,sizeof(SectorData));
   return i;
}
static int cpysect(XWadInstance *inst,int s) {
   int i=inst->nsects++;
   memcpy(&SE(i),&SE(s),sizeof(SectorData));
   return i;
}

static int mkside(XWadInstance *inst) {
   int i=inst->nsides++;
   memset(inst->side+i,0,sizeof(SideData));
   inst->side[i].sector=-1;
   return i;
}
static int mkline(XWadInstance *inst) {
   int l=inst->nlines++;
   memset(&L(l),0,sizeof(LineData));
   L(l).side[0]=mkside(inst);
   L(l).side[1]=-1;
   return l;
}
static int mkline2s(XWadInstance *inst) {
   int l=inst->nlines++;
   memset(&L(l),0,sizeof(LineData));
   L(l).side[0]=mkside(inst);
   L(l).side[1]=mkside(inst);
   return l;
}
static int cpyline(XWadInstance *inst,int o) {
   int i,l=inst->nlines++;
   memcpy(&L(l),&L(o),sizeof(LineData));
   for(i=0;i<2;i++) 
      if(L(o).side[i]>=0) {
	 L(l).side[i]=inst->nsides++;
	 memcpy(&S(l,i),&S(o,i),sizeof(SideData));
      }
   return l;
}

/* Find all the lines that begin or end at VER and put the numbers of
 * the TBLLEN first ones to TBL in ascending order.  Return the number
 * of lines found, whether that is less or more than TBLLEN.  If
 * TBLLEN is 0, TBL is not used and may be NULL.  */
static unsigned
find_lines_at_vertex(AppInst *inst, int ver, int tbl[], unsigned tbllen)
{
   unsigned found=0;
   int l;
   for (l=0; l<inst->nlines; l++) {
      if (inst->line[l].ver1==ver || inst->line[l].ver2==ver) {
	 /* Now someone of course says that I could optimize by
          * breaking out of the loop as soon as found==tbllen and
          * entering another loop where found wouldn't be tested at
          * all... well, how many comparisons would that save?  How
          * many lines have you seen connected to one vertex?  */
	 if (found < tbllen)
	    tbl[found] = l;
	 found++;		/* even if it's >tbllen */
      }
   }
   return found;
}

/* Find all selected lines and put the numbers of the TBLLEN first
 * ones to TBL in ascending order.  LineMode must be active.  Return
 * the number of lines found, whether that is less or more than
 * TBLLEN.  If TBLLEN is 0, TBL is not used and may be NULL.  */
static unsigned
find_sel_lines(AppInst *inst, int tbl[], unsigned tbllen)
{
   unsigned found=0;
   int l;
   for (l=0; l<inst->nlines; l++) {
      if ((inst->enttbl[l] & ENT_SELECTED)
	  || l==inst->curselect) {
	 if (found < tbllen)
	    tbl[found] = l;
	 found++;
      }
   }
   return found;
}

void split_sel_lines(XWadInstance *inst) {
   int i;
   int nv,nl;
   for(i=0;i<inst->nlines;i++) {
      if(i!=inst->curselect&&!SEL(i)) continue;
      nv=inst->nvers++;
      V(nv).x=( V(L(i).ver1).x + V(L(i).ver2).x )/2;
      V(nv).y=( V(L(i).ver1).y + V(L(i).ver2).y )/2;
      nl=cpyline(inst,i);
      L(nl).ver2=L(i).ver2;
      L(nl).ver1=L(i).ver2=nv;
   }
   /*new_selection(-1,inst,0);*/
}

/* If lines (L1X1,L1Y1)-(L1X2,L1Y2) and (L2X1,L2Y1)-(L2X2,L2Y2) cross,
 * put the coordinates of the crossing point to *XP and *YP and return
 * 1.  If the lines don't cross, return 0 without changing *XP and
 * *YP.  */
static int
crossing_point(int l1x1, int l1y1, int l1x2, int l1y2,
	       int l2x1, int l2y1, int l2x2, int l2y2,
	       int *xp, int *yp)
{
#define TWOWAY_BETWEEN(x,a,b) (((a)<=(b)) ? ((a)<=(x)&&(x)<=(b)) : ((b)<=(x)&&(x)<=(a)))
#define SWAP_INTS(a,b) do { int swaptmp=(a); (a)=(b); (b)=swaptmp; } while (0)
   double x, y;
   double l1dx, l1dy, l2dx, l2dy; /* used in the general case */
   /* Sanity check.  */
   if (l1x1==l1x2 && l1y1==l1y2) return 0;
   if (l2x1==l2x2 && l2y1==l2y2) return 0;
   /* Vertical lines would cause division by zero later.
    * Handle them right here.  */
   if (l2x1 == l2x2) {
      if (l1x1 == l1x2) {
	 /* Both lines are vertical, so they can't cross.  */
	 return 0;
      }
      /* Swap the lines so that when one of the lines is vertical,
       * it's always the first one.  */
      SWAP_INTS(l1x1, l2x1);
      SWAP_INTS(l1y1, l2y1);
      SWAP_INTS(l1x2, l2x2);
      SWAP_INTS(l1y2, l2y2);
   }
   if (l1x1 == l1x2) {
      /* The second line is already known not to be vertical.  */
      x = (double) l1x1;	/* which equals l1x2 */
      /* If x isn't between l2x1 and l2x2, the lines can't cross.  */
      if (!TWOWAY_BETWEEN(x, l2x1, l2x2))
	 return 0;
      /* Find the y coordinate of the line at x.  */
      y = l2y1 + (x-l2x1)*(l2y2-l2y1)/(l2x2-l2x1);
      /* Is it on the first line?  */
      if (!TWOWAY_BETWEEN(y, l1y1, l1y2))
	 return 0;
      *xp = (int) x;
      *yp = (int) y;
      return 1;
   }
   /* Now the general case...
    *
    * To simplify calculations, define:
    *  l1dx = l1x2-l1x1
    *  l1dy = l1y2-l1y1
    *  l2dx = l2x2-l2x1
    *  l2dy = l2y2-l2y1
    *
    * (x-l1x1)/l1dx = (y-l1y1)/l1dy
    * (x-l2x1)/l2dx = (y-l2y1)/l2dy
    *
    * y-l1y1 = (x-l1x1)*l1dy/l1dx
    * y-l2y1 = (x-l2x1)*l2dy/l2dx
    *
    * y = (x-l1x1)*l1dy/l1dx + l1y1
    * y = (x-l2x1)*l2dy/l2dx + l2y1
    *
    * (x-l1x1)*l1dy/l1dx+l1y1 = (x-l2x1)*l2dy/l2dx+l2y1
    *
    * x*l1dy/l1dx - l1x1*l1dy/l1dx + l1y1
    * = x*l2dy/l2dx - l2x1*l2dy/l2dx + l2y1
    *
    * x*l1dy/l1dx - x*l2dy/l2dx 
    * = l1x1*l1dy/l1dx - l2x1*l2dy/l2dx + l2y1 - l1y1
    *
    * x*(l1dy/l1dx - l2dy/l2dx)
    * = l1x1*l1dy/l1dx - l2x1*l2dy/l2dx + l2y1 - l1y1
    *
    * x = (l1x1*l1dy/l1dx - l2x1*l2dy/l2dx + l2y1 - l1y1)
    *     / (l1dy/l1dx - l2dy/l2dx)
    */
   l1dx = l1x2-l1x1;		/* the d-variables are of type double */
   l1dy = l1y2-l1y1;
   l2dx = l2x2-l2x1;
   l2dy = l2y2-l2y1;
   x = ((l1x1*l1dy/l1dx - l2x1*l2dy/l2dx + l2y1 - l1y1)
	/ (l1dy/l1dx - l2dy/l2dx));
   y = l2y1 + (x-l2x1)*(l2y2-l2y1)/(l2x2-l2x1);
   if (!TWOWAY_BETWEEN(x, l1x1, l1x2))
      return 0;
   if (!TWOWAY_BETWEEN(x, l2x1, l2x2))
      return 0;
   /* no need to check y */
   *xp = (int) x;
   *yp = (int) y;
   return 1;
#undef TWOWAY_BETWEEN
#undef SWAP_INTS
}

/* If L1 and L2 cross, add a new vertex at that point, split both
 * lines at it and return 1.  If they don't cross, return 0 and don't
 * change anything.  */
static int
cross_lines(XWadInstance *inst, int l1, int l2)
{
   int x, y;
   if (crossing_point(V(L(l1).ver1).x, V(L(l1).ver1).y,
		      V(L(l1).ver2).x, V(L(l1).ver2).y,
		      V(L(l2).ver1).x, V(L(l2).ver1).y,
		      V(L(l2).ver2).x, V(L(l2).ver2).y,
		      &x, &y)) {
      int crossver;
      /* The crossing point may be at an endpoint of one line; if so,
       * don't make a new vertex.  */
      if (V(L(l1).ver1).x==x && V(L(l1).ver1).y==y)
	 crossver = L(l1).ver1;
      else if (V(L(l1).ver2).x==x && V(L(l1).ver2).y==y)
	 crossver = L(l1).ver2;
      else if (V(L(l2).ver1).x==x && V(L(l2).ver1).y==y)
	 crossver = L(l2).ver1;
      else if (V(L(l2).ver2).x==x && V(L(l2).ver2).y==y)
	 crossver = L(l2).ver2;
      else
	 crossver = mkvxy(inst, x, y);
      /* Don't make zero-length lines.
       * NOTE: The two original lines may already share a vertex!  */
      if (L(l1).ver1!=crossver && L(l1).ver2!=crossver) {
	 int nl = cpyline(inst, l1);
	 L(nl).ver2 = L(l1).ver2;
	 L(nl).ver1 = L(l1).ver2 = crossver;
      }
      if (L(l2).ver1!=crossver && L(l2).ver2!=crossver) {
	 int nl = cpyline(inst, l2);
	 L(nl).ver2 = L(l2).ver2;
	 L(nl).ver1 = L(l2).ver2 = crossver;
      }
      return 1;
   } else /* lines don't cross */
      return 0;
}

void
cross_sel_lines(XWadInstance *inst) 
{
   int l[2];
   if (find_sel_lines(inst, l, 2) != 2) {
      qmessage(inst, "You must select exactly two lines.");
      return;
   }
   if (!cross_lines(inst, l[0], l[1]))
      qmessage(inst, "Lines don't cross.");
}

void connect_sel_vers(XWadInstance *inst,int ccw) {
   double a=0.0;
   int i,v,first_line;
   int concave=0;
   char *touched;
   /* make sure we have at least one selection (though we really need 3) */
   if(inst->curselect<0||inst->curselect>=inst->nvers) return;
   inst->enttbl[inst->curselect]|=ENT_SELECTED;
   touched=(char *)safe_calloc(1,inst->nvers);
   /* find an appropriate vertex to start with */
   v=inst->curselect;
   for(i=0;i<inst->nvers;i++) 
      if(inst->enttbl[i]&ENT_SELECTED&&V(i).x>V(v).x)
	 v=i;
   first_line=inst->nlines;
   //logprintf(LOG_DEBUG,'C',"first vertex is %d, angle %f",v,a);
   /* loop through vertices connecting them */
   i=v;
   do {
      int j,l;
      int mv=-1;
      double ma=0.0,md=0.0;
      touched[i]=1;
      /* find the next vertex to connect */
      for(j=0;j<inst->nvers;j++) {
	 double na,d;
	 if(j==i||!SEL(j)) continue;
	 //if(touched[j]) continue;
	 na=atan2(V(j).y-V(i).y,V(j).x-V(i).x);
	 if(ccw) d=na-a;
	 else d=a-na;
	 while(d>2.0*M_PI) d+=2.0*M_PI;
	 while(d<0.0) d+=2.0*M_PI;
	 //logprintf(LOG_DEBUG,'C',"  vertex %d, angle is %f",j,d);
	 if(d>md) {
	    ma=na;
	    md=d;
	    mv=j;
	 }
      }
      /* check that i and mv aren't already connected */
      for(j=0;j<inst->nlines;j++) 
	 if((L(j).ver1==i&&L(j).ver2==mv)||(L(j).ver1==mv&&L(j).ver2==i)) 
	    break;
      /* connect i -> mv */
      if(j==inst->nlines) {
	 logprintf(LOG_DEBUG,'C',"connecting %d -> %d",i,mv);
	 l=mkline(inst);
	 L(l).ver1=i;
	 L(l).ver2=mv;
      }
      else
	 logprintf(LOG_DEBUG,'C',"%d and %d are already connected",i,mv);
      /* ready for next stage of loop */
      i=mv;
      a=ma;
   } while(i!=v&&i>=0);
   /* check for concavity */
   for(i=0;i<inst->nvers;i++) 
      if((inst->enttbl[i]&ENT_SELECTED)&&!touched[i]) concave=1;
   /* go to line mode */
   enter_mode(inst,LineMode);
   for(i=first_line;i<inst->nlines;i++) new_selection(i,inst,1);
   /* clean up */
   safe_free(touched);
   if(first_line==inst->nlines) qmessage(inst,"No vertices were connected.");
   else if(concave) qmessage(inst,"Some vertices were not connected.");
}

void
join_lines_at_vertex(XWadInstance *inst, int joinver)
{
   /* I have a nasty feeling that I've made this a lot more complex
    * than necessary... */
   int l[2];			/* linedef numbers */
   int v[2];			/* other endpoints of lines */
   int side[2], altside[2];	/* sides to be used */
   int prefer_turn=0;
   int i;
   if (find_lines_at_vertex(inst, joinver, l, 2) != 2) {
      qmessage(inst, "There must be exactly two lines connected to"
	       " the selected vertex.");
      return;
   }
   for (i=0; i<2; i++) {
      LineData *p = &inst->line[l[i]]; /* for convenience */
      if (p->ver1 == joinver) {
	 /*       altside[i]
	  * v[i] -----------> joinver
	  *        side[i]
	  */
	 v[i] = p->ver2;
	 altside[i] = p->side[0];
	 side[i] = p->side[1];
	 if (i==0)
	    prefer_turn=1;
      } else {
	 /*       altside[i]
 	  * v[i] <----------- joinver
	  *        side[i]
	  */
	 v[i] = p->ver1;
	 side[i] = p->side[0];
	 altside[i] = p->side[1];
      }
   }
   /*       altside[0]            side[1]
    * v[0] <----------> joinver <----------> v[1]
    *        side[0]             altside[1]
    *
    * will be transformed to:
    *
    *       side[1]
    * v[0] <-------> v[1]
    *       side[0]
    *
    * where the linedef will be turned as necessary to make the front side
    * not -1.
    */
   /* avoid losing information */
   if (side[0]==-1) side[0]=altside[1];	/* which may be -1 too */
   if (side[1]==-1) side[1]=altside[0];
   if (prefer_turn ? side[1]!=-1 : side[0]==-1) {
      int tmp;
      tmp=side[0]; side[0]=side[1]; side[1]=tmp;
      tmp=v[0]; v[0]=v[1]; v[1]=tmp;
   }
   /* apply!  */
   inst->line[l[0]].side[0] = side[0]; /* don't use memcpy(), */
   inst->line[l[0]].side[1] = side[1]; /* the types differ */
   inst->line[l[0]].ver1 = v[0];
   inst->line[l[0]].ver2 = v[1];
   if (side[1] == -1)
      inst->line[l[0]].flags &= ~LINE_TWOSIDED;
   else
      inst->line[l[0]].flags |= LINE_TWOSIDED;
   /* delete l[1]... FIXME: dirty */
   inst->line[l[1]] = inst->line[--inst->nlines];
   /* note: l[0] < l[1] */
   enter_mode(inst, LineMode);
   new_selection(l[0], inst, 0);
}

void make_sector_from_sel_lines(XWadInstance *inst) {
   int i=inst->curselect;
   int side=0,sect;
   /* make sure at least one is selected */
   if(i<0||i>=inst->nlines) return;
   inst->enttbl[i]|=ENT_SELECTED;
   /* internal or external? */
   if(L(i).side[0]>=0&&S(i,0).sector>=0) side=1;
   /* make the sector */
   sect=mksect(inst);
   /* loop through the lines */
   for(i=0;i<inst->nlines;i++) {
      if(!SEL(i)) continue;
      if(L(i).side[side]<0) L(i).side[side]=mkside(inst);
      S(i,side).sector=sect;
      if(side) L(i).flags|=LINE_TWOSIDED;
   }
   /* go to sector mode */
   enter_mode(inst,SectMode);
   new_selection(sect,inst,0);
}

int make_corridor_between(XWadInstance *inst,int l1,int l2) {
   int s1,s2;
   int csect,l3,l4;
   s1=S(l1,0).sector;
   s2=S(l2,0).sector;
   if(S(l1,0).sector<0||S(l2,0).sector<0||L(l1).side[1]>=0||L(l2).side[1]>=0) {
      qmessage(inst,"You must select two one-sided sector boundaries.");
      return -1;
   }
   /* build corridor */
   csect=cpysect(inst,s1);
   L(l1).side[1]=mkside(inst);
   L(l2).side[1]=mkside(inst);
   L(l1).flags|=LINE_TWOSIDED;
   L(l2).flags|=LINE_TWOSIDED;
   S(l1,1).sector=csect;
   S(l2,1).sector=csect;
   *S(l1,0).texture=0;
   *S(l2,0).texture=0;
   l3=mkline(inst);
   l4=mkline(inst);
   L(l3).ver1=L(l1).ver1;
   L(l3).ver2=L(l2).ver2;
   L(l4).ver1=L(l2).ver1;
   L(l4).ver2=L(l1).ver2;
   S(l3,0).sector=csect;
   S(l4,0).sector=csect;
   return csect;
}

static int get_corridor(XWadInstance *inst,int sect,
			int portals[2],int walls[2]) {
   int i,np=0,nw=0;
   for(i=0;i<inst->nlines;i++) {
      if(L(i).side[0]>=0&&S(i,0).sector==sect) {
	 if(nw>=2) return -1;
	 else walls[nw++]=i;
      }
      else if(L(i).side[1]>=0&&S(i,1).sector==sect) {
	 if(np>=2) return -1;
	 else portals[np++]=i;
      }
   }
   if(np==2&&nw==2) return 0;
   else return -1;
}

#define PS(n) SE(S(p[n],0).sector)
void make_stairs(XWadInstance *inst,int sect,int inc) {
   int p[2],w[2];
   int i,nstairs,lw0,lw1,lsect;
   int w0v1,w0v2,w1v1,w1v2;
   if(get_corridor(inst,sect,p,w)) {
      qmessage(inst,"Sector does not seem corridor-like.");
      return;
   }
   /* put lower portal in p[0] */
   if(PS(0).floor>PS(1).floor) {
      i=p[0];
      p[0]=p[1];
      p[1]=i;
   }
   /* make sure w[0] and p[0] intersect on w[0].ver1 */
   if(L(w[0]).ver1!=L(p[0]).ver1&&L(w[0]).ver1!=L(p[0]).ver2) {
      i=w[0];
      w[0]=w[1];
      w[1]=i;
   }
   /* work out how many stairs will be needed */
   nstairs=(PS(1).floor-PS(0).floor)/inc;
   logprintf(LOG_DEBUG,'C',"make_stairs: nstairs=%d",nstairs);
   /* adjust floor and ceiling to sensible values */
   SE(sect).floor=PS(0).floor;
   SE(sect).ceiling=PS(1).ceiling;
   /* build them */
   w0v1=L(w[0]).ver1;
   w1v1=L(w[1]).ver1;
   w0v2=L(w[0]).ver2;
   w1v2=L(w[1]).ver2;
   lsect=sect;
   lw0=w[0];
   lw1=w[1];
   for(i=1;i<nstairs;i++) {
      int l=mkline2s(inst);
      S(l,0).sector=lsect;
      S(l,1).sector=lsect=cpysect(inst,lsect);
      SE(lsect).floor=SE(lsect).floor+inc;
      L(lw0).ver2=L(l).ver1=mkvxy(inst,
				  (V(w0v2).x*i+V(w0v1).x*(nstairs-i))/nstairs,
				  (V(w0v2).y*i+V(w0v1).y*(nstairs-i))/nstairs
				  );
      L(lw1).ver1=L(l).ver2=mkvxy(inst,
				  (V(w1v1).x*i+V(w1v2).x*(nstairs-i))/nstairs,
				  (V(w1v1).y*i+V(w1v2).y*(nstairs-i))/nstairs
				  );
      lw0=cpyline(inst,lw0);
      lw1=cpyline(inst,lw0);
      L(lw0).ver1=L(l).ver1;
      L(lw1).ver2=L(l).ver2;
      S(lw0,0).sector=lsect;
      S(lw1,0).sector=lsect;
   }
   S(p[1],1).sector=lsect;
   L(lw0).ver2=w0v2;
   L(lw1).ver1=w1v1;
}

// Local Variables:
// c-basic-offset: 3
// End:
