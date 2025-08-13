
#ifndef XWAD_XPROTO_H
#define XWAD_XPROTO_H
#include "lib/fixed.h"
#include "wad/wadio.h"
#include "dumb/prothing.h"
#include "controls.h"
#include "choose.h"

extern Display *dpy;
extern Window root;
extern int screen;

extern const ControlSet act_cset[1];
extern const ControlSet cho_cset[1];
extern const ControlSet disp_cset[1];

typedef struct AppInstance {
   const ProtoThing *protos;
   LumpNum protos_ln;
   int nprotos;
   const ThingPhase *phase_tbl;
   int curphase;
   int currot;
   int phcount;
   ChooseInst chooser;
   CSetInstance choctls,dispctls,actctls;
   Window frame,wdisp;
   int min_width,min_height;
   int want_quit:1;
   int animate:1;
   int rotate:1;
} XPInstance,XProtoInstance;

#define CURPROTO (inst->protos[inst->chooser.curitem])
#define CURPHASE (inst->phase_tbl[inst->curphase])

void init_instance(XPInstance *inst);
void free_instance(XPInstance *inst);

void update_wmtitle(XPInstance *inst);
void update_intgeo(XPInstance *inst);

void redraw_wdisp(XPInstance *inst);
void xproto_redraw(XPInstance *inst);

void xproto_sendsig(XPInstance *inst,ThingSignal sig);
void xproto_enter_phase(XPInstance *inst,int ph);

#endif








