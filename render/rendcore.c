
/* included by render.c */

/**********************************************************************
**
** This section contains the guts of the wall drawing functions.  There
**   are a number of static variables declared here which are used to pass
**   information between these functions.  They are not used anyplace else.
*/

static fixed pstart1, pend1, pstart2, pend2;
static fixed top, bottom;
static fixed z;
static int front, back;

static void draw_wall_segment(fixed pstart, fixed pend,
			      fixed start,
			      Texture *tex,
			      int tex_column,
			      fixed tex_dy,
			      int yoffset,
			      int opaque,
			      fixed scale)
{
   Pixel *fb_byte, *last_byte;
   int fb_start, fb_end;
   const TPixel *tex_base;
   int fb_column;

   if(tex==NULL) return;
#ifdef LAZY_WALL_TEXTURES
   cond_load_texels(tex,sizeof(TPixel));
#endif   
   
   if (tex_column < 0)
      tex_column = tex->width-1 - (tex->width-1 - tex_column) % tex->width;
   else
      tex_column %= tex->width;
   /* A slower way to do this:
    *   while(tex_column<0) tex_column+=tex->width;
    *   tex_column%=tex->width;
    * Faster but less general:
    *   tex_column&=((1<<tex->log2width) - 1);
    */
   tex_base=TEXTURE_COLUMN(tex,tex_column);
   
   /* Clip the wall slice. */
   if (pstart < bottom) {
      start += fixdiv(bottom-pstart,z);
      pstart = bottom;
   }
   if (pend > top)
     pend = top;
   
   fb_start = (view_height >> 1) - 1 -
     FIXED_TO_INT(FIXED_SCALE(pstart, view_height));
   fb_end   = (view_height >> 1) - 
     FIXED_TO_INT(FIXED_SCALE(pend, view_height));

   if(fb_start<fb_end) return;
   
#ifdef CHK_FBROWS
   if(fb_start<0||fb_end<0||fb_start>=view_height||fb_end>=view_height) {
      logprintf(LOG_INFO,'R',"start=%d end=%d",fb_start,fb_end);
      logfatal('R',"problems with fb coords in draw_wall");
   };
#endif
   
   fb_column = view_width - rw_column - 1;
   fb_byte   = fb + fb_column + fb_rows[fb_start];
   last_byte = fb + fb_column + fb_rows[fb_end];

   start=fixmul(start,scale);
   start-=INT_TO_FIXED(yoffset);
   if(start<0) start+=INT_TO_FIXED(tex->height);
   
   if (opaque)
     draw_wall_slice(fb_byte, last_byte, 
		     tex_base, start, tex_dy,
		     tex->log2height);
   else
     draw_trans_slice(fb_byte, last_byte, 
		      tex_base, start, tex_dy,
		      tex->log2height);
};

#define FIXED_2THIRDS (INT_TO_FIXED(2)/3)

static void draw_sky_segment(fixed pstart, fixed pend,
			     Texture *tex)
{
   Pixel *fb_byte,*last_byte;
   int fb_start,fb_end;
   const TPixel *tex_base;
   fixed tex_dy,tex_y;
   int tex_column,fb_column;
   fixed angle;

#ifdef LAZY_WALL_TEXTURES
   cond_load_texels(tex,sizeof(TPixel));
#endif   
   /* Compute the angle of this column.  We'll use the angle
    **   exclusively to determine which texture column to
    **   display for this slice of sky.
    */
   angle = view->angle +
     fixdiv(FIXED_SCALE(view->arc, rw_column - (view_width >> 1)),
	    INT_TO_FIXED(view_width));
   angle = fixdiv(angle, FIXED_2PI);
   if (angle < FIXED_ZERO) angle+=FIXED_ONE;

   tex_column = FIXED_TO_INT(fixmul(angle,INT_TO_FIXED(1024))) &
     ((1<<tex->log2width) - 1);

   /* sky always takes up 3/4 of the viewport
      TODO: make this configurable? */
   tex_dy = fixdiv(4*INT_TO_FIXED(tex->height), 3*INT_TO_FIXED(view_height));
   tex_base=TEXTURE_COLUMN(tex,tex_column);
   
   /* Clip the wall slice. */
   if (pstart < bottom) pstart = bottom;
   if (pend > top) pend = top;
   
   fb_start = (view_height >> 1) - 1 -
     FIXED_TO_INT(FIXED_SCALE(pstart, view_height));
   fb_end   = (view_height >> 1) -
     FIXED_TO_INT(FIXED_SCALE(pend, view_height));

   if(fb_start<fb_end) return;
   
#ifdef CHK_FBROWS
   if(fb_start<0||fb_end<0||fb_start>=view_height||fb_end>=view_height) {
      logprintf(LOG_INFO,'R',"start=%d end=%d",fb_start,fb_end);
      logfatal('R',"problems with fb coords in draw_wall");
   };
#endif
   
   fb_column = view_width - rw_column - 1;
   fb_byte   = fb + fb_column + fb_rows[fb_start];
   last_byte = fb + fb_column + fb_rows[fb_end];

   tex_y=INT_TO_FIXED(tex->height-1)-fixmul(tex_dy,INT_TO_FIXED(fb_start));
   /*tex_y=INT_TO_FIXED(tex->height-1)-
      fixmul(tex_dy,FIXED_SCALE(pstart, view_height));*/
   
   draw_wall_slice(fb_byte, last_byte, 
		   tex_base, tex_y, tex_dy,
		   tex->log2height);
};


static void draw_wall(Active_wall *active,
		      fixed Vx, fixed Vy)
{
   const int wall=active->wall,side=active->sidenum;
   char do_lower, do_upper, do_middle;
   fixed pstart3,pend3,pfend,pcstart;
#ifdef USE_COLORMAP
   fixed dark;
#endif
   int tex_col,tex_yoffset;
   fixed tex_dy;
   fixed scale=INT_TO_FIXED(16);
   
   /*logprintf(LOG_DEBUG,'R',"draw_wall(column=%d wall=%d side=%d)",column,wall,side);*/
   
   /* This would be wrong!  The textures must be clipped, not moved.
    *   if (pend1 > pend2) pend1 = pend2;
    *   if (pstart2 < pstart1) pstart2 = pstart1;
    * Instead, I adjust top & bottom after the ceiling and floor 
    * have been drawn.  The adjustment was originally in do_walls().
    */

   pfend=MIN(pstart1,top);
   pcstart=MAX(pend2,bottom);
   
   pstart3=MAX(pend1,pstart1);
   pend3=MIN(pend2,pstart2);
   
   do_lower = (pstart1 < top) &&
     (pend1 > bottom) &&
     (pend1 - pstart1 > FIXED_EPSILON);
   do_middle = (pstart3 < top) &&
     (pend3 > bottom) &&
     (pend3 - pstart3 > FIXED_EPSILON) &&
     ldsided(ld)[side].mtex!=NULL;
   do_upper = (pstart2 < top) &&
     (pend2 > bottom) &&
     (pend2 - pstart2 > FIXED_EPSILON);


#ifdef USE_COLORMAP
   colormap=((const Pixel *)load_lump(colormap_ln));
#endif   
   
   if(bottom<pfend) {
      if(REGION_HAS_SKY_FLOOR(front))
      	draw_sky_segment(bottom, pfend, REGION_FTEX(front));
      else 
      	draw_floor_slices(active, front, bottom, pfend);
   };
   if(pcstart<top) {
      if(REGION_HAS_SKY_CEILING(front))  {
	 if(back>=0&&REGION_HAS_SKY_CEILING(back)) {
	    do_upper=0;
	    pcstart=pstart2;
	 };
	 draw_sky_segment(pcstart, top, REGION_CTEX(front));
      }
      else 
      	draw_ceiling_slices(active, front, pcstart,top);
   };
   
   /* Adjust bottom & top to avoid drawing walls over ceiling & floor.
    * This adjustment used to be in do_walls().  */
   if (bottom < pstart1) bottom = pstart1;
   if (top > pend2) top = pend2;

   /* if there's nothing more to do, skip costly ray-intersect */
   if(!do_lower&&!do_upper&&!do_middle) return; 
     
#ifdef USE_COLORMAP
   dark=fixdiv(ldsectord(ld)[front].dark,z);
   dark=DARK_ADJUST(dark);
   dark=FIXED_TO_INT(dark);
   if(dark>30) dark=30;
   colormap+=dark*256;
#endif   

   if(WALL_ISPOSTER(wall)) 
      tex_col=FIXED_TO_INT(FIXED_SCALE(wall_ray_intersection(Vx, Vy, wall),
				       ldsided(ld)[side].mtex->width));
   else {
      tex_col=FIXED_TO_INT(FIXED_SCALE(wall_ray_intersection(Vx, Vy, wall),
				       WALL_LENGTH_IN_TEXELS(wall)));
      tex_col-=WALL_LENGTH_IN_TEXELS(wall);
   };
   tex_col-=SIDE_XOFFSET_IN_TEXELS(side);
   tex_yoffset=SIDE_YOFFSET_IN_TEXELS(side);

   if(WALL_ISPOSTER(wall)) 
      scale=fixdiv(INT_TO_FIXED(ldsided(ld)[side].mtex->height),
		   REGION_CEILING(front)-REGION_FLOOR(front));
		   
   if (z <= FIXED_EPSILON)
     tex_dy = 0;
   else
     tex_dy = fixdiv(scale,FIXED_SCALE(z, view_height));
   
   if (do_lower)
      draw_wall_segment(pstart1, pend1,
		       WALL_LOUNPEGGED(wall)?
		       REGION_FLOOR(front)-REGION_CEILING(front):
		       REGION_FLOOR(front)-REGION_FLOOR(back),
		       ldsided(ld)[side].ltex,
		       tex_col,tex_dy,tex_yoffset,
		       1,
		       scale);
   if (do_middle)
     draw_wall_segment(pstart3, pend3, 
		       WALL_LOUNPEGGED(wall)?
		       0:
		       REGION_FLOOR(front)-REGION_CEILING(front),
		       ldsided(ld)[side].mtex,
		       tex_col,tex_dy,tex_yoffset,
		       !(ldline(ld)[wall].flags&LINE_TWOSIDED),
		       scale);
   
   if (do_upper)  {
      if(REGION_HAS_SKY_CEILING(front)&&back>=0&&REGION_HAS_SKY_CEILING(back)) {
#ifdef USE_COLORMAP
	 colormap=((const Pixel *)load_lump(colormap_ln));
#endif   
	 draw_sky_segment(pstart2, top, REGION_CTEX(front));
      }
      else
	 draw_wall_segment(pstart2, pend2, 
			   WALL_UPUNPEGGED(wall)?
			   REGION_CEILING(back)-REGION_CEILING(front):
			   0,
			   ldsided(ld)[side].utex,
			   tex_col,tex_dy,tex_yoffset,
			   1,
			   scale);
   };
};


static void draw_object(int on,
			int tex_column)
{
   fixed pstart, pend;
   fixed tex_y, tex_dy;
   int fb_start, fb_end;
   Pixel *fb_byte, *last_byte;
   Texture *oimage=OBJECT_IMAGE(on);
#ifdef USE_COLORMAP
   int dark;
#endif
   int fb_column;

   if(oimage==NULL||tex_column>=oimage->width) return;

   if(OBJECT_IS_MIRRORIMAGE(on)) tex_column=oimage->width-(1+tex_column);
   
   /* Project the bottom and top of the object onto the slice. */
     
   if(OBJECT_ZCENTERED(on)) 
     pstart = fixmul(z, OBJECT_Z(on)-(OBJECT_HEIGHT(on)/2 + view->height));
   else if(OBJECT_ZPEGGED(on)) 
     pstart = fixmul(z, OBJECT_Z(on)-(OBJECT_HEIGHT(on) + view->height));
#if 0
   else if(oimage->topofs>0)
     pstart = fixmul(z, OBJECT_Z(on) + (oimage->topofs<<12) - view->height);
#endif
   else  
     pstart = fixmul(z, OBJECT_Z(on) - view->height);
   pstart+=view->horizon;
   pend   = fixmul(z, OBJECT_HEIGHT(on)) + pstart;
      
   /* See if the object is visible in this column. */
   if (pstart >= top || pend <= bottom || pend - pstart < FIXED_EPSILON)
     return;

#ifdef LAZY_SPRITE_TEXTURES
   cond_load_texels(oimage,sizeof(TPixel));
#endif   

#ifdef USE_COLORMAP
   colormap=((const Pixel *)load_lump(colormap_ln));
   if(!OBJECT_GLOWS(on))  {
      dark=fixdiv(ldsectord(ld)[OBJECT_REGION(on)].dark,z);
      dark=DARK_ADJUST(dark);
      dark=FIXED_TO_INT(dark);
      if(dark>30) dark=30;
      colormap+=dark*256;
   };
#endif   

   tex_dy = fixdiv(INT_TO_FIXED(oimage->height),
		   FIXED_SCALE(pend - pstart, view_height));
   /* Clip the object slice. */
   if (pstart < bottom) {
      tex_y = fixmul(bottom - pstart, FIXED_SCALE(tex_dy, view_height));
      pstart = bottom;
   } else
     tex_y = FIXED_ZERO;
   if (pend > top)
     pend = top;
   
   fb_start = (view_height >> 1) - 1 -
     FIXED_TO_INT(FIXED_SCALE(pstart, view_height));
   fb_end   = (view_height >> 1) -
     FIXED_TO_INT_UP(FIXED_SCALE(pend, view_height));
   if(fb_start<fb_end) return;
   fb_column = view_width - rw_column - 1;

#ifdef CHK_FBROWS
   if(fb_start<0||fb_end<0||fb_start>=view_height||fb_end>=view_height)  {
      logprintf(LOG_INFO,'R',"start=%d end=%d",fb_start,fb_end);
      logfatal('R',"problems with fb coords in draw_obj");
   };
   if(tex_column<0||tex_column>=oimage->width)
     logfatal('R',"Column (%d) off edge of sprite (width=%d)",tex_column,oimage->width);
#endif
   
   fb_byte   = fb + fb_column + fb_rows[fb_start];
   last_byte = fb + fb_column + fb_rows[fb_end];

   if(OBJECT_PARTINVIS(on))
      draw_invis_slice(fb_byte, last_byte,
		       TEXTURE_COLUMN(oimage, tex_column),
		       tex_y, tex_dy,
		       oimage->log2height);
   else
      draw_trans_slice(fb_byte, last_byte,
		       TEXTURE_COLUMN(oimage, tex_column),
		       tex_y, tex_dy,
		       oimage->log2height);
};


typedef struct {
   fixed top, bottom;
   Active_wall *active;
   Active_object *active_obj;
} Saved_wall;

#define MAX_SAVED_WALLS 16

static Saved_wall transparent_walls[MAX_SAVED_WALLS];
static int free_saved_walls;

/* do_walls() draws all the walls for a screen column. */
static void do_walls(Active_wall *active, int n_active,
		     Active_object *active_obj, int n_active_obj,
		     fixed Vx, fixed Vy)
{
   const fixed height = view->height;
   Saved_wall *last_saved = transparent_walls;
   free_saved_walls=MAX_SAVED_WALLS;
   
   /*
    **   the top and bottom of the view port for this column.  Because of
    **   the restrictions placed on world geometry, the view port will
    **   always be a single interval, making clipping extremely simple.
    **   When bottom is greater than top, the view port is closed--nothing
    **   behind the current wall will be visible.
    */
   top    = FIXED_ONE_HALF;
   bottom = -FIXED_ONE_HALF;

   /**   This loop moves from front to back through the list of 'active'
    **   walls.  We stop when we reach the last wall in the list, or
    **   when the walls we've already looked at completely obscure anything
    **   behind them.
    */
   while (n_active-- > 0 && bottom < top) {

      /**   See if there are any objects in front of this wall which need
       **   to be drawn.  Since objects have transparent parts, drawing 
       **   them must be deferred until after the opaque wall slices have
       **   been drawn.  Transparent walls are handled in a similar
       **   manner later on in the loop.
       */
      while (n_active_obj > 0 && active_obj->z >= active->z && free_saved_walls) {
	 /* Record the object in the save buffer. */
	 last_saved->top = top;
	 last_saved->bottom = bottom;
	 last_saved->active_obj = active_obj;
	 last_saved->active = NULL;
	 last_saved++;
	 free_saved_walls--;
	 /* Advance through the object list. */
	 n_active_obj--;
	 active_obj++;
      }
      
      /* Set up the front and back region pointers */
      front=ldside(ld)[active->sidenum].sector;
      back=active->backsect;

      /* If this wall has not been visible before, set the visible flag 
       **   and set up the projected coordinates and deltas.
       */
      if (AWALL_UNSEEN(active)) {
	 const fixed z = active->z;
	 const fixed dz = active->dz;
	 /*logprintf(LOG_DEBUG,'R',"awall %d seen: z=%f dz=%f",active->wall,
		   FIXED_TO_FLOAT(FROM_FIX_2_30(z)),
		   FIXED_TO_FLOAT(FROM_FIX_2_30(dz)));*/
#ifdef FAST_FLATS
	 /* maxcrow and maxfrow will get inited in draw_*_slice */
	 active->mincrow=view_height+1;
	 active->maxfrow=-view_height-1;
#endif
	 active->pstart1 =
	   fixmul2_30(TO_FIX_8_24(REGION_FLOOR(front) - height), z);
	 active->pend2 =
	   fixmul2_30(TO_FIX_8_24(REGION_CEILING(front) - height), z);
	 if(back>=0) {
	    active->pend1 =
	      fixmul2_30(TO_FIX_8_24(REGION_FLOOR(back) - height), z);
	    active->pstart2 =
	      fixmul2_30(TO_FIX_8_24(REGION_CEILING(back) - height), z);
	 }
	 else  {
	    active->pend1 = active->pstart1;
	    active->pstart2 = active->pend2;
	 };
	 active->dpstart1 =
	   fixmul2_30(TO_FIX_8_24(REGION_FLOOR(front) - height), dz);
	 active->dpend2 =
	   fixmul2_30(TO_FIX_8_24(REGION_CEILING(front) - height), dz);
	 if(back>=0) {
	    active->dpend1 =
	      fixmul2_30(TO_FIX_8_24(REGION_FLOOR(back) - height), dz);
	    active->dpstart2 =
	      fixmul2_30(TO_FIX_8_24(REGION_CEILING(back) - height), dz);
	 }
	 else {
	    active->dpend1 = active->dpstart1;
	    active->dpstart2 = active->dpend2;
	 };
      }

      pstart1 = FROM_FIX_8_24(active->pstart1)+view->horizon;
      pend1 = FROM_FIX_8_24(active->pend1)+view->horizon;
      pstart2 = FROM_FIX_8_24(active->pstart2)+view->horizon;
      pend2 = FROM_FIX_8_24(active->pend2)+view->horizon;
      z = FROM_FIX_2_30(active->z);
      
      /* if we have non-opaque textures, remind ourselves to draw them later */
      if ((ldline(ld)[active->wall].flags&LINE_TWOSIDED)
	  &&ldsided(ld)[active->sidenum].mtex
	  &&free_saved_walls) {
	 last_saved->top        = top;
	 last_saved->bottom     = bottom;
	 last_saved->active     = active;
	 last_saved->active_obj = NULL;
	 last_saved++;
	 free_saved_walls--;
	 /* this must be done after saving top & bottom */
	 if (bottom < pstart1) bottom = pstart1;
	 if (top > pend2) top = pend2;
      }
      else
      	draw_wall(active, 
		  Vx, Vy);

      /* adjust bottom and top */
      /* adjustment between pend2 and pstart1 is now done in draw_wall() 
       * if the wall texture is opaque */
      if (bottom < pend1) bottom = pend1;
      if (top > pstart2) top = pstart2;

      /* if this wall is terminal, we can quit now */
      if(back<0) break;
      
      active++;
   }

   /* now check the leftover objects.  If any of them end before the
    * terminal wall (if any) they will never be drawn (assuming they're
    * not warping through the wall) and can be discarded now
    *
    * hopefully, this will result in many objects that would never
    * be drawn (far, far behind a terminating wall) being discarded
    * from the active-list early, resulting in a nice short list.
    *
    * OTOH, if it doesn't, this loop will just slow things down.
    */
   if(n_active>0)  {
      const int wend=active->endcol; 
      while(n_active_obj>0)  {
	 if(active_obj->endcol<=wend) active_obj->endcol=rw_column;
	 n_active_obj--;
	 active_obj++;
      };
   };
   
   /* Now, handle the objects and transparent walls.  last_saved points
    **   to an entry in the saved walls buffer.  Each entry consists of the
    **   the active list entry plus the top and bottom clipping info.
    */
   
   while (last_saved > transparent_walls) {
      Active_wall *active;
      Active_object *active_obj;

      last_saved--;
      
      top = last_saved->top;
      bottom = last_saved->bottom;
      active = last_saved->active;

      /* If active is not NULL, this entry is a wall;  if it is NULL,
       **    this entry is an object and active_obj is not null.
       */
      if (active != NULL) {
	 pstart1 = FROM_FIX_8_24(active->pstart1)+view->horizon;
	 pend1   = FROM_FIX_8_24(active->pend1)+view->horizon;
	 pstart2 = FROM_FIX_8_24(active->pstart2)+view->horizon;
	 pend2   = FROM_FIX_8_24(active->pend2)+view->horizon;
	 z = FROM_FIX_2_30(active->z);

	 front=ldside(ld)[active->sidenum].sector;
	 back=active->backsect;
	 
	 draw_wall(active, 
		   Vx, Vy);
	 
      } else {
	 active_obj = last_saved->active_obj;
	 z = FROM_FIX_2_30(active_obj->z);
	 draw_object(active_obj->o,
		     FIXED_TO_INT(active_obj->x));
      }
   }
};

// Local Variables:
// c-basic-offset:3
// End:
