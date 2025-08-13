#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"

#include "controls.h"
#include "colour.h"
#include "disphash.h"

#define CTLWIDTH 56
#define CTLHEIGHT 24

#define LITEWIDTH 6
#define LITEHEIGHT 4
#define LITEXO (CTLWIDTH-LITEWIDTH-5)
#define LITEYO 3

#define BUFBORDX 4
#define BUFBORDY 4

Display *dpy=NULL;
int screen;
Window root;
GC ctlgc;
XFontStruct *ctlfont,*ctlfont1,*ctlfont2;

void setctlfont(XFontStruct *f) {
   if(ctlfont!=f) {
      XSetFont(dpy,ctlgc,f->fid);
      ctlfont=f;
   }
}

void init_controls(Display *d,XFontStruct *f1,XFontStruct *f2) {
   dpy=d;
   root=DefaultRootWindow(dpy);
   screen=DefaultScreen(dpy);
   ctlgc=XCreateGC(dpy,root,0,NULL);
   ctlfont1=f1;
   ctlfont2=f2;
   ctlfont=NULL;
   setctlfont(f1);
}
void reset_controls(void) {
   XFreeGC(dpy,ctlgc);
}

void get_cset_size(const ControlSet *cset,int *width,int *height) {
   *width=cset->cols*CTLWIDTH;
   *height=cset->rows*CTLHEIGHT;
}

static void draw_text(Window w,int x,int y,const char *text,int center) {
   XCharStruct xcs;
   int dirn,asc,dsc;
   int len=strlen(text);
   XTextExtents(ctlfont1,text,len,&dirn,&asc,&dsc,&xcs);
   /* figure out where to start drawing */
   if(center) x-=xcs.width/2;
   y+=asc/2;
   /* now do it */
   /*XDrawImageString(XDISP,drawable,gc,x,y,text,len);*/
   XDrawString(dpy,w,ctlgc,x,y,text,len);
}

static void draw_ctllight(const Control *ctl,const CtlInstance *ci,
			  int col,int row,
			  Window w,AppInst *inst) {
   int xo=col*CTLWIDTH,yo=row*CTLHEIGHT;
   if(ci->pressed&&controls_3d) {xo++;yo++;}
   if(ctl->f&CTLF_HASLITE) {
      XSetForeground(dpy,ctlgc,
		     ctl->pred(inst)?CTLC(LitLight):CTLC(UnlitLight));
      XFillRectangle(dpy,w,ctlgc,xo+LITEXO,yo+LITEYO,
		     LITEWIDTH,LITEHEIGHT);
      XSetForeground(dpy,ctlgc,BlackPixel(dpy,screen));
      XDrawRectangle(dpy,w,ctlgc,xo+LITEXO,yo+LITEYO,
		     LITEWIDTH,LITEHEIGHT);
   }
}

static void redraw_control(int col,int row,Window w) {
   const int xo=col*CTLWIDTH,yo=row*CTLHEIGHT;
   XClearArea(dpy,w,xo,yo,CTLWIDTH,CTLHEIGHT,True);
}

void draw_control(const Control *ctl,CtlInstance *ci,
		  int col,int row,
		  Window w,AppInst *inst,
		  int clear) {
   const int xo=col*CTLWIDTH,yo=row*CTLHEIGHT;
   /* fill background colour, if necessary */
   if(ci->pressed) {
      if(ctl->f&CTLF_DANGER)
	 XSetForeground(dpy,ctlgc,CTLC(PressedDanger));
      else
	 XSetForeground(dpy,ctlgc,CTLC(PressedCtl));
      XFillRectangle(dpy,w,ctlgc,xo,yo,CTLWIDTH,CTLHEIGHT);
   }
   else if(ctl->f&CTLF_DANGER) {
      XSetForeground(dpy,ctlgc,CTLC(UnpressedDanger));
      XFillRectangle(dpy,w,ctlgc,xo,yo,CTLWIDTH,CTLHEIGHT);
   }
   else if(clear) 
      XClearArea(dpy,w,xo,yo,CTLWIDTH,CTLHEIGHT,False);
   /* border */
   XSetForeground(dpy,ctlgc,CTLC(Border));
   XDrawRectangle(dpy,w,ctlgc,xo,yo,CTLWIDTH-1,CTLHEIGHT-1);
   /* highlights */
   if(controls_3d) {
      XPoint tl[3]={
	 {xo+1,yo+CTLHEIGHT-2},
	 {xo+1,yo+1},
	 {xo+CTLWIDTH-2,yo+1}
      };
      XPoint br[3]={
	 {xo+1,yo+CTLHEIGHT-2},
	 {xo+CTLWIDTH-2,yo+CTLHEIGHT-2},
	 {xo+CTLWIDTH-2,yo+1}
      };
      if(!ci->pressed) {
	 XSetForeground(dpy,ctlgc,(ctl->f&CTLF_DANGER)?
			CTLC(HighlightDanger):CTLC(HighlightCtl));
	 XDrawLines(dpy,w,ctlgc,ci->pressed?br:tl,3,CoordModeOrigin);
      }
      XSetForeground(dpy,ctlgc,(ctl->f&CTLF_DANGER)?
		     CTLC(LowlightDanger):CTLC(LowlightCtl));
      XDrawLines(dpy,w,ctlgc,ci->pressed?tl:br,3,CoordModeOrigin);
   }
   /* buffer space */
   if((ctl->f&CTLF_HASBUF)&&controls_3d&&!ci->pressed) {
      XSetForeground(dpy,ctlgc,ci->editing?
		     CTLC(PressedDanger):CTLC(PressedCtl));
      XFillRectangle(dpy,w,ctlgc,
		     xo+BUFBORDX,yo+BUFBORDY,
		     CTLWIDTH-1-BUFBORDX*2,CTLHEIGHT-1-BUFBORDY*2);
      /*if(controls_3d)*/ {
	 XPoint tl[3]={
	    {xo+BUFBORDX,yo+CTLHEIGHT-(2+BUFBORDY)},
	    {xo+BUFBORDX,yo+BUFBORDY},
	    {xo+CTLWIDTH-(2+BUFBORDX),yo+BUFBORDY}
	 };
      	 XSetForeground(dpy,ctlgc,ci->editing?
			CTLC(LowlightDanger):CTLC(LowlightCtl));
	 XDrawLines(dpy,w,ctlgc,tl,3,CoordModeOrigin);
      }
   }

   /* write buffer text */ 
   if(ctl->f&CTLF_HASBUF) {
      if(ctl->output&&!ci->editing) ctl->output(ci->buf,inst);
      if(ci->pressed)
	 XSetForeground(dpy,ctlgc,CTLC(PressedText));
      else
	 XSetForeground(dpy,ctlgc,CTLC(UnpressedText));
      setctlfont((ctl->f&CTLF_USEFONT2)?ctlfont2:ctlfont1);
      draw_text(w,xo+BUFBORDX+2,yo+CTLHEIGHT/2-1,ci->buf,0);
   }

   /* write name of button */
   else if(ctl->name) {
      int xc=xo+CTLWIDTH/2,yc=yo+CTLHEIGHT/2;
      if(ci->pressed&&controls_3d) {xc++;yc++;}
      if(ci->disabled) 
	 XSetForeground(dpy,ctlgc,CTLC(LowlightCtl));
      else if(ci->pressed)
	 XSetForeground(dpy,ctlgc,CTLC(PressedText));
      else
	 XSetForeground(dpy,ctlgc,CTLC(UnpressedText));
      setctlfont((ctl->f&CTLF_USEFONT2)?ctlfont2:ctlfont1);
      draw_text(w,xc,yc,ctl->name,1);
   }

   /* light */
   draw_ctllight(ctl,ci,col,row,w,inst);
}

static void ctl_edit(CSetInstance *csi,int i,
		     AppInst *inst,const char *s) {
   const Control *ctl=csi->cset->ctls+i;
   CtlInstance *ci=csi->inst+i;
   int l;
   if(!ci->editing) {
      ci->editing=1;
      if(ctl->output) ctl->output(ci->buf,inst);
      else *ci->buf=0;
   }
   l=strlen(ci->buf);
   strncat(ci->buf,s,CTLBUFMAX-l);   
}
static void ctl_edit_back(CSetInstance *csi,int i,
			  AppInst *inst) {
   //const Control *ctl=csi->cset->ctls+i;
   CtlInstance *ci=csi->inst+i;
   int l;
   if(!ci->editing) {
      ci->editing=1;
      *ci->buf=0;
   }
   else {
      l=strlen(ci->buf);
      if(l>0) ci->buf[l-1]=0;
   }
}
static void ctl_edit_done(CSetInstance *csi,int i,
			  AppInst *inst,int act) {
   const Control *ctl=csi->cset->ctls+i;
   CtlInstance *ci=csi->inst+i;
   if(ci->editing) {
      if(act&&ctl->input) ctl->input(ci->buf,inst);
      ci->editing=0;
   }
}

void release_buttons(CSetInstance *csi,AppInst *inst,int act) {
   int row,col;
   for(row=0;row<csi->cset->rows;row++) for(col=0;col<csi->cset->cols;col++) {
      const Control *ctl;
      CtlInstance *ci;
      int i=col+row*csi->cset->cols;
      int want_redraw=0;
      ctl=csi->cset->ctls+i;
      ci=csi->inst+i;
      if(ci->pressed) {
	 if(act&&ctl->action) ctl->action(inst);
	 ci->pressed=0;
	 want_redraw=1;
      }
      if((ctl->f&CTLF_HASBUF)&&ci->editing) {
	 ctl_edit_done(csi,i,inst,act);
	 want_redraw=1;
      }
      /* action might have changed the CSet that's drawn here */
      if(want_redraw) redraw_control(col,row,
				     csi->w);
   }
}

void rdlights_cseti(CSetInstance *csi,AppInst *inst) {
   int col,row,i;
   for(row=0;row<csi->cset->rows;row++) for(col=0;col<csi->cset->cols;col++) {
      i=col+row*csi->cset->cols;
      draw_ctllight(csi->cset->ctls+i,csi->inst+i,
		    col,row,
		    csi->w,inst);
   }
}

void rdoutp_cseti(CSetInstance *csi,AppInst *inst) {
   int col,row,i;
   for(row=0;row<csi->cset->rows;row++) for(col=0;col<csi->cset->cols;col++) {
      i=col+row*csi->cset->cols;
      if((csi->cset->ctls[i].f&CTLF_HASBUF)&&csi->cset->ctls[i].output)
	 draw_control(csi->cset->ctls+i,csi->inst+i,
		      col,row,
		      csi->w,inst,0);
   }
}

void rdable_cseti(CSetInstance *csi,AppInst *inst) {
   int col,row,i;
   for(row=0;row<csi->cset->rows;row++) for(col=0;col<csi->cset->cols;col++) {
      i=col+row*csi->cset->cols;
      draw_control(csi->cset->ctls+i,csi->inst+i,
		   col,row,
		   csi->w,inst,0);
   }
}

void expose_cseti(CSetInstance *csi,XExposeEvent *ev,AppInst *inst) {
   int col1,row1,col2,row2,col,row,i;
   col1=ev->x/CTLWIDTH;
   col2=(ev->x+ev->width+CTLWIDTH-1)/CTLWIDTH;
   row1=ev->y/CTLHEIGHT;
   row2=(ev->y+ev->height+CTLHEIGHT-1)/CTLHEIGHT;
   for(row=row1;row<row2;row++) for(col=col1;col<col2;col++) {
      i=col+row*csi->cset->cols;
      draw_control(csi->cset->ctls+i,csi->inst+i,
		   col,row,
		   ev->window,inst,0);
   }
}

void motev_cseti(CSetInstance *csi,XMotionEvent *ev,AppInst *inst) {
}

void butev_cseti(CSetInstance *csi,XButtonEvent *ev,AppInst *inst) {
   int col,row,i;
   const Control *ctl;
   CtlInstance *ci;
   /* find control and instance */
   col=ev->x/CTLWIDTH;
   row=ev->y/CTLHEIGHT;
   i=col+row*csi->cset->cols;
   ctl=csi->cset->ctls+i;
   ci=csi->inst+i;
   if(col<0||col>=csi->cset->cols) return;
   if(row<0||row>=csi->cset->rows) return;
   /* deal with event */
   if(ev->type==ButtonPress&&(ctl->f&CTLF_CANPRESS)&&!(ci->disabled)) {
      ci->pressed=1;
      draw_control(ctl,ci,
		   col,row,
		   csi->w,inst,1);
   }
   else if(ev->type==ButtonRelease) 
      release_buttons(csi,inst,1);
}

void keyev_cseti(CSetInstance *csi,XKeyEvent *ev,AppInst *inst) {
   int col,row,i;
   const Control *ctl;
   CtlInstance *ci;
   /* find control and instance */
   col=ev->x/CTLWIDTH;
   row=ev->y/CTLHEIGHT;
   i=col+row*csi->cset->cols;
   ctl=csi->cset->ctls+i;
   ci=csi->inst+i;
   if(col<0||col>=csi->cset->cols) return;
   if(row<0||row>=csi->cset->rows) return;
   /* deal with event */
   if(ev->type==KeyPress&&ctl->f&CTLF_HASBUF&&ctl->input) {
      /* work out what they typed */
      char str[CTLBUFLEN];
      int nchars;
      KeySym ks;
      nchars=XLookupString(ev,str,CTLBUFMAX,&ks,NULL);
      str[nchars]=0;
      /* deal with it */
      if(ks==XK_Return) ctl_edit_done(csi,i,inst,1);
      else if(ks==XK_Escape) ctl_edit_done(csi,i,inst,0);
      else if(ks==XK_BackSpace) ctl_edit_back(csi,i,inst);
      else if(ks==XK_Delete) ctl_edit_back(csi,i,inst);
      else if(nchars>0) ctl_edit(csi,i,inst,str);
      draw_control(ctl,ci,
		   col,row,
		   csi->w,inst,1);
   }
}

void crossev_cseti(CSetInstance *csi,XCrossingEvent *ev,AppInst *inst) {
   if(ev->type==LeaveNotify) release_buttons(csi,inst,0);
}


/* create and destroy ControlSet Instances */

#define CTL_BUTMASK ButtonPressMask|ButtonReleaseMask/*|ButtonMotionMask*/
#define CTL_KEYMASK KeyPressMask
#define CTL_MSCMASK ExposureMask|LeaveWindowMask
#define CTL_EVMASK CTL_BUTMASK|CTL_KEYMASK|CTL_MSCMASK

static void ctldhf(XEvent *ev,AppInst *inst,void *info) {
   switch(ev->type) {
   case(ButtonPress):
   case(ButtonRelease):
      /* button event */
      butev_cseti(info,&ev->xbutton,inst);
      break;
   case(KeyPress):
   case(KeyRelease):
      /* key event */
      keyev_cseti(info,&ev->xkey,inst);
      break;
   case(MotionNotify):
      /* pointer motion event */
      motev_cseti(info,&ev->xmotion,inst);
      break;
   case(EnterNotify):
   case(LeaveNotify):
      /* pointer leaving controls */
      crossev_cseti(info,&ev->xcrossing,inst);
      break;
   case(Expose):
      /* need to redraw */
      expose_cseti(info,&ev->xexpose,inst);
      break;
   }
}

void init_cseti(CSetInstance *csi,
		AppInst *inst,Window parent,const ControlSet *cset,int num) {
   int i,j;
   csi->w=XCreateSimpleWindow(dpy,parent,0,0,
			      8,8,
			      1,BlackPixel(dpy,screen),
			      CTLC(UnpressedCtl));
   csi->cset=cset;
   j=0;
   for(i=0;i<num;i++) {
      int k=cset[i].rows*cset[i].cols;
      if(k>j) j=k;
   }
   csi->inst=safe_calloc(j,sizeof(ControlInstance));
   add_dh(csi->w,ctldhf,inst,csi);
   XMapWindow(dpy,csi->w);
   XSelectInput(dpy,csi->w,CTL_EVMASK);
}

void free_cseti(CSetInstance *csi) {
   remove_dh(csi->w);
   XDestroyWindow(dpy,csi->w);
   safe_free(csi->inst);
}

 




