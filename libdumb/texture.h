
#ifndef TEXTURE_H
#define TEXTURE_H

#include "libdumbwad/wadio.h"

typedef enum {
   TT_BAD,TT_WALL,TT_SPRITE,TT_FLAT,TT_PATCH,TT_FONT,TT_MISC
} TextureType;

typedef struct {
   int width,height,topofs,leftofs;
   unsigned char log2width,log2height;
   unsigned char bpp,_spare1;
   void *texels;
   unsigned int avg_texel;
   int xpix[2];
   TextureType type;
   LumpNum lumpnum;
   char opaque,alloced_texels;
   char name[10];
} Texture;

void free_texels(Texture *tex);
void load_texels(Texture *tex,int bytesperpixel);

#define cond_load_texels(t,bpp) if((t)->texels==NULL) load_texels(t,bpp)

Texture *get_flat_texture(const char *name);
Texture *get_sprite_texture(const char *name);
Texture *get_sprite(const char *name,char phase,char rot,char *mirror_ret);
Texture *get_wall_texture(const char *name);

int get_texture_num(TextureType tt,const char *name);
Texture *get_texture_bynum(TextureType tt,int num);
int count_textures(TextureType tt);

int init_font(const char *fmt,int nchars,int firstchar);
Texture *get_font_texture(int fontnum,unsigned char ch);

Texture *get_misc_texture(const char *tname);

void init_textures(void);
void reset_textures(void);

typedef void (*SetPalFunc)(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue);

void set_playpal(int i,SetPalFunc func);

#endif


