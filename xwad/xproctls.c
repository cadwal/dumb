#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xproto.h"

/* general helpers */

#define CUR CURPROTO

#define TOG_FUNX(n) static CTLACTION(n##_tog) {inst->n^=1;};\
static CTLPRED(is_##n) {return inst->n;}
#define TOG_CTL(sn,n,f) LBUTTON(sn,n##_tog,is_##n,f)

#define N_FUNX(n) static CTLOUTPUT(out_##n) {sprintf(buf,"%d",inst->n);};\
static CTLINPUT(inp_##n) {inst->n=atoi(buf);xproto_redraw(inst);};
#define N_CTL(n,f) INPUTCTL(inp_##n,out_##n,f)

#define PHASE_N_FUNX(n) static CTLOUTPUT(out_##n) { \
			  sprintf(buf,"%d",(int)(CUR.n));};

/* Display controls */

TOG_FUNX(rotate);
TOG_FUNX(animate);

N_FUNX(curphase);
N_FUNX(currot);

static CTLACTION(advance_currot) {
   inst->currot++;
   if(inst->currot>=8) inst->currot=0;
   xproto_redraw(inst);
};
static CTLACTION(advance_curphase) {
   xproto_enter_phase(inst,CURPHASE.next);
   xproto_redraw(inst);
};

static CTLOUTPUT(out_sprtex) {
   Texture *t=find_phase_sprite(&CURPROTO,
				inst->curphase,
				inst->currot+'1');
   if(t) strcpy(buf,t->name);
   else *buf=0;
};
static CTLOUTPUT(out_sprtexsiz) {
   Texture *t=find_phase_sprite(&CURPROTO,
				inst->curphase,
				inst->currot+'1');
   if(t) sprintf(buf,"%dx%d",t->width,t->height);
   else *buf=0;
};

#define DISP_WIDTH 4
#define DISP_HEIGHT 3

static const Control disp_ctls[DISP_WIDTH*DISP_HEIGHT]={
LABELCTL("RotState:",0),
N_CTL(currot,0),
TOG_CTL("Rotate",rotate,0),
IBUTTON("Advance",advance_currot,0),

LABELCTL("Phase:",0),
N_CTL(curphase,0),
TOG_CTL("Animate",animate,0),
IBUTTON("Advance",advance_curphase,0),

LABELCTL("Sprite:",0),
OUTCTL(out_sprtex,CTLF_USEFONT2),
LABELCTL("SpriteSize:",0),
OUTCTL(out_sprtexsiz,0)

}; 

const ControlSet disp_cset[1]={{DISP_WIDTH,DISP_HEIGHT,disp_ctls}};


/* Action controls */

#define TSIGFUNC(n) static CTLACTION(sig_##n) {xproto_sendsig(inst,TS_##n);};

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

static const Control act_ctls[ACT_WIDTH*ACT_HEIGHT]={
IBUTTON("Init",sig_INIT,0),
IBUTTON("Detect",sig_DETECT,0),
IBUTTON("Fight",sig_FIGHT,0),
IBUTTON("Shoot",sig_SHOOT,0),
IBUTTON("Special",sig_SPECIAL,0),
IBUTTON("Ouch",sig_OUCH,0),
IBUTTON("Die",sig_DIE,0),
IBUTTON("Explode",sig_EXPLODE,0),
IBUTTON("Animate",sig_ANIMATE,0)
}; 

const ControlSet act_cset[1]={{ACT_WIDTH,ACT_HEIGHT,act_ctls}};


/* Choose controls (under the choose window) */

static CTLACTION(quit) {inst->want_quit=1;};
static CTLOUTPUT(id_out) {sprintf(buf,"%d",(int)(CUR.id));};

#define CHO_WIDTH 3
#define CHO_HEIGHT 1

static const Control cho_ctls[CHO_WIDTH*CHO_HEIGHT]={
LABELCTL("ID:",0),
OUTCTL(id_out,0),
IBUTTON("Quit",quit,CTLF_DANGER)
}; 

const ControlSet cho_cset[1]={{CHO_WIDTH,CHO_HEIGHT,cho_ctls}};
