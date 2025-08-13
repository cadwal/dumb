#ifndef DRAW_H
#define DRAW_H

#include "libdumb/texture.h"

void init_draw(int width,int height,int bpp,int real_width);

void draw(void *framebuf,Texture *t,int x,int y);
void draw_outline(void *framebuf,Texture *t,int x,int y);

void draw_center(void *fb,Texture *t);

void drawtext(void *fb,const char *text,int len,int font,int x,int y);
void drawstr(void *fb,const char *text,int font,int x,int y);

#endif /* DRAW_H */
