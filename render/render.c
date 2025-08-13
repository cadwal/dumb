#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "wad/wadio.h"
#include "lib/log.h"
#include "lib/fixed.h"
#include "view.h"
#include "dumb/levdyn.h"
#include "lib/safem.h"
#include "render.h"
#include "dumb/things.h"

/* Things that can be fiddled with */

/* this is used to fine-tune how fast darkness kicks in */
#define DARK_ADJUST(d) (d+d/2)

/* these cause some sanity checks to be compiled in */

//#define CHK_ACTIVE_COUNTS
//#define CHK_FBROWS
//#define CHK_FBCOLS

/* these should always be on, for DUMB */

#define LAZY_WALL_TEXTURES
#define LAZY_FLAT_TEXTURES
#define LAZY_SPRITE_TEXTURES
#define FAST_FLATS

/* Configuration stops here */

#ifndef BPP
#error "You need to define BPP before compiling render.c"
#endif

#if (BPP==1 && defined(WANT_1BPP)) ||(BPP==2 && defined(WANT_2BPP)) ||(BPP==4 && defined(WANT_4BPP))

#if BPP==1
#define USE_COLORMAP
#define COLORMAP "COLORMAP"
typedef Pixel8 Pixel;
typedef Pixel8 TPixel;
#define render render8
#define init_renderer init_renderer8
#define reset_renderer reset_renderer8
#endif

#if BPP==2
#define USE_COLORMAP
#define COLORMAP "COLORM16"
typedef Pixel16 Pixel;
typedef Pixel8 TPixel;
#define render render16
#define init_renderer init_renderer16
#define reset_renderer reset_renderer16
#endif

#if BPP==4
#define USE_COLORMAP
#define COLORMAP "COLORM32"
typedef Pixel32 Pixel;
typedef Pixel8 TPixel;
#define render render32
#define init_renderer init_renderer32
#define reset_renderer reset_renderer32
#endif

/* These macros convert 16.16 fixed point numbers to and from a 2.30 format.
**   The extra fractional precision is needed when doing doing 1 / distance
**   calculations for perspective.
*/
#define TO_FIX_2_30(f)   ((f) << 14)
#define FROM_FIX_2_30(f) ((f) >> 14)
#define TO_FIX_8_24(f)   ((f) << 8)
#define FROM_FIX_8_24(f) ((f) >> 8)

#define MAX_ERROR  view_width


/*** typedefs for wall event lists ***/

typedef struct {
   fixed z, dz;
   short wall;
   short sidenum;
   short backsect;
   short endcol;
} Wall_start;

#define MAX_WALL_EVENTS 64
typedef struct {
     int n_events;
     Wall_start events[MAX_WALL_EVENTS];
} Wall_start_list;

typedef struct {
   fixed z,dz;  /* dz = how much z changes in each screen column */
   short wall;
   short sidenum;
   short backsect;
   short endcol;
#ifdef FAST_FLATS
   short mincrow,maxcrow,minfrow,maxfrow;
#endif
   /*  ___________top___________
    * |\                       /|
    * | \________pend2________/ |
    * |  #                   #  |   ^ +z
    * |  #______pstart2______#  |   |
    * |  #\                 /#  |   |
    * |  # \_______________/ #  |   |
    * |  #  |             |  #  |
    * |  #  |             |  #  |
    * |  #  |             |  #  |
    * |  #  |             |  #  |
    * |  #  |_____________|  #  |
    * |  # /               \ #  |
    * |  #/______pend1______\#  |
    * |  #                   #  |
    * |  #______pstart1______#  |
    * | /                     \ |
    * |/________bottom_________\|
    */
   fixed pstart1, pend1, pstart2, pend2;
   fixed dpstart1, dpend1, dpstart2, dpend2;
} Active_wall;

#define AWALL_UNSEEN(a) ((a)->dpstart1==FIXED_MAX)
#define MARK_AWALL_UNSEEN(a) ((a)->dpstart1=FIXED_MAX)

/* true if AWALL completely obscures anything (at all) behind it */
#define AWALL_TERMINAL(a) ((a)->backsect<0||ldsectord(ld)[(a)->backsect].floor>=ldsectord(ld)[(a)->backsect].ceiling)

#define copy_wevent_to_active(wv,a) memcpy(a,wv,sizeof(Wall_start))

/*** typedefs for object event lists ***/

typedef struct {
   short o;
   short endcol;
   fixed z;
   fixed first_slice;
} Object_start;

#define MAX_OBJECT_EVENTS 64
typedef struct {
     int n_events;
     Object_start events[MAX_OBJECT_EVENTS];
} Object_start_list;

typedef struct {
   short o;
   short endcol;
   fixed x, dx;
   fixed z;
} Active_object;

/***/

typedef struct {
     fixed screen_dy, screen_dx;
     fixed view_sin, view_cos;
     fixed sin_dx, cos_dx;
     fixed *sin_tab, *cos_tab, *row_view, *row_reci;
} View_constants;

static void transform_vertices(void);
static void clip_walls(void);
static void add_wall_events(int wall,
			    fixed x1, fixed px1, fixed x2, fixed px2);
static void render_walls(void);
static int add_events(Active_wall *active, int n_active, int column);
static int wall_obscured(const VertexDyn *common,
			 const VertexDyn *v1,
			 const VertexDyn *v2) __attribute__((const));
static int remove_events(Active_wall *active, int n_active, int column);
static void add_objects(void);
static int add_obj_events(Active_object *active, int n_active, int column);
static int remove_obj_events(Active_object *active, int n_active, int column);
static fixed wall_ray_intersection(fixed Vx,
				   fixed Vy,
				   int wall) __attribute__((const));
static void init_buffers(void);
static void calc_view_constants(int screen_width, int screen_height);
static void do_walls(Active_wall *active, int n_active,
		     Active_object *active_obj, int n_active_obj,
		     fixed Vx, fixed Vy);

/* Event lists */
static Wall_start_list   *start_events     = NULL;
static Object_start_list *obj_start_events = NULL;

#ifdef USE_COLORMAP
static LumpNum colormap_ln=BAD_LUMPNUM;
static const Pixel *colormap = NULL,*invismap = NULL;
#endif

#define PIXEL_TO_MAP(i) ((i)<<12)

static LevData *ld = NULL;
#define WALL_VER1(i) (ldvertexd(ld)[ldline(ld)[i].ver1])
#define WALL_VER2(i) (ldvertexd(ld)[ldline(ld)[i].ver2])
#define WALL_UPUNPEGGED(i) (ldline(ld)[i].flags&LINE_UPUNPEG)
#define WALL_LOUNPEGGED(i) (ldline(ld)[i].flags&LINE_LOUNPEG)
#define WALL_ISPOSTER(i) (ldline(ld)[i].flags&LINE_POSTER)
#define WALL_LENGTH_IN_TEXELS(wall) (ldlined(ld)[wall].length>>12)
#define SIDE_XOFFSET_IN_TEXELS(side) (ldside(ld)[side].xoffset)
#define SIDE_YOFFSET_IN_TEXELS(side) (ldside(ld)[side].yoffset)

//#define OBJECT_HEIGHT(on) (ldthingd(ld)[on].proto->height)
//#define OBJECT_WIDTH(on) (ldthingd(ld)[on].proto->radius*2)
#define OBJECT_ZPEGGED(on) (ldthingd(ld)[on].proto->flags&PT_ZPEG)
#define OBJECT_ZCENTERED(on) (ldthingd(ld)[on].proto->flags&PT_ZCENTER)
#define OBJECT_HEIGHT(on) (ldthingd(ld)[on].image->height<<12)
#define OBJECT_WIDTH(on) (ldthingd(ld)[on].image->width<<12)
#define OBJECT_Z(on) (ldthingd(ld)[on].z)
#define OBJECT_X(on) (ldthingd(ld)[on].x)
#define OBJECT_Y(on) (ldthingd(ld)[on].y)
#define OBJECT_REGION(on) (ldthingd(ld)[on].sector)
#define OBJECT_GLOWS(on) (ldthingph(ld,on)->flags&TPH_GLOW)
#define OBJECT_PARTINVIS(on) \
     ((ldthingd(ld)[on].proto->flags&PT_PINVIS) | \
      (ldthingd(ld)[on].tmpinv))
#define OBJECT_IMAGE(on) (ldthingd(ld)[on].image)
#define OBJECT_IS_MIRRORIMAGE(on) (ldthingd(ld)[on].mirror_image)
#define OBJECT_INVISIBLE(on) (ldthingd(ld)[on].proto->sprite[0]==0||(ldthingd(ld)[on].proto->flags&PT_BOGUS))
#define OBJECT_EXISTS(on) (ldthingd(ld)[on].proto!=NULL)

#define REGION_FLOOR(i) ldsectord(ld)[i].floor
#define REGION_CEILING(i) ldsectord(ld)[i].ceiling
#define REGION_DARKNESS(i) ldsectord(ld)[i].dark
#define REGION_FTEX(i) ldsectord(ld)[i].ftex
#define REGION_CTEX(i) ldsectord(ld)[i].ctex
#define REGION_HAS_INFINITE_CEILING(i) 0
#define REGION_HAS_INFINITE_FLOOR(i) 0
#define REGION_HAS_SKY_CEILING(i) (ldsectord(ld)[i].sky&1)
#define REGION_HAS_SKY_FLOOR(i) (ldsectord(ld)[i].sky&2)

#define NUM_WALLS ldnlines(ld)
#define NUM_OBJECTS ldnthings(ld)

#define TEXTURE_COLUMN(t,c) (((const TPixel *)(t->texels))+(c<<t->log2height))

static Pixel *fb = NULL;
static int *fb_rows = NULL;
static int fb_topleft, fb_width;
static int view_width, view_height;
static View_constants view_constants;

static const View *view=NULL;
static int rw_column;

#if defined(i386)&&!defined(DEBUG)
#include "slice86.c"
#else
#include "slice.c"
#endif

#include "flrender.c"

void reset_renderer(void) {
   if(start_events) safe_free(start_events);
   if(obj_start_events) safe_free(obj_start_events);
   start_events=NULL;
   obj_start_events=NULL;
#ifdef USE_COLORMAP
   if(colormap_ln!=BAD_LUMPNUM) free_lump(colormap_ln);
#endif   
};
   
void init_renderer(int v_width, int v_height,int real_width,int real_height)
{
   int i,xo=0,yo=0;

   if(start_events!=NULL) reset_renderer();
   
   /* Allocate space for event lists. */
   start_events     = safe_calloc(sizeof(Wall_start_list) , (v_width + 1));
   obj_start_events = safe_calloc(sizeof(Object_start_list) , (v_width + 1));

   /* Precalculate the offsets of rows in the frame buffer--this is another
    **   'avoid multiplication' optimization.
    */
   fb_rows = safe_calloc(sizeof(int),v_height);
   for (i = 0; i < v_height; i++)
     fb_rows[i] = i * real_width;

   view_width = v_width;
   view_height = v_height;
   fb_width = real_width;

#if 0  
   xo=(real_width-view_width)/2;
   yo=(real_height-view_height)/2;
#endif   

   fb_topleft=xo+yo*real_width;
   
#ifdef USE_COLORMAP
   colormap_ln=getlump(COLORMAP);
#endif   
   
   logprintf(LOG_INFO,'R',"init_renderer fb dimensions (%d,%d) view (%d,%d) bpp=%d",
	     real_width,real_height,view_width,view_height,BPP);
};

void render(void *fbp,LevData *w, const View *v)
{
   static int invis=8,invinc=1;
   ld=w;
   fb=fbp;
   view=v;
   fb+=fb_topleft;
#ifdef USE_COLORMAP
   invismap=load_lump(colormap_ln);
   invismap+=256*invis;
   invis+=invinc;
   if(invis==16||invis==8) invinc=-invinc;
#endif
   init_buffers();
   calc_view_constants(view_width, view_height);
   transform_vertices();
   clip_walls();
   add_objects();
   render_walls();
}

static void transform_vertices(void)
{
   VertexDyn *vertex;
   const fixed view_sin = view_constants.view_sin;
   const fixed view_cos = view_constants.view_cos;
   int i;

   vertex = ldvertexd(ld);

   for (i=0;i<ldnvertices(ld);i++,vertex++) {
      fixed x = vertex->x - view->x;
      fixed y = vertex->y - view->y;

      vertex->tx = fixmul(x, view_cos) - fixmul(y, view_sin);
      vertex->ty = fixmul(x, view_sin) + fixmul(y, view_cos);
      if (vertex->tx > view->eye_distance)
	/* project point onto view plane */
	vertex->proj = fixdiv(vertex->ty, vertex->tx);
   }
}

static void clip_walls(void) {
   int i;
   for (i=0;i<NUM_WALLS;i++) {
      fixed x1, y1, px1, x2, y2, px2;
      unsigned char outcode1, outcode2;

      /* if that wall isn't in any sector that can see this one, throw it */
      //if(view->sector>=0&&reject_sector_wall(ld,view->sector,i)) continue;

      /* length 0 walls are invisible */
      if(wall_length(ld,i)==0) continue;

      x1 = WALL_VER1(i).tx;
      x2 = WALL_VER2(i).tx;

      /* See if the wall lies completely behind the view plane. */
      if (x1 <= view->eye_distance && x2 <= view->eye_distance)
	continue;

      y1 = WALL_VER1(i).ty;
      y2 = WALL_VER2(i).ty;
      px1 = WALL_VER1(i).proj;
      px2 = WALL_VER2(i).proj;

      /*** Clipping ***/

      /* First, clip to the view plane (line, really, since we're
       **   working in only two dimensions.)
       */
      if (x1 <= view->eye_distance) {
	 /* be careful for division overflow */
	 if (x2 - x1 < FIXED_EPSILON)
	   continue;
	 y1 = y1 + fixmul(view->eye_distance - x1,
			  fixdiv(y2 - y1, x2 - x1));
	 px1 = y1;
	 x1 = view->eye_distance;
      }
      if (x2 <= view->eye_distance) {
	 if (x1 - x2 < FIXED_EPSILON)
	   continue;
	 y2 = y2 + fixmul(view->eye_distance - x2,
			  fixdiv(y1 - y2, x1 - x2));
	 px2 = y2;
	 x2 = view->eye_distance;
      }

      /* Now, clip to the sides of the view polygon. */
      outcode1 = FIXED_SIGN(view->view_plane_size + px1);
      outcode1 |= FIXED_SIGN(view->view_plane_size - px1) << 1;
      outcode2 = FIXED_SIGN(view->view_plane_size + px2);
      outcode2 |= FIXED_SIGN(view->view_plane_size - px2) << 1;

      /* trivial reject */
      if ((outcode1 & outcode2) != 0)
	continue;
      /* check for trivial accept */
      if ((outcode1 | outcode2) != 0) {
	 /* Damn . . . we need to clip. */
	 fixed base_slope, slope, denom, y_diff;

	 denom = (x2 - x1);
	 if (FIXED_ABS(denom) < FIXED_EPSILON) {
	    if (denom < 0)
	      base_slope = FIXED_MIN + view->view_plane_size + 1;
	    else
	      base_slope = FIXED_MAX - view->view_plane_size;
	 } else
	   base_slope = fixdiv(y2 - y1, denom);

	 if (outcode1 == 1) {
	    px1 = -view->view_plane_size;
	    y_diff = y1 - fixmul(x1, -view->view_plane_size);
	    slope = base_slope + view->view_plane_size;
	    if (FIXED_ABS(slope) > FIXED_EPSILON)
	      x1 -= fixdiv(y_diff, slope);
	    else
	      x1 = FIXED_MAX;
	 } else if (outcode1 == 2) {
	    px1 = view->view_plane_size;
	    y_diff = y1 - fixmul(x1, view->view_plane_size);
	    slope = base_slope - view->view_plane_size;
	    if (FIXED_ABS(slope) > FIXED_EPSILON)
	      x1 -= fixdiv(y_diff, slope);
	    else
	      x1 = FIXED_MAX;
	 }

	 if (outcode2 == 1) {
	    px2 = -view->view_plane_size;
	    y_diff = y2 - fixmul(x2, -view->view_plane_size);
	    slope = base_slope + view->view_plane_size;
	    if (FIXED_ABS(slope) > FIXED_EPSILON)
	      x2 -= fixdiv(y_diff, slope);
	    else
	      x2 = FIXED_MAX;
	 } else if (outcode2 == 2) {
	    px2 = view->view_plane_size;
	    y_diff = y2 - fixmul(x2, view->view_plane_size);
	    slope = base_slope - view->view_plane_size;
	    if (FIXED_ABS(slope) > FIXED_EPSILON)
	      x2 -= fixdiv(y_diff, slope);
	    else
	      x2 = FIXED_MAX;
	 }
      }

      add_wall_events(i, x1, px1, x2, px2);
   }
}

/* Add a wall to the event list--one event is added for the start of the
**   wall, and another is added to mark the end of the wall.
*/
static void add_wall_events(int wall,
			    fixed x1, fixed px1, fixed x2, fixed px2)
{
   int fb1, fb2;
   fixed z1, z2;
   Wall_start *event;
   int sidenum;

   /* convert to frame buffer coordinates */
   px1 = fixdiv(px1, view_constants.screen_dx + 1);
   px2 = fixdiv(px2, view_constants.screen_dx + 1);
   fb1 = FIXED_TO_INT(px1) + (view_width >> 1);
   fb2 = FIXED_TO_INT(px2) + (view_width >> 1);

#ifdef CHK_FBCOLS
   if(fb1<0||fb1>=view_width||fb2<0||fb2>=view_width)
     logprintf(LOG_ERROR,'R',"fb1=%d fb2=%d for wall %d",fb1,fb2,wall);
#endif

   /* There's no need to deal with walls that start and end in the same
    **   screen column.  In a properly constructed world, we're guaranteed
    **   that throwing them away won't leave any gaps.
    */
   if (fb1 < fb2) sidenum=ldline(ld)[wall].side[0];
   else if(fb1 > fb2) sidenum=ldline(ld)[wall].side[1];
   else return;

   /* likewise, throw away walls that don't have a side facing us */
   if(sidenum<0) return;

   /* Here we use a 2.30 fixed point format.  The result of this calculation
    **   is always between 1 and zero, as the distance can never be less
    **   than the view plane distance.  The extra fractional bits are
    **   critical for the inverses.  Note that using 2.30 restricts the
    **   size of the view plane to something less than 2.
    */
   z1 = fixdiv(TO_FIX_2_30(view->eye_distance), x1);
   z2 = fixdiv(TO_FIX_2_30(view->eye_distance), x2);

   if (fb1 < fb2) {
      if(start_events[fb1].n_events>=MAX_WALL_EVENTS)
      	logfatal('R',"Too many wall start_events column=%d",fb2);

      event = &start_events[fb1].events[start_events[fb1].n_events];
      event->z = z1;
      event->dz = fixdiv(z2 - z1, INT_TO_FIXED(fb2 - fb1));
      event->sidenum = sidenum;
      event->backsect=ldline(ld)[wall].side[1];
      event->endcol=fb2;
      start_events[fb1].n_events++;

   } else {
      if(start_events[fb2].n_events>=MAX_WALL_EVENTS)
      	logfatal('R',"Too many wall start_events column=%d",fb2);

      event = &start_events[fb2].events[start_events[fb2].n_events];
      event->z = z2;
      event->dz = fixdiv(z1 - z2, INT_TO_FIXED(fb1 - fb2));
      event->backsect=ldline(ld)[wall].side[0];
      event->endcol=fb1;
      start_events[fb2].n_events++;
     };

   /* event init stuff that doesn't depend on wall's sense */
   event->wall=wall;
   event->sidenum=sidenum;
   if(event->backsect>=0) event->backsect=ldside(ld)[event->backsect].sector;
}

static void add_objects(void) {
   int i;
   const fixed view_sin = view_constants.view_sin;
   const fixed view_cos = view_constants.view_cos;

   for (i=0;i<NUM_OBJECTS;i++) {
      fixed x, y, z;
      fixed tx, ty;
      fixed height, width;
      ThingDyn *o = ldthingd(ld)+i;

      /* if this thing doesn't exist anymore, skip */
      if(!OBJECT_EXISTS(i)) continue;

      /* if thing isn't in any sector that we can see, throw it */
      if(view->sector>=0&&reject_sector_thing(ld,view->sector,i)) continue;

      /* Skip the object if it is invisible. */
      if (OBJECT_INVISIBLE(i)) continue;

      /* see object from the right angle, when the time comes */
      thing_rotate_image(ld,i,view->angle);
      if(o->image==NULL) continue;

      /* Convert object coordinates to fixed point and transform. */
      x = o->x - view->x;
      y = o->y - view->y;
      z = o->z;
#if 0 /* the old way: uses object's real width and height */
      height = OBJECT_HEIGHT(i);
      width  = OBJECT_WIDTH(i);
#else /* the new way: use width and height of sprite */
      height = PIXEL_TO_MAP(o->image->height);
      width  = PIXEL_TO_MAP(o->image->width);
#endif
      /* Rotate into viewer's coordinate system. */
      tx = fixmul(x, view_cos) - fixmul(y, view_sin);
      ty = fixmul(x, view_sin) + fixmul(y, view_cos);

      /* Only worry about the object if it is in front of the view plane */
      if (tx > view->eye_distance + FIXED_EPSILON) {
	 int fb1, fb2;
	 fixed pstart, pend;
	 fixed z;

	 /* Project the object onto the view plane . . . */
	 pstart = fixdiv(ty - FIXED_HALF(width), tx);
	 pend   = fixdiv(ty + FIXED_HALF(width), tx);

	 /* check for a potential overflow */
	 if(pstart>INT_TO_FIXED(1)||pend<INT_TO_FIXED(-1))
	   continue;

	 /* Convert to frame buffer coordinates. */
	 pstart = fixdiv(pstart, view_constants.screen_dx + 1);
	 pend   = fixdiv(pend,   view_constants.screen_dx + 1);
	 fb1 = FIXED_TO_INT(pstart) + (view_width >> 1);
	 fb2 = FIXED_TO_INT(pend)   + (view_width >> 1) - 1;

	 if (fb2 >= 0 && fb1 < view_width) {
	    Object_start_list *start_list;
	    Object_start      *start;
	    fixed             first_slice = FIXED_ZERO;

	    z = fixdiv(TO_FIX_2_30(view->eye_distance), tx);
	    if (fb1 < 0) {
	       first_slice = fixdiv(INT_TO_FIXED(-fb1),
				    INT_TO_FIXED(fb2 - fb1));
	       fb1 = 0;
	    }
	    start_list = &obj_start_events[fb1];

	    if (fb2 >= view_width)
	      fb2 = view_width - 1;

	    if(start_list->n_events>=MAX_OBJECT_EVENTS)
	      logfatal('R',"Too many obj start_events");

	    start = &start_list->events[start_list->n_events];
	    start->z = z;
	    start->o = i;
	    start->endcol=fb2;
	    start->first_slice = first_slice;
	    start_list->n_events++;
	 }
      }
   }
};

#define MAX_ACTIVE_OBJECTS 128

static void render_walls(void) {
   static int last_wall_count = 0;
   static Active_wall *active;
   static Active_object *active_obj;
   int active_count = 0, active_obj_count = 0;
   fixed Vx, Vy, dVy;


   /* Make sure that the active list is large enough to hold all the
    **   lines in a world.
    */
   if (last_wall_count != NUM_WALLS) {
      last_wall_count = NUM_WALLS;
      if (active == NULL)
      	active = safe_malloc(sizeof(Active_wall) * last_wall_count);
      else
      	active = safe_realloc(active,
			      sizeof(Active_wall) * last_wall_count);

   }
   if (active_obj == NULL)
     active_obj = safe_malloc(sizeof(Active_object) * MAX_ACTIVE_OBJECTS);

   /* Set up for fast calculation of view rays. */
   Vx = view->eye_distance;
   Vy = -view->view_plane_size;
   dVy = fixdiv(FIXED_SCALE(view->view_plane_size, 2),
		INT_TO_FIXED(view_width));

   for (rw_column = 0; rw_column < view_width; rw_column++) {
      Active_wall   *current, *last;
      Active_object *current_obj, *last_obj;

#ifdef CHK_ACTIVE_COUNT
      if(active_count<0)
      	logfatal('R',"active_count=%d at rw_column %d",active_count,rw_column);
      if(active_obj_count<0)
      	logfatal('R',"active_obj_count=%d at rw_column %d",active_obj_count,rw_column);
#endif

      active_count     = add_events(active, active_count, rw_column);
      active_obj_count = add_obj_events(active_obj, active_obj_count,
					    rw_column);

#ifdef CHK_ACTIVE_COUNT
      if(active_obj_count>MAX_ACTIVE_OBJECTS)
      	logfatal('R',"Too many active objects (%d)",active_obj_count);
#endif

      do_walls(active, active_count,
	       active_obj, active_obj_count,
	       Vx, Vy);

      active_count     = remove_events(active, active_count, rw_column);
      active_obj_count = remove_obj_events(active_obj, active_obj_count,
					   rw_column);

      /* Keep track of distances of walls in the active list.  Notice that
       **   we're not actually tracking the distances of walls, but
       **   1 / distance instead.  That's because we can linearly
       **   interpolate 1 / distance.  Also, most calculations that
       **   use distance are really using 1 / distance (i.e. distance
       **   appears in the denominator).
       */
      last = active + active_count;
      for (current = active; current < last; current++) {
	 current->z += current->dz;
	 if (!AWALL_UNSEEN(current)) {
	    current->pstart1 += current->dpstart1;
	    current->pend1 += current->dpend1;
	    current->pstart2 += current->dpstart2;
	    current->pend2 += current->dpend2;
	 };
      }

      last_obj = active_obj + active_obj_count;
      for (current_obj = active_obj; current_obj < last_obj; current_obj++)
      	current_obj->x += current_obj->dx;

      Vy += dVy;
   }

};

#ifdef i386
static inline void memmove_fwd(void *d,const void *s,size_t l)  {
   asm("rep\n\t"
       "movsl\n\t"
       :
       : "D" (d), "S" (s), "c" (l/4)
       : "edi", "esi", "ecx"
       );
};
static inline void memmove_bwd(void *d,const void *s,size_t l)  {
   asm("std\n\t"
       "leal (%%esi,%%ecx,4),%%esi\n\t"
       "leal (%%edi,%%ecx,4),%%edi\n\t"
       "inc %%ecx\n\t"
       "rep\n\t"
       "movsl\n\t"
       "cld\n\t"
       :
       : "D" (d), "S" (s), "c" ((l/4)-1)
       : "edi", "esi", "ecx"
       );
};
#else
#define memmove_fwd memmove
#define memmove_bwd memmove
#endif

#ifdef USE_VER_EQ
/* are these two vertices "different but equal"? */
static int ver_eq(int _v1,int _v2)  {
   const VertexDyn *v1=ldvertexd(ld)+_v1;
   const VertexDyn *v2=ldvertexd(ld)+_v2;
   return v1->x==v2->x&&v1->y==v2->y;
};
#endif

/* Add new walls to the active list.  The active list is kept
**   depth sorted.  We have to be careful here.  Correct depth
**   ordering of the walls is vital for rendering.  At corners
**   we have two or more walls at the same distance; however,
**   there is still a correct and incorrect ordering.  If one
**   wall is obscured by another, the visible wall must be placed
**   in front in the list.
*/
static int add_events(Active_wall *active, int n_active, int column) {
   int i, j;

   for (i = 0; i < start_events[column].n_events; i++) {
      const Wall_start *event = start_events[column].events + i;
      const int wall = event->wall;
      const fixed z = event->z;

      /*logprintf(LOG_DEBUG,'R',"add_events(column=%d wall=%d)",column,wall);*/

      for (j = 0; j < n_active; j++) {
	 const int wall2 = active[j].wall;
	 const VertexDyn *common, *v1, *v2;

	 if (z < active[j].z - MAX_ERROR)
	   continue;
	 else if (z > active[j].z + MAX_ERROR)
	   break;

	 /* See if the walls share a vertex. */
	 if (ldline(ld)[wall].ver1 == ldline(ld)[wall2].ver1) {
	    common = &WALL_VER1(wall);
	    v1 = &WALL_VER2(wall);
	    v2 = &WALL_VER2(wall2);
	 } else if (ldline(ld)[wall].ver1 == ldline(ld)[wall2].ver2) {
	    common = &WALL_VER1(wall);
	    v1 = &WALL_VER2(wall);
	    v2 = &WALL_VER1(wall2);
	 } else if (ldline(ld)[wall].ver2 == ldline(ld)[wall2].ver1) {
	    common = &WALL_VER2(wall);
	    v1 = &WALL_VER1(wall);
	    v2 = &WALL_VER2(wall2);
	 } else if (ldline(ld)[wall].ver2 == ldline(ld)[wall2].ver2) {
	    common = &WALL_VER2(wall);
	    v1 = &WALL_VER1(wall);
	    v2 = &WALL_VER1(wall2);
	 }
#ifdef USE_VER_EQ
	 /* Ill formed doom levels sometimes have coinciding vertices.
	  * Also, when we have moving vertices, this might be allowable.
	  */
	 else if (ver_eq(ldline(ld)[wall].ver1,ldline(ld)[wall2].ver1)) {
	    common = &WALL_VER1(wall);
	    v1 = &WALL_VER2(wall);
	    v2 = &WALL_VER2(wall2);
	 } else if (ver_eq(ldline(ld)[wall].ver1,ldline(ld)[wall2].ver2)) {
	    common = &WALL_VER1(wall);
	    v1 = &WALL_VER2(wall);
	    v2 = &WALL_VER1(wall2);
	 } else if (ver_eq(ldline(ld)[wall].ver2,ldline(ld)[wall2].ver1)) {
	    common = &WALL_VER2(wall);
	    v1 = &WALL_VER1(wall);
	    v2 = &WALL_VER2(wall2);
	 } else if (ver_eq(ldline(ld)[wall].ver2,ldline(ld)[wall2].ver2)) {
	    common = &WALL_VER2(wall);
	    v1 = &WALL_VER1(wall);
	    v2 = &WALL_VER1(wall2);
	 }
#endif
	 else {
	    /*    We have two walls which are really close
	     **   together, but share no vertices.  Because
	     **   of roundoff error, we don't know for certain
	     **   which one is really in front.  Ideally, this
	     **   situation will be avoided by creating worlds
	     **   which don't place non-adjoining walls extremely
	     **   close together.
	     */
	    /*
	    if(z == active[j].z) {
	       logprintf(LOG_DEBUG,'R',"serious wall closeness problem (%d,%d) z=0x%lx",wall,wall2,z);
	       logprintf(LOG_DEBUG,'R',"wall1=%d, ver1=%d, ver2=%d",wall,ldline(ld)[wall].ver1,ldline(ld)[wall].ver2);
	       logprintf(LOG_DEBUG,'R',"wall2=%d, ver1=%d, ver2=%d",wall2,ldline(ld)[wall2].ver1,ldline(ld)[wall2].ver2);
	       break;
	    };
	    */

	    if (z >= active[j].z)
	      break;
	    else
	      continue;
	 }

	 if (!wall_obscured(common, v1, v2) &&
	     wall_obscured(common, v2, v1))
	   break;

	 /* this wall is in front of me. will it always obscure me? */
	 if(AWALL_TERMINAL(active+j)&&active[j].endcol>=event->endcol)  {
	    j=-1;
	    break;
	 };

      }

      /* did this wall turn out to be invisible? */
      if(j<0) continue;

      /* insert ourselves in front */
      if(j<n_active) memmove_bwd(active + j + 1, active + j,
				 sizeof(Active_wall) * (n_active - j));
      copy_wevent_to_active(event,active+j);
      MARK_AWALL_UNSEEN(active+j);
      n_active++;
   };

   return n_active;
}


/* Insert an object into a depth sorted list. */
static int add_obj_events(Active_object *active, int n_active, int column)
{
   int i, j;
   for (i = 0; i < obj_start_events[column].n_events; i++) {
      const Object_start *event = obj_start_events[column].events + i;
      const Texture *oimage=OBJECT_IMAGE(event->o);

      for (j = 0; j < n_active && event->z < active[j].z; j++);

      memmove_bwd(active + j + 1, active + j,
	      sizeof(Active_object) * (n_active - j));
      active[j].z = event->z;
      active[j].o = event->o;
      active[j].endcol=event->endcol;
      if (event->first_slice == FIXED_ZERO)
	active[j].x = FIXED_ZERO;
      else
	active[j].x = FIXED_SCALE(event->first_slice, oimage->width);
      active[j].dx =
	fixdiv(INT_TO_FIXED(oimage->width * 2),
	       fixmul(OBJECT_WIDTH(event->o),
		      FIXED_SCALE(FROM_FIX_2_30(event->z), view_width)));
      n_active++;
   }
   return n_active;
};

/* Determine whether wall 1 is obscured by wall 2 from the view point.
**   This will be the case if a halfplane defined by wall 1 contains both the
**   view point and wall 2.  Wall 1 is defined by the points common and v1;
**   wall2 is defined by command and v2.  Note that this function uses the
**   transformed coordinates of the vertices, so the view point and view
**   direction need not be passed explicitly.
*/
static int wall_obscured(const VertexDyn *common, const VertexDyn *v1, const VertexDyn *v2)
{
   unsigned int sign1, sign2;

   const fixed x1 = common->tx - v1->tx;
   const fixed y1 = common->ty - v1->ty;
   const fixed x2 = common->tx - v2->tx;
   const fixed y2 = common->ty - v2->ty;

   /* There's some tricky stuff done here to try to find the signs of
    **   cross products without actually doing any multiplication.  I'm
    **   really not sure if avoiding a few multiplies is worth the extra
    **   overhead of sign checking, but the profiler shows this function as
    **   taking a surprisingly small percentage of execution time.
    */
   if (FIXED_PRODUCT_SIGN(x1, y2) ^ FIXED_PRODUCT_SIGN(x2, y1))
     sign1 = FIXED_PRODUCT_SIGN(x1, y2);
   else
     sign1 = FIXED_SIGN(fixmul(x1, y2) - fixmul(x2, y1));
   if (FIXED_PRODUCT_SIGN(x1, common->ty) ^
       FIXED_PRODUCT_SIGN(common->tx, y1))
     sign2 = FIXED_PRODUCT_SIGN(x1, common->ty);
   else
     sign2 = FIXED_SIGN(fixmul(x1, common->ty) - fixmul(common->tx, y1));

   if (sign1 ^ sign2)
     return 0;
   else
     return 1;
}


/* Remove walls from the active list.  Return the number of remaining
** active walls.
*/
static int remove_events(Active_wall *active, int n_active, int column)
{
   int i=0;
   while(i<n_active)  {
      int j;
      /* find the start of a gap */
      while(i<n_active&&active[i].endcol>column) i++;
      /* find its end */
      j=i;
      while(j<n_active&&active[j].endcol<=column) j++;
      /* delete gap, if found */
      if(j==n_active) n_active=i;
      else if(j>i)  {
	 memmove_fwd(active+i,active+j,sizeof(Active_wall)*(n_active-j));
	 n_active-=(j-i);
      }
      /* if not, move onto next active */
      else i++;
   };
   return n_active;
};

static int remove_obj_events(Active_object *active, int n_active, int column)
{
   int i=0;
   while(i<n_active)  {
      int j;
      /* find the start of a gap */
      while(i<n_active&&active[i].endcol>column) i++;
      /* find its end */
      j=i;
      while(j<n_active&&active[j].endcol<=column) j++;
      /* delete gap, if found */
      if(j==n_active) n_active=i;
      else if(j>i)  {
	 memmove_fwd(active+i,active+j,sizeof(Active_object)*(n_active-j));
	 n_active-=(j-i);
      }
      /* if not, move onto next active */
      else i++;
   };
   return n_active;
};

/* Calculate the value of the parameter t at the intersection
**   of the view ray and the wall.  t is 0 at the origin of
**   the wall, and 1 at the other endpoint.
*/
static fixed wall_ray_intersection(fixed Vx, fixed Vy, int wall)
{
     fixed denominator;

     const fixed Nx = -Vy;
     const fixed Ny = Vx;
     const fixed Wx = WALL_VER2(wall).tx - WALL_VER1(wall).tx;
     const fixed Wy = WALL_VER2(wall).ty - WALL_VER1(wall).ty;

     denominator = fixmul(Nx, Wx) + fixmul(Ny, Wy); /* N dot W */
     if (denominator < FIXED_EPSILON)
	  return FIXED_ONE - fixdiv(fixmul(Nx, WALL_VER1(wall).tx) +
				    fixmul(Ny, WALL_VER1(wall).ty),
				    -denominator);
     else if (denominator > FIXED_EPSILON)
	  return fixdiv(fixmul(Nx, WALL_VER1(wall).tx) +
			fixmul(Ny, WALL_VER1(wall).ty),
			-denominator);
     else
	  return FIXED_ZERO;
}


static void init_buffers(void)
{
   int i;
   for (i = 0; i < view_width + 1; i++) {
      start_events[i].n_events = 0;
      obj_start_events[i].n_events = 0;
   };
};


/* Calculate values that are dependent only on the screen dimensions and
**   the view.
*/
static void calc_view_constants(int screen_width, int screen_height) {
   static int last_height = 0, last_width = 0;
   int i;
   fixed x, y;

   /* Make sure that enough memory has been allocated for the tables. */
   if (screen_height != last_height) {
      if (last_height == 0) {
	 view_constants.row_view =
	   safe_malloc(screen_height * sizeof(fixed));
	 view_constants.row_reci =
	   safe_malloc(screen_height * sizeof(fixed));
      } else {
	 view_constants.row_view =
	   safe_realloc(view_constants.row_view,
			screen_height * sizeof(fixed));
	 view_constants.row_reci =
	   safe_realloc(view_constants.row_view,
			screen_height * sizeof(fixed));
      };
      last_height = screen_height;
   }
   if (screen_width != last_width) {
      if (last_width == 0) {
	 view_constants.sin_tab = safe_malloc(screen_width * sizeof(fixed));
	 view_constants.cos_tab = safe_malloc(screen_width * sizeof(fixed));
      } else {
	 view_constants.sin_tab =
	   safe_realloc(view_constants.sin_tab,
			screen_width * sizeof(fixed));
	 view_constants.cos_tab =
	   safe_realloc(view_constants.cos_tab,
			screen_width * sizeof(fixed));
      }
      last_width = screen_width;
   }

   view_constants.view_sin =fixsin(-view->angle);
   view_constants.view_cos =fixcos(-view->angle);
   view_constants.screen_dx = fixdiv(FIXED_DOUBLE(view->view_plane_size),
				     INT_TO_FIXED(screen_width));
   view_constants.screen_dy = fixdiv(FIXED_ONE,
				     INT_TO_FIXED(screen_height));
   view_constants.sin_dx = fixmul(view_constants.view_sin,
				  view_constants.screen_dx);
   view_constants.cos_dx = fixmul(view_constants.view_cos,
				  view_constants.screen_dx);
   y = FIXED_SCALE(view_constants.sin_dx, -(screen_width >> 1));
   x = FIXED_SCALE(view_constants.cos_dx, -(screen_width >> 1));
   for (i = 0; i < screen_width; i++) {
      view_constants.sin_tab[i] = y;
      view_constants.cos_tab[i] = x;
      y += view_constants.sin_dx;
      x += view_constants.cos_dx;
   }

   y = FIXED_SCALE(view_constants.screen_dy, screen_height >> 1);
   y-=view->horizon;
   for (i = 0; i < screen_height; i++) {
      view_constants.row_view[i] = y;
#ifdef ROW_RECI_SHR
      /* the row_reci table is used for fast(er) floors and ceilings */
      if(FIXED_ABS(y)>=FIXED_EPSILON)
	view_constants.row_reci[i] = fixdiv(FIXED_ONE>>ROW_RECI_SHR,y);
      else
	view_constants.row_reci[i] = FIXED_ONE>>ROW_RECI_SHR;
#endif
      y -= view_constants.screen_dy;
   }
};

#include "rendcore.c"

#endif /* WANT_BPPx */

// Local Variables:
// c-basic-offset:3
// End:
