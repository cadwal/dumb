#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <aalib.h>

#include "libdumbutil/log.h"
#include "video.h"

#define ALLOW_FASTRENDER

static const ConfEnum dither_choices[] = {
   { "none", AA_NONE },
   { "error-distribution", AA_ERRORDISTRIB },
   { "floyd-steinberg", AA_FLOYD_S },
   {NULL}
};

ConfItem video_conf[]={
#ifdef ALLOW_FASTRENDER
   CONFB("fastrender",NULL,0,"very fast (but not as perfect)"),
#else
   CONFB("fastrender",NULL,0,"<disabled at compile time>"),
#endif
   CONFI("bright",NULL,0,"brightness (0-255)", 96),
   CONFI("contrast",NULL,0,"contrast (0-255)", 96),
   /* TODO: make configuration system support floating point directly */
   CONFI("gamma",NULL,0,"gamma (1000 corresponds to 1.0)", 1000),
   CONFE("dither",NULL,0,"dithering type",AA_FLOYD_S,dither_choices),
   CONFI("randomval",NULL,0,"range of random value to add to each pixel (0-inf)", 0),
   {NULL}
};
#define cnf_fastrender (video_conf[0].intval)
#define cnf_bright (video_conf[1].intval)
#define cnf_contrast (video_conf[2].intval)
#define cnf_gamma (video_conf[3].intval)
#define cnf_dither (video_conf[4].intval)
#define cnf_randomval (video_conf[5].intval)

aa_context *aa_ctxt=NULL;
#define aa aa_ctxt

#ifdef ALLOW_FASTRENDER
static unsigned char graypal[256];
#endif
static aa_palette aapal;
static aa_renderparams aarp;

void
video_preinit(void)
{
}

void
init_video(int *width,int *height,int *bpp,int *real_width)
{
   if(*width>0 && *width<256 && *height>0 && *height<256) {
      aa_defparams.width=*width;
      aa_defparams.height=*height;
   }
   /* Parse AA-lib environment options from $AAOPTS */
   aa_parseoptions(NULL, NULL, NULL, NULL);
   aa=aa_autoinit(&aa_defparams);
   if (!aa)
      logfatal('V', "AA-lib initialisation failed");
   *bpp=1;
   *real_width=*width=aa_imgwidth(aa);
   *height=aa_imgheight(aa);
   /* warm up the renderer */
#ifdef ALLOW_FASTRENDER
   if (cnf_fastrender) 
      aa_fastrender(aa,0,0,aa_scrwidth(aa),aa_scrheight(aa));
   else
#endif
   {
      memset(&aarp,0,sizeof(aarp));
      aarp.bright=cnf_bright;
      aarp.contrast=cnf_contrast;
      aarp.gamma=cnf_gamma/1000.0;
      aarp.dither=cnf_dither;
      aarp.randomval=cnf_randomval;
      aa_renderpalette(aa,aapal,&aarp,0,0,aa_scrwidth(aa),aa_scrheight(aa));
   }
}

void 
reset_video(void)
{
   aa_close(aa);
   aa=NULL;
}

void
video_setpal(unsigned char idx,
	     unsigned char red, unsigned char green, unsigned char blue)
{
#ifdef ALLOW_FASTRENDER
   graypal[idx]=red;
   /* I guess aa_setpalette() is fast enough to be always called.  */
#endif
   aa_setpalette(aapal,idx,red,green,blue);
}

void *
video_newframe(void)
{
   return aa_image(aa);
}

void
video_updateframe(void *v)
{
#ifdef ALLOW_FASTRENDER
   if (cnf_fastrender) {
      int i=aa_imgwidth(aa)*aa_imgheight(aa);
      unsigned char *fb=aa_image(aa);
      while(i>0) {
	 i--;
	 fb[i]=graypal[fb[i]];
      }
      aa_fastrender(aa,0,0,aa_scrwidth(aa),aa_scrheight(aa));
   } else
#endif
      aa_renderpalette(aa,aapal,&aarp,0,0,aa_scrwidth(aa),aa_scrheight(aa));
   aa_flush(aa);
}

void 
video_winstuff(const char *desc,int xdim,int ydim) 
{
}

// Local Variables:
// c-basic-offset: 3
// End:
