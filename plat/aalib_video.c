#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <aalib.h>

#include "lib/log.h"
#include "video.h"

/*#define USE_FASTRENDER*/

aa_context *aa_ctxt=NULL;
#define aa aa_ctxt

#ifdef USE_FASTRENDER
static unsigned char mypal[256];
#else
static aa_palette aapal;
static aa_renderparams aarp;
#endif

void video_preinit(void) {
};

void init_video(int *width,int *height,int *bpp) {
   if(*width>0&&*width<256&&*height>0&&*height<256) {
      aa_defparams.width=*width;
      aa_defparams.height=*height;
   };
   aa=aa_autoinit(&aa_defparams);
   *bpp=1;
   *width=aa_imgwidth(aa);
   *height=aa_imgheight(aa);
#ifdef USE_FASTRENDER 
   /* warm up the renderer */
   aa_fastrender(aa,0,0,aa_scrwidth(aa),aa_scrheight(aa));
#else
   memset(&aarp,0,sizeof(aarp));
   aarp.bright=96;
   aarp.contrast=96;
   aarp.gamma=1.0;
   aarp.dither=AA_FLOYD_S;
   aa_renderpalette(aa,aapal,&aarp,0,0,aa_scrwidth(aa),aa_scrheight(aa));
#endif
};

void reset_video(void) {
   aa_close(aa);
   aa=NULL;
};

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue) {
#ifdef USE_FASTRENDER
   mypal[idx]=red;
#else
   aa_setpalette(aapal,idx,red,green,blue);
#endif
};

void *video_newframe(void) {
   return aa_image(aa);
};
void video_updateframe(void *v) {
#ifdef USE_FASTRENDER
   int i=aa_imgwidth(aa)*aa_imgheight(aa);
   unsigned char *fb=aa_image(aa);
   while(i>0) {
      i--;
      fb[i]=mypal[fb[i]];
   };
   aa_fastrender(aa,0,0,aa_scrwidth(aa),aa_scrheight(aa));
#else
   aa_renderpalette(aa,aapal,&aarp,0,0,aa_scrwidth(aa),aa_scrheight(aa));
#endif
   aa_flush(aa);
};

void video_winstuff(const char *desc,int xdim,int ydim) {};








