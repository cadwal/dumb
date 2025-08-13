/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/linuxfb_video.c: linux framebuffer video driver.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/types.h>
#include <linux/fb.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "video.h"

ConfItem video_conf[] =
{
   CONFS("fb-dev", NULL, 0, "framebuffer device special", "/dev/fb0"),
   CONFB("no-check-visual", NULL, 0, "skip visual sanity check"),
   CONFITEM_END
};

#define cnf_devname (video_conf[0].strval)
#define cnf_nocheck (video_conf[1].intval)

const char video_dep_name[] = "lfbdumb"; /* for --version */

/* page flipping stuff */
#define MAX_PAGES 3
static int curpage=0,npages=0,pageysep=0;
static void *page[MAX_PAGES];
static int pageyofs[MAX_PAGES];

/* pointer to start of mmaped framebuffer */
static void *fb = NULL;

/* descriptor of open fb device */
static int fd = -1;

/* screen info structs */
static struct fb_fix_screeninfo fsi;
static struct fb_var_screeninfo vsi;

/* colourmap */
static struct fb_cmap *cmap=NULL,*saved_cmap=NULL;

/* non-zero if colourmap is in need of update */
static int cmap_needs_update=0;

#define USE_CMAP (fsi.visual==FB_VISUAL_PSEUDOCOLOR)
#define CMAP_LEN 256

/* allocate a colourmap with malloc */
static struct fb_cmap *alloc_cmap(void) 
{
   struct fb_cmap *cmap;
   cmap=safe_calloc(1,sizeof(*cmap));
   cmap->start=0;
   cmap->len=CMAP_LEN;
   cmap->red=safe_calloc(CMAP_LEN,sizeof(cmap->red[0]));
   cmap->green=safe_calloc(CMAP_LEN,sizeof(cmap->green[0]));
   cmap->blue=safe_calloc(CMAP_LEN,sizeof(cmap->blue[0]));
   cmap->transp=NULL;
   return cmap;
}

/* free a cmap */
static void free_cmap(struct fb_cmap *cmap) 
{
   safe_free(cmap->red);
   safe_free(cmap->green);
   safe_free(cmap->blue);
   safe_free(cmap);
};

/* read and write cmap via ioctl */
static void get_cmap(struct fb_cmap *cmap)
{
   if(ioctl(fd,FBIOGETCMAP,cmap)) 
      logfatal('V',"%s failed on %s","FBIOGETCMAP",cnf_devname);
}
static void put_cmap(struct fb_cmap *cmap)
{
   if(ioctl(fd,FBIOPUTCMAP,cmap)) 
      logfatal('V',"%s failed on %s","FBIOPUTCMAP",cnf_devname);
}

/* read screen info into fsi and vsi */
static void get_screen_infos(void) 
{
   if(ioctl(fd,FBIOGET_FSCREENINFO,&fsi)) 
      logfatal('V',"%s failed on %s","FBIOGET_FSCREENINFO",cnf_devname);
   if(ioctl(fd,FBIOGET_VSCREENINFO,&vsi)) 
      logfatal('V',"%s failed on %s","FBIOGET_VSCREENINFO",cnf_devname);
};

/* store panning info */
static void pan_vsi(void) 
{
   if(ioctl(fd,FBIOPAN_DISPLAY,&vsi)) 
      logfatal('V',"%s failed on %s","FBIOPAN_DISPLAY",cnf_devname);
};

void
video_preinit(void)
{
}

void
init_video(int *width, int *height, int *bpp, int *real_width)
{
   int i;
   int fbtype_insane=0;

   /* open fb device */
   fd=open(cnf_devname,O_RDWR);
   if (fd < 0) 
      logfatal('V',"couldn't open framebuffer device %s",cnf_devname);

   /* retrieve info about fb via ioctl */
   get_screen_infos();
   logprintf(LOG_INFO,'V',
	     "framebuffer is %s, %dK video memory @ 0x%lx",
	     fsi.id,fsi.smem_len/1024,fsi.smem_start);

   /* check fb type and visual */
   if(fsi.type!=FB_TYPE_PACKED_PIXELS) {
      logprintf(LOG_ERROR,'V',
		"framebuffer is not linear (type=%d)",
		fsi.type);
      fbtype_insane=1;
   };
   if(vsi.bits_per_pixel==8&&fsi.visual!=FB_VISUAL_PSEUDOCOLOR) {
      logprintf(LOG_ERROR,'V',
		"framebuffer is 8bit, but not pseudocolor (visual=%d)",
		fsi.visual);
      fbtype_insane=1;
   };
   if(vsi.bits_per_pixel%8) {
      logprintf(LOG_ERROR,'V',
		"bits per pixel is not a multiple of 8 (bpp=%d visual=%d)",
		vsi.bits_per_pixel,fsi.visual);
      fbtype_insane=1;
   };
   if(fbtype_insane) {
      if(cnf_nocheck) 
	 logprintf(LOG_INFO,'V',
		   "WARNING: this framebuffer is probably not supported");
      else
	 logfatal('V',
		  "(run with --vid-no-check-visual to try anyway)");
   };

   /* fill in info from vsi */
   *width=vsi.xres;
   *height=vsi.yres;
   *bpp=vsi.bits_per_pixel/8;
   *real_width=vsi.xres_virtual;

   /* init paging */
   pageysep=vsi.xres;
   npages=(fsi.smem_len/fsi.line_length)/pageysep;
   curpage=0;
   if(npages>MAX_PAGES) npages=MAX_PAGES;
   logprintf(LOG_INFO,'V',"page height=%d lines npages=%d",pageysep,npages);
   if(npages<1) logfatal('V',"not enough video memory for even one page?");
   if(npages<2) logfatal('V',"not enough video memory for page flipping");

   /* map fb into our address space */
   fb = mmap((caddr_t)0, fsi.smem_len, PROT_READ | PROT_WRITE,
	       MAP_SHARED, fd, (off_t)0);
   if(!fb)
      logfatal('V',"couldn't mmap framebuffer device %s",cnf_devname);

   /* calculate start (in memory, and on virtual screen) of each page */
   page[0]=fb;
   pageyofs[0]=0;
   for(i=1;i<npages;i++) {
      pageyofs[i]=pageyofs[i-1]+pageysep;
      page[i]=((char *)fb)+fsi.line_length*pageyofs[i];
   };

   /* build colourmap */
   if(USE_CMAP) {
      cmap=alloc_cmap();
      saved_cmap=alloc_cmap();
      get_cmap(cmap);
      get_cmap(saved_cmap);
   };
}

void
reset_video(void)
{
   /* free colourmap */
   if(cmap) {
      free_cmap(cmap);
      cmap=NULL;
   };

   /* restore and free saved colourmap */
   if(saved_cmap) {
      put_cmap(saved_cmap);
      free_cmap(saved_cmap);
      saved_cmap=NULL;
   };

   /* unmap framebuffer */
   if(fb) munmap(fb, fsi.smem_len);
   fb=NULL;

   /* and close it */
   close(fd);
   fd=-1;
}

void
video_setpal(unsigned char idx,
	     unsigned char red, unsigned char green, unsigned char blue)
{
   if(cmap) {
      cmap->red[idx]=((short)red)<<8;
      cmap->green[idx]=((short)green)<<8;
      cmap->blue[idx]=((short)blue)<<8;
      cmap_needs_update=1;
   };
}

void *
video_newframe(void)
{
   return page[curpage];
}

void
video_updateframe(void *v)
{
   int pripage;

   /* update colourmap */
   if(cmap&&cmap_needs_update) put_cmap(cmap);

   /* move on to next page */
   if(++curpage>=npages) curpage=0;

   /* point video hardware at next page after this */
   pripage=curpage;
   if(++pripage>=npages) pripage=0;
   vsi.xoffset=0;
   vsi.yoffset=pageyofs[pripage];
   pan_vsi();
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
}

// Local Variables:
// c-basic-offset: 3
// End:
