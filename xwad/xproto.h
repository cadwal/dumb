/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/xproto.h: XProtoThing, a program for viewing ProtoThings.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#ifndef XWAD_XPROTO_H
#define XWAD_XPROTO_H
#include "libdumbutil/fixed.h"
#include "libdumbwad/wadio.h"
#include "libdumb/prothing.h"
#include "controls.h"
#include "choose.h"

extern Display *dpy;
extern Window root;
extern int screen;

extern const ControlSet act_cset[1];
extern const ControlSet cho_cset[1];
extern const ControlSet disp_cset[1];

typedef struct AppInstance {
   ProtoThing *protos;
   LumpNum protos_ln;
   int nprotos;
   const ThingPhase *phase_tbl;
   int curphase;
   int currot;
   int phcount;
   ChooseInst chooser;
   CSetInstance choctls, dispctls, actctls;
   Window frame, wdisp;
   int min_width, min_height;
   int want_quit:1;
   int animate:1;
   int rotate:1;
} XPInstance, XProtoInstance;

#define CURPROTO (inst->protos[inst->chooser.curitem])
#define CURPHASE (inst->phase_tbl[inst->curphase])

void init_instance(XPInstance *inst);
void free_instance(XPInstance *inst);

void update_wmtitle(XPInstance *inst);
void update_intgeo(XPInstance *inst);

void redraw_wdisp(XPInstance *inst);
void xproto_redraw(XPInstance *inst);

void xproto_sendsig(XPInstance *inst, ThingSignal sig);
void xproto_enter_phase(XPInstance *inst, int ph);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
