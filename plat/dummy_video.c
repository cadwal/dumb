#include <stdarg.h>
#include <stdio.h>

#include "lib/log.h"
#include "video.h"
#include "lib/safem.h"

ConfItem video_conf[]={{NULL}};

static void *fb=NULL;

void video_preinit(void) {
};

void init_video(int *width,int *height,int *bpp,int *real_width) {
   if(fb==NULL) fb=safe_calloc((*width)*(*height),*bpp);
#ifdef FB_IN_GS /* FB_IN_GS is i386 only wizardry */
   /* make sure that gs overrides are harmless */
   asm("movw %%ds,%%ax\n\tmovw %%ax,%%gs\n\t":::"eax");
#endif   
};

void reset_video(void) {
   if(fb) safe_free(fb);
};

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue) {
};

void *video_newframe(void) {
   return fb;
};
void video_updateframe(void *v) {
};

void video_winstuff(const char *desc,int xdim,int ydim) {};

