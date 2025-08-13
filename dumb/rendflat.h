
/* this file is included by render.c */

#if defined(i386)&&!defined(DEBUG)

/* this loop could be unrolled for greater efficiency... */

static inline void _flatx_loop(fixed x,fixed y,fixed dx,fixed dy,
			       Pixel *out,Pixel *end,
			       const Pixel *cm,const TPixel *t,
			       int xmask,int ymask)
{
   asm("std\n\t"
       "\t1:\n\t"
       "andl %8,%0\n\t"
       "movl %1,%%eax\n\t"
       "andl %9,%%eax\n\t"
       "orl %0,%%eax\n\t"
       "shrl $16,%%eax\n\t"
       "subl %2,%0\n\t"
       "subl %3,%1\n\t"

#if BPP==1
       "movb (%%esi,%%eax),%%al\n\t"
       "xlatb\n\t"
       "stosb\n\t"
#elif BPP==2
       "movzbl (%%esi,%%eax),%%eax\n\t"
       "movw (%%ebx,%%eax,2),%%ax\n\t"
       "stosw\n\t"
#elif BPP==4
       "movzbl (%%esi,%%eax),%%eax\n\t"
       "movl (%%ebx,%%eax,4),%%eax\n\t"
       "stosl\n\t"
#else
#error Need a BPP value of 1, 2 or 4!       
#endif       
       "cmpl %6, %%edi\n\t"
       "jns 1b\n\t"
       "cld\n\t"
       :
       : "c" (x), "d" (y), "g" (dx), "g" (dy), "D" (out),
         "S" (t), "g" (end), "b" (cm), "g" (xmask), "g" (ymask)
       : "eax"
       );
}

#else /* i386 */

static inline void _flatx_loop(fixed x,fixed y,fixed dx,fixed dy,
			Pixel *out,Pixel *end,
			const Pixel *cm,const TPixel *t,
			int xmask,int ymask)
{
   if (dy == 0) {
       y&=ymask;
       while(out>=end) {
	   x&=xmask;
#ifdef USE_COLORMAP 
	   *(out--)=cm[t[FIXED_TO_INT(y + x)]];
#else
	   *(out--)=t[FIXED_TO_INT(y + x)];
#endif
	   x-=dx;
       }
   } else {
       while(out>=end) {
	   x&=xmask;
#ifdef USE_COLORMAP 
	   *(out--)=cm[t[FIXED_TO_INT((y&ymask) + x)]];
#else
	   *(out--)=t[FIXED_TO_INT((y&ymask) + x)];
#endif
	   x-=dx;
	   y-=dy;
       }
   }
}
#endif

static void flatx_loop(fixed x,fixed y,fixed dx,fixed dy,
			Pixel *out,Pixel *end,
			const Pixel *cm,const Texture *t) {
   switch(t->log2height) {
   case(6):
      _flatx_loop(x,y,dx,dy,out,end,cm,t->texels,0x3fffff,0xfc00000);
      break;
   case(7):
      _flatx_loop(x,y,dx,dy,out,end,cm,t->texels,0x7fffff,0x3f800000);
      break;
   }
}

/* dark ranges from 1 to 0 */

static void draw_flatx(Texture *tex,fixed height,int start_row,int end_row,
		       int column,int end_column,
		       fixed dark) 
{
   const fixed sin_x=view_constants.sin_tab[column];
   const fixed cos_x=view_constants.cos_tab[column];
   height-=view->height;   

   cond_load_texels(tex,sizeof(TPixel));

   column=view_width-(column+1);
   end_column=view_width-(end_column+1);
   start_row=(view_height/2+1)-(start_row+1);
   end_row=(view_height/2+1)-(end_row+1);

   if(start_row>=view_height) start_row=view_height-1;

   dark=fixmul(dark,height);
   dark=DARK_ADJUST(dark);
   
   while(start_row>=end_row) {
      fixed x,dx=0,y,dy=0,y1;
      Pixel *out=fb+fb_rows[start_row]+column;
      Pixel *end=fb+fb_rows[start_row]+end_column;
      unsigned int cmidx;
      
      if(FIXED_ABS(view_constants.row_view[start_row])<FIXED_EPSILON) {
	 y1=FIXED_ONE;
	 cmidx=30;
      } else {
	 y1=fixdiv(height,view_constants.row_view[start_row])<<4;
	 cmidx=fixdiv(dark,view_constants.row_view[start_row])>>16;
	 if(cmidx>30) cmidx=30;
      }
      
      x = fixmul(view_constants.view_sin - cos_x, y1) - (view->y << 4);
      y = fixmul(-view_constants.view_cos - sin_x, y1) - (view->x << 4);
      if(column>end_column) {
	 dx = fixmul(view_constants.cos_dx, y1);
	 dy = fixmul(view_constants.sin_dx, y1);
      }

      x<<=6;
      dx<<=6;

#ifdef USE_COLORMAP
      /* this silly stuff is necessary to correct for the difference in sense
         between Doom's textures and ours */
      flatx_loop(-y,x,-dy,dx,out,end,colormap+256*cmidx,tex);
#else      
      flatx_loop(-y,x,-dy,x,out,end,NULL,tex);
#endif
      
      start_row--;
   }
}

/* these fns just keep track of where we've been */

static void draw_floor_slices(Active_wall *active,
			      int sect, fixed start, fixed end) 
{   
   const int start_row = FIXED_TO_INT(FIXED_SCALE(start, view_height))+1;
   const int end_row = FIXED_TO_INT(FIXED_SCALE(end, view_height));

   if(start_row>end_row)
      ;
   else if(active->maxfrow<-view_height) {
      draw_flatx(REGION_FTEX(sect),REGION_FLOOR(sect),
		 start_row,end_row,
		 rw_column,active->endcol,
		 REGION_DARKNESS(sect));
      active->maxfrow=end_row;
   } else {
      if(start_row<active->minfrow) {
	 draw_flatx(REGION_FTEX(sect),REGION_FLOOR(sect),
		    start_row,MIN(end_row,active->minfrow-1),
		    rw_column,active->endcol,
		    REGION_DARKNESS(sect));
      }
      if(end_row>active->maxfrow) {
	 draw_flatx(REGION_FTEX(sect),REGION_FLOOR(sect),
		    MAX(start_row,active->maxfrow+1),end_row,
		    rw_column,active->endcol,
		    REGION_DARKNESS(sect));
	 active->maxfrow=end_row;
      }
   }
   active->minfrow=start_row;
}

static void draw_ceiling_slices(Active_wall *active, 
				int sect, fixed start, fixed end)
{   
   const int start_row = FIXED_TO_INT(FIXED_SCALE(start, view_height));
   const int end_row = FIXED_TO_INT(FIXED_SCALE(end, view_height));

   if(start_row>end_row);
   else if(active->mincrow>view_height) {
      draw_flatx(REGION_CTEX(sect),REGION_CEILING(sect),
		 start_row,end_row,
		 rw_column,active->endcol,
		 REGION_DARKNESS(sect));
      active->mincrow=start_row;
   }
   else {
      if(start_row<active->mincrow) {
	 draw_flatx(REGION_CTEX(sect),REGION_CEILING(sect),
		    start_row,MIN(end_row,active->mincrow-1),
		    rw_column,active->endcol,
		    REGION_DARKNESS(sect));
	 active->mincrow=start_row;
      }
      if(end_row>active->maxcrow) {
	 draw_flatx(REGION_CTEX(sect),REGION_CEILING(sect),
		    MAX(start_row,active->maxcrow+1),end_row,
		    rw_column,active->endcol,
		    REGION_DARKNESS(sect));
      }
   }
   active->maxcrow=end_row;
}
   

// Local Variables:
// c-basic-offset: 3
// End:
