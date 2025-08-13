
#ifndef XTEXTURE_H
#define XTEXTURE_H

#include "dumb/texture.h"

Colormap make_xtexture_cmap(Display *dpy,Window w);
void set_xtexture_cmap(Display *dpy,Window w);
void xtexture(Display *dpy,Drawable w,Texture *t,int mirror);

#endif
