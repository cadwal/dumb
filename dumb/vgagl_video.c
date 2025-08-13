#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vga.h>
#include <vgagl.h>

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "video.h"

#ifndef linux
#error Compiling vgagl_video.c for a non-linux system???
/* Well, maybe they're trying to port to BSD or something... */
#endif

#define MAX_PAGES 2
#define MIN_PAGES 2

static int vgamode=TEXT;

static GraphicsContext physicalscreen;

void video_preinit(void) {
   vga_init();
}

void init_video(int *_width,int *_height,int *_bpp) {
   /* 
    * For security reasons, we shouldn't call vga_init() here:
    * it MUST be called as the first thing in main()
    */

   switch(*_bpp) {
    case(4):
      switch(*_width) {
       case(1280): vgamode=(G1280x1024x16M); break;
       case(1024): vgamode=(G1024x768x16M); break;
       case(800): vgamode=(G800x600x16M); break;
       case(640): vgamode=(G640x480x16M); break;
       case(320): vgamode=(G320x200x16M); break;
      }
      break;
    case(2):
      switch(*_width) {
       case(1280): vgamode=(G1280x1024x64K); break;
       case(1024): vgamode=(G1024x768x64K); break;
       case(800): vgamode=(G800x600x64K); break;
       case(640): vgamode=(G640x480x64K); break;
       case(320): vgamode=(G320x200x64K); break;
      }
      break;
    case(1):
      switch(*_width) {
       case(1280): vgamode=(G1280x1024x256); break;
       case(1024): vgamode=(G1024x768x256); break;
       case(800): vgamode=(G800x600x256); break;
       case(640): vgamode=(G640x480x256); break;
       case(360): vgamode=(G360x480x256); break;
       default:
       case(320): 
	 switch(*_height) {
	  default:
	  case(400): vgamode=(G320x400x256); break;
	  case(240): vgamode=(G320x240x256); break;
	  case(200): vgamode=(G320x200x256); break;
	 }
      }
      break;
    default:
      logfatal('V',"Bad BPP (%d) in init_video",*_bpp);
   }

   vga_setmode(vgamode);
   logprintf(LOG_INFO,'V',"VGA-linear=%x",vga_setlinearaddressing());
   gl_setcontextvga(vgamode);
   gl_getcontext(&physicalscreen);
   logprintf(LOG_INFO,'V',"GL-flipmode=%d",gl_enablepageflipping(&physicalscreen));
   gl_setcontextvgavirtual(vgamode);
   
   *_width=vga_getxdim();
   *_height=vga_getydim();
}

void reset_video(void) {
   vga_setmode(TEXT);
}

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue) {
   vga_setpalette(idx,red>>2,green>>2,blue>>2);
}

void *video_newframe(void) {
   return VBUF;
}
void video_updateframe(void *v) {
   gl_copyscreen(&physicalscreen);
}

void video_winstuff(const char *desc,int xdim,int ydim) {}

// Local Variables:
// c-basic-offset: 3
// c-file-offsets: ((case-label . 1) (statement-case-intro . 2))
// End:
