
#ifndef BANNER_H
#define BANNER_H

#include "texture.h"

int init_banner(int baseline,int start,int stop,int speed);
void reset_banner(int banner);
void reset_banners(void);

void add_to_banner(int banner,Texture *t,int xoffset,int yoffset);
void add_text_to_banner(int banner,int font,const char *text,int len);
void add_str_to_banner(int banner,int font,const char *text);

void update_banners(void *fb,int ticks);

int banner_queuelen(int banner);

#endif
