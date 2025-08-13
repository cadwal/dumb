#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "lib/log.h"
#include "things.h"
#include "lib/fixed.h"

/* Return the region in which a point falls.  The method for determining
**   this is simple--probably too much so.  We shoot a ray horizontally
**   from the point.  The region containing the point is either the front
**   or back region of the wall which intersects the ray closest to the
**   point.
*/
int findsector(const LevData *ld, fixed x, fixed y, fixed z) {
   int closest_region = -1;
   fixed closest = FIXED_MAX;
   int i;

   for (i=0; i<ldnlines(ld); i++) {
      const LineData *wall=ldline(ld)+i;
      fixed y1 = ldvertexd(ld)[wall->ver1].y;
      fixed y2 = ldvertexd(ld)[wall->ver2].y;
	
      if ((y >= y1 && y < y2) || (y >= y2 && y < y1)) {
	 fixed dist;
	 const VertexDyn *v1 = ldvertexd(ld)+wall->ver1; 
	 const VertexDyn *v2 = ldvertexd(ld)+wall->ver2;

	 /* Kludge to avoid division overflows for near-
	  **   horizontal walls.
	  */
	 if (FIXED_ABS(y1 - y2) < FIXED_EPSILON)
	   dist = MIN(v1->x - x, v2->x - x);
	 else
	   dist = fixmul(fixdiv(v1->x - v2->x, v1->y - v2->y),
			 y - v1->y) + v1->x - x;
	 if (dist > FIXED_ZERO && dist < closest) {
	    closest = dist;
	    if (y1 < y2)
	      closest_region = wall->side[0];
	    else
	      closest_region = wall->side[1];
	 }
      }
   }
   if(closest_region>=0) return ldside(ld)[closest_region].sector;
   else return -1;
};

void thingd_findsector(const LevData *ld,ThingDyn *td) {
   td->sector=findsector(ld,td->x,td->y,td->z);
};

int thing2player(const LevData *ld,int th) {
   int i;
   for(i=0;i<MAXPLAYERS;i++) 
      if(ld->player[i]==th) return i;
   return -1;
};

void thing_to_view(const LevData *ld,int th,View *v,const ViewTrans *vx)  {
   ThingDyn *td=ldthingd(ld)+th;
   SectorDyn *sd=NULL;
   if(td->sector>=0) sd=ldsectord(ld)+td->sector;
   v->height=td->z;
   if(td->proto) v->height+=SHOOT_HEIGHT(td);
   v->angle=td->angle+vx->angle;
   NORMALIZE_ANGLE(v->angle);
   v->x=td->x;
   if(vx->offset) v->x+=fixmul(vx->offset,fixcos(v->angle));
   v->y=td->y;
   if(vx->offset) v->y+=fixmul(vx->offset,fixsin(v->angle));
   v->sector=td->sector;
   /* work out horizon */
   if(sd&&(td->proto==NULL||td->hits<=0)) {
      v->height=sd->floor+(2<<12);
      v->horizon=-FIXED_ONE/4;
   }
   else if(sd&&v->height<sd->floor) {
      v->height=sd->floor+(2<<12);
      v->horizon=-FIXED_ONE/4;
   }
   else if(sd&&v->height<sd->floor+FIXED_ONE/4) {
      v->horizon=v->height-(sd->floor+FIXED_ONE/4);
      v->height+=(2<<12);
   }
   else
      v->horizon=-fixsin(td->elev);
};

/* These are now inlined */
/*
int reject_sectors(LevData *ld,int s1,int s2) {
   int bit,byte;
   if(s1<0||s2<0) return 1;
   bit=s1+(s2*ldnsectors(ld));
   byte=bit/8;
   bit%=8;
   return (ldreject(ld)[byte]>>bit)&1;
};

int reject_sector_wall(LevData *ld,int s,int w) {
   int side0=ldline(ld)[w].side[0];
   int side1=ldline(ld)[w].side[1];
   if(!reject_sectors(ld,s,ldside(ld)[side0].sector)) return 0;
   if(side1>0&&!reject_sectors(ld,s,ldside(ld)[side1].sector)) return 0;
   return 1;
};
*/









