#include <string.h>

#include "draw.h"

static int fb_height=200,fb_width=320,fb_bpp=1,real_width=320;

/*** drawtext ***/

void drawtext(void *fb,const char *text,int len,int font,int x,int y) {
   int i;
   for(i=0;i<len;i++) {
      Texture *t=get_font_texture(font,text[i]);
      load_texels(t,fb_bpp);
      draw(fb,t,x,y);
      x+=t->width;
   };
};

void drawstr(void *fb,const char *text,int font,int x,int y) {
   drawtext(fb,text,strlen(text),font,x,y);
};

/*** draw ***/

void init_draw(int w,int h,int b,int r) {
   fb_height=h;
   fb_width=w;
   fb_bpp=b;
   real_width=r;
};

#define DRAWSPEC(bpp,Pixel,T) \
static inline void draw##bpp(void *fb,Texture *t,int xo,int yo) {\
   int x,y,dx,dy,ix=0,iy=0;\
   dx=t->width;\
   dy=t->height;\
   if(fb_width-xo<dx) dx=fb_width-xo;\
   if(fb_height-yo<dy) dy=fb_height-yo;\
   if(xo<0) {ix=-xo;};\
   if(yo<0) {iy=-yo;yo=0;};\
   for(x=ix;x<dx;x++) {\
      const Pixel *tpix=t->texels;\
      Pixel *fpix=fb;\
      tpix+=((t->width-1-x)<<t->log2height)+t->height;\
      fpix+=x+xo+yo*real_width;\
      for(y=iy;y<dy;y++) {\
	 tpix--;\
	 if(*tpix!=-1)\
	    *fpix=T;\
	 fpix+=real_width;\
      };\
   };\
};

DRAWSPEC(1,signed char,*tpix);
DRAWSPEC(2,short,*tpix);
DRAWSPEC(4,int,*tpix);
DRAWSPEC(o1,signed char,0);
DRAWSPEC(o2,short,0);
DRAWSPEC(o4,int,0);

void draw(void *fb,Texture *t,int xo,int yo) {
   load_texels(t,fb_bpp);
   switch(fb_bpp) {
   case(1): draw1(fb,t,xo,yo); break;
   case(2): draw2(fb,t,xo,yo); break;
   case(4): draw4(fb,t,xo,yo); break;
   };
};
void draw_outline(void *fb,Texture *t,int xo,int yo) {
   load_texels(t,fb_bpp);
   switch(fb_bpp) {
   case(1): drawo1(fb,t,xo,yo); break;
   case(2): drawo2(fb,t,xo,yo); break;
   case(4): drawo4(fb,t,xo,yo); break;
   };
};

void draw_center(void *fb,Texture *t) {
   int x=(fb_width-t->width)/2,y=(fb_height-t->height)/2;
   draw(fb,t,x,y);
};


