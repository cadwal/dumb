
#ifndef DISPHASH_H
#define DISPHASH_H

struct AppInstance;

#define AppInst struct AppInstance

typedef void (*DispatchFunc)(XEvent *ev,AppInst *inst,void *info);

void add_dh(Window w,DispatchFunc func,AppInst *inst,void *info);
void remove_dh(Window w);
int dispatch(XEvent *ev);

void init_dh(void);

#endif
