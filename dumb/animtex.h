
#ifndef ANIMTEX_H
#define ANIMTEX_H

#include "texture.h"

typedef struct {
   unsigned char seqlen;
   unsigned char myseqnum;
   char name[9];
   unsigned char flags;
   int duration,_spare;
} AnimTexTable;

#define AT_FLAT 0x0001
#define AT_SWITCH 0x0002

typedef short AnimTexNum;

AnimTexNum get_animtex(const char *name);
int update_anim_state(AnimTexNum num,int seqnum,int tickspassed);
Texture *get_anim_texture(AnimTexNum num,int seqnum);
void init_animtex(void);
void reset_animtex(void);

#endif
