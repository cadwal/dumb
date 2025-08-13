
#ifndef VIDEO_H
#define VIDEO_H

#include "libdumbutil/confdef.h"

extern ConfItem video_conf[];

void init_video(int *width,int *height,int *bpp,int *real_width);
void reset_video(void);

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue);

void *video_newframe(void);
void video_updateframe(void *v);

/* accessing the video subsystem on many systems requires special permissions
 * video_preinit() should do whatever initialization requires these, and
 * then release them, for security reasons.  It should not change the 
 * video mode.  It will be called first thing by main() */
void video_preinit(void);

/* set information to be used by a window manager */
void video_winstuff(const char *desc,int xdim,int ydim);

#endif
