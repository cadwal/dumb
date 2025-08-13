
#ifndef XWAD_CHOOSE_H
#define XWAD_CHOOSE_H

struct AppInstance;

#define AppInst struct AppInstance

#define CHOOSEBUFSIZE 256

typedef struct {
   int nitems;
   int curitem;
   int scroll;
   int sbdragstart;
   unsigned int width,height;
   Window w,wscroll;
   void **tbl;
   const char *(*text)(int item,AppInst *inst);
   int (*find)(const char *prefix,AppInst *inst);
   void (*choose)(int item,AppInst *inst);
   void (*chcur)(int item,AppInst *inst);
} ChooseInst;

void choose_cur(ChooseInst *ci,int i,AppInst *inst);
void choose_keycmd(ChooseInst *ci,int code,AppInst *inst);
void choose_setcur(ChooseInst *ci,int i,AppInst *inst);

void init_choose(ChooseInst *ci,Window parent,AppInst *inst);

void free_choose(ChooseInst *ci);

void init_choosers(XFontStruct *f);
void reset_choosers(void);
 
#endif

