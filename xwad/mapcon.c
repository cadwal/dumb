#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>

#include "lib/log.h"

#include "xwad.h"
#include "xwadmap.h"


/* functions for finding the closest map object of a certain kind to a
   point in the map window */

#define BIGFLOAT (65536.0*65536.0)
#define SUMSQF(x,y) (x*x+y*y)
#define SUMSQ(x,y) SUMSQF(((float)(x)),((float)(y)))

static int nearest_vertex(int x,int y,const XWadInstance *inst) {
   int i,j=-1;
   float min=BIGFLOAT;
   for(i=0;i<inst->nvers;i++) {
      int ix=VER_SCREENX(inst->ver+i),iy=VER_SCREENY(inst->ver+i);
      float dist=SUMSQ(x-ix,y-iy);
      if(dist<min) {
	 j=i;
	 min=dist;
      };
   };
   return j;
}; 
static int nearest_thing(int x,int y,const XWadInstance *inst) {
   int i,j=-1;
   float min=BIGFLOAT;
   for(i=0;i<inst->nthings;i++) {
      int ix=VER_SCREENX(inst->thing+i),iy=VER_SCREENY(inst->thing+i);
      float dist=SUMSQ(x-ix,y-iy);
      if(dist<min) {
	 j=i;
	 min=dist;
      };
   };
   return j;
}; 

static double line_dist(const XWadInstance *inst,int line,int x,int y) {
   const double cx=x,cy=y;
   const double ax=VER_SCREENX(inst->ver+inst->line[line].ver1);
   const double ay=VER_SCREENY(inst->ver+inst->line[line].ver1);
   const double bx=VER_SCREENX(inst->ver+inst->line[line].ver2);
   const double by=VER_SCREENY(inst->ver+inst->line[line].ver2);

   const double delta_x = bx-ax;
   const double delta_y = by-ay;
   double line_len, r, s, t;
   /* the length of the line in **2 */
   line_len = delta_x*delta_x+delta_y*delta_y;
   r = ((ay-cy)*(-delta_y)-(ax-cx)*delta_x)/line_len;
   /* If s becomes negative we are facing the line clockwise */
   s = ((ay-cy)*(delta_x)-(ax-cx)*delta_y)/sqrt( line_len );

   if(r<0.0) t=sqrt((cx-ax)*(cx-ax)+(cy-ay)*(cy-ay));
   else if(r>1.0) t=sqrt((cx-bx)*(cx-bx)+(cy-by)*(cy-by));
   else return s;
   if(s>0.0) return t;
   else return -t;
};

static int nearest_line(int x,int y,const XWadInstance *inst) {
   int i,j=-1;
   float min=BIGFLOAT;
   for(i=0;i<inst->nlines;i++) {
      float dist=fabs(line_dist(inst,i,x,y));
      if(dist<min) {
	 j=i;
	 min=dist;
      };
   };
   return j;
}; 

static int nearest_sector(int x,int y,const XWadInstance *inst) {
   int line=nearest_line(x,y,inst);
   float dist=line_dist(inst,line,x,y);
   int side=inst->line[line].side[dist>0.0?0:1];
   if(side<0) return -1;
   return inst->side[side].sector;
}; 


/* create new map objects */

void new_entity(int x,int y,XWadInstance *inst,int extend) {
   int i=inst->curselect;
   /* x and y are *screen* coordinates! */
   x=MAPX(x);
   y=MAPY(y);
   if(inst->showgrid) {
      x=GRIDIFY(x);
      y=GRIDIFY(y);
   };
   switch(inst->mode) {
   case(VerMode):
      i=inst->nvers++;
      inst->ver[i].x=x;
      inst->ver[i].y=y;
      break;
   case(ThingMode):
      i=inst->nthings++;
      memset(inst->thing+i,0,sizeof(ThingData));
      inst->thing[i].x=x;
      inst->thing[i].y=y;
      break;
   default:
      message(inst,"No Create function for this mode yet!");
      break;
   };
   new_selection(i,inst,extend);
};


/* move selected objects */

extern inline int ver_selected_for_move(XWadInstance *inst,int v) {
   switch(inst->mode) {
   case(VerMode): 
      return inst->curselect==v||(inst->enttbl[v]&ENT_SELECTED);
      /* TODO: add other cases */
   default:
      break;
   };
   return 0;
};

void move_entity(int x,int y,XWadInstance *inst) {
   int i;
   int do_adjust=inst->showgrid;
   /* Things get special treatment as they are independent of vertices */
   if(inst->mode==ThingMode)
      for(i=0;i<inst->nthings;i++) {
	 if(i!=inst->curselect&&!(inst->enttbl[i]&ENT_SELECTED)) continue;
	 inst->thing[i].x-=x;
	 inst->thing[i].y-=y;
      }
   else 
      for(i=0;i<inst->nvers;i++) {
	 if(!ver_selected_for_move(inst,i)) continue;
	 if(do_adjust) {
	    int tx=inst->ver[i].x-x;
	    int ty=inst->ver[i].y-y;
	    x+=tx-GRIDIFY(tx);
	    y+=ty-GRIDIFY(ty);
	    do_adjust=0;
	 };
	 inst->ver[i].x-=x;
	 inst->ver[i].y-=y;
      };
   /* in thingmode or vermode, current selection's stats may have changed */
   if(inst->mode==ThingMode||inst->mode==VerMode)
      rdoutp_cseti(&inst->modectls,inst);
};


/* mouse action helpers */

static void selection_by_mouse(int x,int y,XWadInstance *inst,int ext) {
   switch(inst->mode) {
   case(ThingMode): new_selection(nearest_thing(x,y,inst),inst,ext); break;
   case(VerMode): new_selection(nearest_vertex(x,y,inst),inst,ext); break;
   case(LineMode): new_selection(nearest_line(x,y,inst),inst,ext); break;
   case(SectMode): new_selection(nearest_sector(x,y,inst),inst,ext); break;
   };
};

static void drag_start(XWadInstance *inst,int x,int y,DragMode mode) {
   inst->dragstartx=x;
   inst->dragstarty=y;
   inst->dragmode=mode;
   switch(mode) {
   case(DragObj): XDefineCursor(dpy,inst->map,drag_obj_cursor); break;
   case(DragMap): XDefineCursor(dpy,inst->map,drag_map_cursor); break;
   case(NoDrag): break;
   };
};
static void drag_update(XWadInstance *inst,int x,int y) {
   int mapdx=UNSCALE(inst->dragstartx-x);
   int mapdy=-UNSCALE(inst->dragstarty-y);
   switch(inst->dragmode) {
   case(DragMap):
      inst->xoffset+=mapdx;
      inst->yoffset+=mapdy;
      break;
   case(DragObj):
      if(inst->showgrid&&abs(mapdx)<inst->gridsize&&abs(mapdy)<inst->gridsize)
	 return; /* don't update if within a grid square */
      move_entity(mapdx,mapdy,inst);
      break;
   case(NoDrag): break;
   };
   inst->dragstartx=x;
   inst->dragstarty=y;
};
static void drag_end(XWadInstance *inst,int x,int y) {
   drag_update(inst,x,y);
   inst->dragmode=NoDrag;
   XClearArea(dpy,inst->map,0,0,0,0,True);
   XUndefineCursor(dpy,inst->map);
};


/* event handlers */

void map_button(XButtonEvent *ev,XWadInstance *inst) {
   if(ev->type==ButtonPress) {
      //logprintf(LOG_DEBUG,'M',"mapbpress d=%d s=%d",ev->button,ev->state);
      /* button 1: "select" */
      if(ev->button==1) {
	 selection_by_mouse(ev->x,ev->y,inst,ev->state&ShiftMask);
	 /* if we're in ver mode, and click was right on top of
	    the now-selected vertex,... */
	 if(inst->mode==VerMode&&inst->curselect>=0&&inst->dragmode==NoDrag&&
	    SUMSQ(ev->x-VER_SCREENX(inst->ver+inst->curselect),
		  ev->y-VER_SCREENY(inst->ver+inst->curselect))<128.0)
	    drag_start(inst,ev->x,ev->y,DragObj);
      }
      /* button 2: "create" */
      else if(ev->button==2)
	 new_entity(ev->x,ev->y,inst,ev->state&ShiftMask);
      /* button 3: "drag map" */
      else if(ev->button==3&&inst->dragmode==NoDrag)
	 drag_start(inst,ev->x,ev->y,DragMap);
   }
   else {
      //logprintf(LOG_DEBUG,'M',"mapbrel d=%d s=%d",ev->button,ev->state);
      if(ev->button==3&&inst->dragmode==DragMap)
	 drag_end(inst,ev->x,ev->y);
      else if(ev->button==1&&inst->dragmode==DragObj)
	 drag_end(inst,ev->x,ev->y);
   };
};
void map_motion(XMotionEvent *ev,XWadInstance *inst) {
   //logprintf(LOG_DEBUG,'M',"mapmotion s=%d",ev->state);
   if(inst->dragmode!=NoDrag) {
      drag_update(inst,ev->x,ev->y);
      draw_map(inst,inst->map,1);
   };
};

