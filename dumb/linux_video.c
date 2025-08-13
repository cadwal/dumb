#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vga.h>

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"

#include "video.h"

#ifndef linux
#error Compiling linux_video.c for a non-linux system???
#endif

ConfItem video_conf[]={
   CONFI("max-pages",NULL,0,"maximum pages to use for flipping",3),
   CONFI("min-pages",NULL,0,"minimum pages to use for flipping",2),
   {NULL}
};

#define MAX_PAGES (video_conf[0].intval)
#define MIN_PAGES (video_conf[1].intval)

static int linearlen=-1,curpage=0,pagelen=0,npages=0;
static char **page=NULL,*pagev=NULL;
static int vgapage=0;
static int was_suid_root=0;

void video_preinit(void) {
   if(getuid()!=geteuid()&&!geteuid()) was_suid_root=1;
   vga_init();
   /* There is a problem: on (some?) systems using a linear framebuffer,
      we need to be superuser when vga_setlinearaddressing() is called.
      But in a suid root binary, vga_init will already have given up
      superuser status!  If this seems to be happenning, init_video
      will tell the user. */
}

void init_video(int *_width,int *_height,int *_bpp,int *_real_width) {
   /* 
    * For security reasons, we shouldn't call vga_init() here:
    * it MUST be called as the first thing in main()
    */
   if(pagev!=NULL) safe_free(pagev);
   if(page!=NULL) safe_free(page);
   pagev=NULL;
   page=safe_calloc(MAX_PAGES,sizeof(char *));

   switch(*_bpp) {
    case(4):
      switch(*_width) {
       case(1280): vga_setmode(G1280x1024x16M); break;
       case(1024): vga_setmode(G1024x768x16M); break;
       case(800): vga_setmode(G800x600x16M); break;
       case(640): vga_setmode(G640x480x16M); break;
       case(320): vga_setmode(G320x200x16M); break;
      }
      break;
    case(2):
      switch(*_width) {
       case(1280): vga_setmode(G1280x1024x64K); break;
       case(1024): vga_setmode(G1024x768x64K); break;
       case(800): vga_setmode(G800x600x64K); break;
       case(640): vga_setmode(G640x480x64K); break;
       case(320): vga_setmode(G320x200x64K); break;
      }
      break;
    case(1):
      switch(*_width) {
       case(1280): vga_setmode(G1280x1024x256); break;
       case(1024): vga_setmode(G1024x768x256); break;
       case(800): vga_setmode(G800x600x256); break;
       case(640): vga_setmode(G640x480x256); break;
       case(360): vga_setmode(G360x480x256); break;
       default:
       case(320):
	 switch(*_height) {
	  default:
	  case(400): vga_setmode(G320x400x256); break;
	  case(240): vga_setmode(G320x240x256); break;
	  case(200): vga_setmode(G320x200x256); break;
	 }
      }
      break;
    default:
      logfatal('V',"Bad BPP (%d) in init_video",*_bpp);
   }

   *_real_width=*_width=vga_getxdim();
   *_height=vga_getydim();
   
   linearlen=vga_setlinearaddressing();
   if(linearlen<0&&was_suid_root) 
      logprintf(LOG_ERROR,'V',
		"No linear framebuf? Try running as root (not just suid root)");
   pagelen=vga_getxdim()*vga_getydim()*(*_bpp);
   page[0]=vga_getgraphmem();
   if(linearlen==-1) linearlen=65536;
   npages=linearlen/pagelen;
   if(npages>MAX_PAGES) npages=MAX_PAGES;
   
   if(npages<=0)
     logprintf(LOG_FATAL,'V',"Too little video RAM, or no linear fb: linearlen=%d pagelen=%d",linearlen,pagelen);

   /*if(npages<MIN_PAGES&&pagelen<=65536) {
      npages=2;
      pagelen=65536;
      vgapage=1;
      logprintf(LOG_INFO,'V',"init_video in %s mode linearlen=%d pagelen=%d npages=%d","vga-paging",linearlen,pagelen,npages);   
   } else*/ if(npages<MIN_PAGES) {
      logprintf(LOG_INFO,'V',"init_video in %s mode linearlen=%d pagelen=%d npages=%d","copying",linearlen,pagelen,npages);
      pagev=safe_malloc(pagelen);
   } else {
      int i;
      logprintf(LOG_INFO,'V',"init_video in %s mode linearlen=%d pagelen=%d npages=%d","flipping",linearlen,pagelen,npages);
      //pagev=safe_malloc(pagelen);
      for(i=1;i<npages;i++) page[i]=page[0]+i*pagelen;
   }
   
   memset(page[0],0,linearlen);
}

void reset_video(void) {
   if(pagev!=NULL) safe_free(pagev);
   if(page!=NULL) safe_free(page);
   pagev=NULL;
   page=NULL;
   vga_setmode(TEXT);
}

#define SHRPAL 2
void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue) {
   vga_setpalette(idx,red>>SHRPAL,green>>SHRPAL,blue>>SHRPAL);
}

void *video_newframe(void) {
   if(pagev) return pagev;
   else if(vgapage) {
      vga_setpage(curpage);
      return page[0];
   }
   else return page[curpage];
}
void video_updateframe(void *v) {
   if(pagev)
      memcpy(page[curpage],v,pagelen);
   if(npages>1) {
      /*vga_waitretrace();*/
      vga_setdisplaystart(curpage*pagelen);
      curpage=(++curpage)%npages;
   }
}

void video_winstuff(const char *desc,int xdim,int ydim) {}

// Local Variables:
// c-basic-offset: 3
// c-file-offsets: ((case-label . 1) (statement-case-intro . 2))
// End:
