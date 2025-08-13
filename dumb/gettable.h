
#ifndef GETTABLE_H
#define GETTABLE_H

#include "libdumb/gettablestruct.h"
#include "levdata.h"

void init_gettables(void);
void reset_gettables(void);
void reset_local_gettables(LevData *ld);

void cheat_gettables(LevData *ld,int plnum);

int gettable_chk_key(const LevData *ld,int plnum,int keytype);

void draw_gettables(LevData *ld,int pl,void *fb,int width,int height);
void update_gettables(LevData *ld,int ticks);

void pickup_gettable(LevData *ld,int plnum,int type,int num);

void rotate_selection(LevData *ld,int plnum,int type);
void use_selection(int type,LevData *ld,int plnum);

/* return 0 if use failed */
int use_item(LevData *ld,int player,const Gettable *gt);

#endif
