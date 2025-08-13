/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/xproctls.c: XProtoThing controls.
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

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "libdumbutil/dumb-nls.h"

#include "xproto.h"

/* general helpers */

#define CUR CURPROTO

#define TOG_FUNX(n) CTLACTION(n##_tog) {inst->n^=1;}\
CTLPRED(is_##n) {return inst->n;}
#define TOG_CTL(sn,n,f) LBUTTON(sn,n##_tog,is_##n,f)

#define N_FUNX(n) CTLOUTPUT(out_##n) {sprintf(buf,"%d",inst->n);}\
CTLINPUT(inp_##n) {inst->n=atoi(buf);xproto_redraw(inst);}
#define N_CTL(n,f) INPUTCTL(inp_##n,out_##n,f)

#define PHASE_N_FUNX(n) CTLOUTPUT(out_##n) { \
			  sprintf(buf,"%d",(int)(CUR.n));}

/* Display controls */

TOG_FUNX(rotate);
TOG_FUNX(animate);

N_FUNX(curphase);
N_FUNX(currot);

CTLACTION(advance_currot)
{
   inst->currot++;
   if (inst->currot >= 8)
      inst->currot = 0;
   xproto_redraw(inst);
}

CTLACTION(advance_curphase)
{
   xproto_enter_phase(inst, CURPHASE.next);
   xproto_redraw(inst);
}

CTLOUTPUT(out_sprtex)
{
   Texture *t = find_phase_sprite(&CURPROTO,
				  inst->curphase,
				  inst->currot + '1');
   if (t)
      strcpy(buf, t->name);
   else
      *buf = 0;
}
CTLOUTPUT(out_sprtexsiz)
{
   Texture *t = find_phase_sprite(&CURPROTO,
				  inst->curphase,
				  inst->currot + '1');
   if (t)
      sprintf(buf, "%dx%d", t->width, t->height);
   else
      *buf = 0;
}

#define DISP_WIDTH 4
#define DISP_HEIGHT 3

static const Control disp_ctls[DISP_WIDTH * DISP_HEIGHT] =
{
   LABELCTL(N_("RotState:"), 0),
   N_CTL(currot, 0),
   TOG_CTL(N_("Rotate"), rotate, 0),
   IBUTTON(N_("Advance"), advance_currot, 0),

   LABELCTL(N_("Phase:"), 0),
   N_CTL(curphase, 0),
   TOG_CTL(N_("Animate"), animate, 0),
   IBUTTON(N_("Advance"), advance_curphase, 0),

   LABELCTL(N_("Sprite:"), 0),
   OUTCTL(out_sprtex, CTLF_USEFONT2),
   LABELCTL(N_("SpriteSize:"), 0),
   OUTCTL(out_sprtexsiz, 0)
};

const ControlSet disp_cset[1] =
{
   {DISP_WIDTH, DISP_HEIGHT, disp_ctls}
};

/* Action controls */

#define TSIGFUNC(n) CTLACTION(sig_##n) {xproto_sendsig(inst,TS_##n);}

TSIGFUNC(INIT);
TSIGFUNC(DETECT);
TSIGFUNC(FIGHT);
TSIGFUNC(SHOOT);
TSIGFUNC(SPECIAL);
TSIGFUNC(OUCH);
TSIGFUNC(DIE);
TSIGFUNC(EXPLODE);
TSIGFUNC(ANIMATE);

#define ACT_WIDTH 3
#define ACT_HEIGHT 3

static const Control act_ctls[ACT_WIDTH * ACT_HEIGHT] =
{
   IBUTTON(N_("Init"), sig_INIT, 0),
   IBUTTON(N_("Detect"), sig_DETECT, 0),
   IBUTTON(N_("Fight"), sig_FIGHT, 0),
   IBUTTON(N_("Shoot"), sig_SHOOT, 0),
   IBUTTON(N_("Special"), sig_SPECIAL, 0),
   IBUTTON(N_("Ouch"), sig_OUCH, 0),
   IBUTTON(N_("Die"), sig_DIE, 0),
   IBUTTON(N_("Explode"), sig_EXPLODE, 0),
   IBUTTON(N_("Animate"), sig_ANIMATE, 0)
};

const ControlSet act_cset[1] =
{
   {ACT_WIDTH, ACT_HEIGHT, act_ctls}
};


/* Choose controls (under the choose window) */

CTLACTION(quit)
{
   inst->want_quit = 1;
}

CTLOUTPUT(id_out)
{
   sprintf(buf, "%d", (int) (CUR.id));
}

#define CHO_WIDTH 3
#define CHO_HEIGHT 1

static const Control cho_ctls[CHO_WIDTH * CHO_HEIGHT] =
{
   LABELCTL(N_("ID:"), 0),
   OUTCTL(id_out, 0),
   IBUTTON(N_("Quit"), quit, CTLF_DANGER)
};

const ControlSet cho_cset[1] =
{
   {CHO_WIDTH, CHO_HEIGHT, cho_ctls}
};

// Local Variables:
// c-basic-offset: 3
// End:
