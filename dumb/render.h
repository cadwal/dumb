
#ifndef RENDER_H
#define RENDER_H

#include "libdumb/view.h"
#include "levdata.h"

typedef unsigned char Pixel8;
typedef unsigned short Pixel16;
typedef unsigned int Pixel32;

#define INIT_RENDERER_SPEC(name) void name(int width,int height,int real_width,int real_height)
#define RESET_RENDERER_SPEC(name) void name(void)
#define RENDER_SPEC(name) void name(void *fb,LevData *ld,const View *v)

INIT_RENDERER_SPEC(init_renderer8);
RESET_RENDERER_SPEC(reset_renderer8);
RENDER_SPEC(render8);
INIT_RENDERER_SPEC(init_renderer16);
RESET_RENDERER_SPEC(reset_renderer16);
RENDER_SPEC(render16);
INIT_RENDERER_SPEC(init_renderer32);
RESET_RENDERER_SPEC(reset_renderer32);
RENDER_SPEC(render32);

#endif
