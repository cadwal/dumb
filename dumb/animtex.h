
#ifndef ANIMTEX_H
#define ANIMTEX_H

#include "libdumb/animtexstruct.h"
#include "libdumb/texture.h"

typedef short AnimTexNum;

AnimTexNum get_animtex(const char *name);
int update_anim_state(AnimTexNum num,int seqnum,int tickspassed);
Texture *get_anim_texture(AnimTexNum num,int seqnum);
void init_animtex(void);
void reset_animtex(void);

#endif
