
/* This gets included by render.c */
/* There is also an optimized intel version in slice86.c */

#define PIXEL_GUARD_VAL ((TPixel) ~0)

static inline void __draw_wall_slice(Pixel *start, Pixel *end, 
				     const TPixel *tex_base,
				     fixed tex_y, fixed tex_dy,
				     int log2_tex_height)
{
   unsigned int utex_y;

   /* As a speed optimization, tex_y and tex_dy are shifted left by several
    **   bits so that tex_y can overflow correctly without explicit
    **   and-masking.  For this to occur correctly, it is necessary to
    **   make utex_y and unsigned quantity.  This is probably non-portable,
    **   but should work with most architectures.
    */
   utex_y = (unsigned int) tex_y;

   utex_y <<= (16-log2_tex_height);
   tex_dy <<= (16-log2_tex_height);

   while (start >= end) {
#ifdef USE_COLORMAP
      *start = colormap[tex_base[utex_y >> (32-log2_tex_height)]];
#else	     
      *start = tex_base[utex_y >> (32-log2_tex_height)];
#endif	     
      utex_y += tex_dy;
      start -= fb_width;
   };
};

static void draw_wall_slice(Pixel *s, Pixel *e, const TPixel *tb,
			    fixed ty, fixed tdy,
			    int log2_tex_height)
{
   /* which cases are present defined what heights can be vertically tiled 
    * non-tiled textures can be safely drawn with a bigger log2_tex_height
    */
   switch (log2_tex_height) {
    case 0:
    case 1:
    case 2:
    case 3: __draw_wall_slice(s, e, tb, ty, tdy,  3); break;
    case 4: __draw_wall_slice(s, e, tb, ty, tdy,  4); break;
    case 5: __draw_wall_slice(s, e, tb, ty, tdy,  5); break;
    case 6: __draw_wall_slice(s, e, tb, ty, tdy,  6); break;
    case 7: __draw_wall_slice(s, e, tb, ty, tdy,  7); break;
    case 8: __draw_wall_slice(s, e, tb, ty, tdy,  8); break;
    case 9: __draw_wall_slice(s, e, tb, ty, tdy,  9); break;
    case 10: __draw_wall_slice(s, e, tb, ty, tdy,  10); break;
   }
}


static inline void __draw_transparent_slice(Pixel *start, Pixel *end, 
					    const TPixel *tex_base,
					    fixed tex_y, fixed tex_dy, 
					    int log2_tex_height)
{
   unsigned int utex_y;
   Pixel p;

   /* As a speed optimization, tex_y and tex_dy are shifted left by several
    **   bits so that tex_y can overflow correctly without explicit
    **   and-masking.  For this to occur correctly, it is necessary to
    **   make utex_y and unsigned quantity.  This is probably non-portable,
    **   but should work with most architectures.
    *
    * tosi@stekt.oulu.fi: 
    **   I recall the latest C standard saying that unsigned integers
    **   always wrap with no error at the end of their range.  
    **   (And <limits.h> gives that range.)
    */
   utex_y = (unsigned int) tex_y;
   
   utex_y <<= (16-log2_tex_height);
   tex_dy <<= (16-log2_tex_height);
   
   while (start >= end) {
      p = tex_base[utex_y >> (32-log2_tex_height)];
      if (p != PIXEL_GUARD_VAL)
#ifdef USE_COLORMAP
	*start = colormap[p];
#else	     
        *start = p;
#endif	     
      utex_y += tex_dy;
      start -= fb_width;
   };
};

static void draw_trans_slice(Pixel *s, Pixel *e, const TPixel *tb,
			     fixed ty, fixed tdy,
			     int log2_tex_height)
{
   switch (log2_tex_height) {
    case 0:
    case 1:
    case 2:
    case 3: __draw_transparent_slice(s, e, tb, ty, tdy,  3); break;
    case 4: __draw_transparent_slice(s, e, tb, ty, tdy,  4); break;
    case 5: __draw_transparent_slice(s, e, tb, ty, tdy,  5); break;
    case 6: __draw_transparent_slice(s, e, tb, ty, tdy,  6); break;
    case 7: __draw_transparent_slice(s, e, tb, ty, tdy,  7); break;
    case 8: __draw_transparent_slice(s, e, tb, ty, tdy,  8); break;
    case 9: __draw_transparent_slice(s, e, tb, ty, tdy,  9); break;
    case 10: __draw_transparent_slice(s, e, tb, ty, tdy,  10); break;
   };
};

static void draw_invis_slice(Pixel *start, Pixel *end, 
					    const TPixel *tex_base,
					    fixed tex_y, fixed tex_dy, 
					    int log2_tex_height)
{
   unsigned int utex_y;
   Pixel p;

   utex_y = (unsigned int) tex_y;
   
   utex_y <<= (16-log2_tex_height);
   tex_dy <<= (16-log2_tex_height);
   
   while (start >= end) {
      p = tex_base[utex_y >> (32-log2_tex_height)];
#if BPP==1
      if (p != PIXEL_GUARD_VAL)
	*start=invismap[*start];
#elif BPP==2
      if (p != PIXEL_GUARD_VAL)
	*start&=0x0eff;
#elif BPP==4
      if (p != PIXEL_GUARD_VAL)
	*start&=0x0000ffff;
#endif
      utex_y += tex_dy;
      start -= fb_width;
   };
};


