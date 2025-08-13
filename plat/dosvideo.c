#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grx20.h>
#include <sys/segments.h>
#include <sys/movedata.h>
#include <dpmi.h>
#include <errno.h>

#include "lib/log.h"
#include "video.h"
#include "lib/safem.h"

static int pagelen=0;
static void *pagev=NULL,*page0=NULL;

unsigned int bogus=0;
unsigned short fb_selector=0;

void init_video(int *width,int *height,int *bpp) {
   if(pagev!=NULL) return;
   *bpp=1;
   GrSetMode(GR_width_height_color_graphics,*width,*height,256);
   *width=GrSizeX();
   *height=GrSizeY();
   pagelen=/*nearest_pow2*/(GrSizeX()*GrSizeY()*(*bpp));
   //page0=GrCurrentContext()->gc_baseaddr[0];
   //fb_selector=GrCurrentContext()->gc_selector;
   //pagev=safe_calloc(pagelen,1);
   page0=(void *) (((unsigned)safe_malloc(65536+4096)+4096)&~4095);
   if(__djgpp_map_physical_memory(page0,65536,0x000a0000))
     logprintf(LOG_FATAL,'V',"DPMI mapping request failed (errno=%d)",errno);
   logprintf(LOG_INFO,'V',"init_video %dx%dx%d  %d bytes",*width,*height,*bpp,pagelen);
};

void reset_video(void) {
   GrSetMode(GR_80_25_text);
   if(pagev) safe_free(pagev);
   pagev=NULL;
};

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue) 
{
   GrSetColor(idx,
	      (unsigned int)(red),
	      (unsigned int)(green),
	      (unsigned int)(blue));
};

void *video_newframe(void) {return pagev?pagev:page0;};
void video_updateframe(void *v) {};

void video_preinit(void) {};

void video_winstuff(const char *desc,int xdim,int ydim) {};
