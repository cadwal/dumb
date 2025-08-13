#include <config.h>

#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "controls.h"
#include "choose.h"
#include "colour.h"
#include "disphash.h"

#define SBAR_WIDTH 16

#define SLOT_HEIGHT (font->ascent+font->descent)
#define BSCR_HEIGHT (ci->height/3)
#define FSCR_HEIGHT (ci->height/3+SLOT_HEIGHT)

#define SLOT_START(i) ((i)*SLOT_HEIGHT-ci->scroll)

static GC gc,sbgc;
static XFontStruct *font;

void init_choosers(XFontStruct *f) {
   sbgc=XCreateGC(dpy,root,0,NULL);
   gc=XCreateGC(dpy,root,0,NULL);
   font=f;
   XSetFont(dpy,gc,f->fid);
}
void reset_choosers(void) {
   XFreeGC(dpy,gc);
   XFreeGC(dpy,sbgc);
}

void choose_cur(ChooseInst *ci,int i,AppInst *inst) {
   if(ci->choose)
      ci->choose(i,inst);
}

static void draw_item(ChooseInst *ci,AppInst *inst,int i) {
   int y=SLOT_START(i);
   const char *s;
   if(y<-SLOT_HEIGHT||y>=ci->height) return;
   s=ci->text(i,inst);
   if(i==ci->curitem) {
      XSetForeground(dpy,gc,CTLC(ChooseCurBg));
      XFillRectangle(dpy,ci->w,gc,0,y,ci->width,SLOT_HEIGHT);
      XSetForeground(dpy,gc,CTLC(ChooseCurFg));
      XSetBackground(dpy,gc,CTLC(ChooseCurBg));
   }
   XDrawImageString(dpy,ci->w,gc,0,y+font->ascent,s,strlen(s));
   if(i==ci->curitem) {
      XSetForeground(dpy,gc,CTLC(ChooseFg));
      XSetBackground(dpy,gc,CTLC(ChooseBg));
   }
}

static void redraw(ChooseInst *ci,AppInst *inst,int y1,int y2) {
   int i;
   y1+=ci->scroll;
   y2+=ci->scroll;
   y1-=SLOT_HEIGHT-1;
   y1/=SLOT_HEIGHT;
   y2+=SLOT_HEIGHT-1;
   y2/=SLOT_HEIGHT;
   if(y1>=ci->nitems) return;
   if(y1<0) y1=0;
   if(y2>=ci->nitems) y2=ci->nitems-1;
   if(y2<0) return;
   for(i=y1;i<=y2;i++)
      draw_item(ci,inst,i);
}

static void movescroll(ChooseInst *ci,int i) {
   ci->scroll+=i;
   if(ci->scroll<0) ci->scroll=0;
   if(ci->scroll>=ci->nitems*SLOT_HEIGHT-ci->height) 
      ci->scroll=ci->nitems*SLOT_HEIGHT-ci->height;
   XClearArea(dpy,ci->wscroll,0,0,0,0,True);
   XClearArea(dpy,ci->w,0,0,0,0,True);
}

#define setcur choose_setcur
void choose_setcur(ChooseInst *ci,int i,AppInst *inst) {
   int y,did_redraw=0;
   if(i<0) i=0;
   if(i>=ci->nitems) i=ci->nitems-1;
   y=SLOT_START(i);
   if(y<0) {
      movescroll(ci,y-BSCR_HEIGHT);
      did_redraw=1;
   }
   else if(y>=ci->height-SLOT_HEIGHT) {
      movescroll(ci,y+FSCR_HEIGHT-ci->height);
      did_redraw=1;
   }
   if(!did_redraw) {
      XClearArea(dpy,ci->w,0,SLOT_START(i),
		 ci->width,SLOT_HEIGHT,True);
      XClearArea(dpy,ci->w,0,SLOT_START(ci->curitem),
		 ci->width,SLOT_HEIGHT,True);
   }
   ci->curitem=i;
   if(ci->chcur) ci->chcur(i,inst);
}

static void butsel(ChooseInst *ci,int y,AppInst *inst) {
   y+=ci->scroll;
   y/=SLOT_HEIGHT;
   if(y==ci->curitem) choose_cur(ci,y,inst);
   else setcur(ci,y,inst);
}

void choose_keycmd(ChooseInst *ci,int code,AppInst *inst) {
   switch(code) {
   case(XK_KP_8):
   case(XK_KP_Up):
   case(XK_Up):
      setcur(ci,ci->curitem-1,inst);
      break;

   case(XK_KP_2):
   case(XK_KP_Down):
   case(XK_Down):
      setcur(ci,ci->curitem+1,inst);
      break;

   case(XK_Return):
      choose_cur(ci,ci->curitem,inst);
      break;
   case(XK_Escape):
      choose_cur(ci,-1,inst);
      break;
      
   case(XK_KP_9):
   case(XK_KP_Page_Up):
   case(XK_Page_Up):
      setcur(ci,ci->curitem-ci->height/SLOT_HEIGHT,inst);
      break;
   case(XK_KP_3):
   case(XK_KP_Page_Down):
   case(XK_Page_Down):
      setcur(ci,ci->curitem+ci->height/SLOT_HEIGHT,inst);
      break;
   }
}
      
static void chksize(ChooseInst *ci) {
   int frame_x,frame_y;
   unsigned int frame_depth,frame_border;
   Window frame_root;
   XGetGeometry(dpy,ci->w,&frame_root,
		&frame_x,&frame_y,&ci->width,&ci->height,
		&frame_border,&frame_depth);
   XMoveResizeWindow(dpy,ci->wscroll,
		     ci->width-SBAR_WIDTH,0,
		     SBAR_WIDTH,ci->height-2);
}

#define CH_BUTMASK ButtonPressMask
#define CH_KEYMASK KeyPressMask
#define CH_MSCMASK ExposureMask|StructureNotifyMask
#define CH_EVMASK CH_BUTMASK|CH_KEYMASK|CH_MSCMASK

#define SC_EVMASK ExposureMask|ButtonPressMask|ButtonMotionMask

static void choosedhf(XEvent *ev,AppInst *inst,void *info) {
   switch(ev->type) {
   case(ButtonPress):
      /* button event */
      butsel(info,ev->xbutton.y,inst);
      break;
   case(KeyPress):
      {
	 KeySym key=XLookupKeysym(&ev->xkey,0);
	 choose_keycmd(info,key,inst);
      }
      break;
   case(EnterNotify):
   case(LeaveNotify):
      /* pointer leaving controls */
      /* crossev_cseti(info,&ev->xcrossing,inst); */
      break;
   case(ConfigureNotify):
      chksize(info);
      break;
   case(Expose):
      /* need to redraw */
      redraw(info,inst,ev->xexpose.y,ev->xexpose.y+ev->xexpose.height);
      break;
   }
}

static void myrect(Drawable d,GC gc,int x,int y,int dx,int dy) {
   /* The initializer elements here aren't computable at load time
    * -> this isn't ANSI C */
   XPoint tl[3]={
      {x,y+dy-1},
      {x,y},
      {x+dx-1,y}
   };
   XPoint br[3]={
      {x,y+dy-1},
      {x+dx-1,y+dy-1},
      {x+dx-1,y}
   };
   /* background */
   XSetForeground(dpy,gc,CTLC(UnpressedCtl));
   XFillRectangle(dpy,d,gc,x,y,dx,dy);
   /* highlights */
   if(!controls_3d) return;
   XSetForeground(dpy,gc,CTLC(HighlightCtl));
   XDrawLines(dpy,d,gc,tl,3,CoordModeOrigin);
   XSetForeground(dpy,gc,CTLC(LowlightCtl));
   XDrawLines(dpy,d,gc,br,3,CoordModeOrigin);
}

static void thumbshape(ChooseInst *ci,int *start,int *end) {
   int dy=ci->nitems*SLOT_HEIGHT;
   int y1=ci->scroll,y2=ci->scroll+ci->height;
   y1*=ci->height-2;
   y2*=ci->height-2;
   y1/=dy;
   y2/=dy;
   if(y2-y1<8) y2=y1+8;
   *start=y1;
   *end=y2;
}

static void draw_sb(ChooseInst *ci) {
   int y1,y2;
   thumbshape(ci,&y1,&y2);
   myrect(ci->wscroll,sbgc,
	  1,y1,
	  SBAR_WIDTH-2,y2-y1);
}

static void sb_button(ChooseInst *ci,int y) {
   int y1,y2;
   thumbshape(ci,&y1,&y2);
   /* are they clicking on the thumb? */
   if(y>=y1&&y<=y2) ci->sbdragstart=y;
   else movescroll(ci,((y-y1)*ci->nitems*SLOT_HEIGHT)/(ci->height-2));
}

static void sb_motion(ChooseInst *ci,int y) {
   movescroll(ci,((y-ci->sbdragstart)*ci->nitems*SLOT_HEIGHT)/(ci->height-2));
   ci->sbdragstart=y;
}

static void scrolldhf(XEvent *ev,AppInst *inst,void *info) {
   switch(ev->type) {
   case(ButtonPress):
      /* button event */
      sb_button(info,ev->xbutton.y);
      break;
   case(MotionNotify):
      /* pointer motion event */
      sb_motion(info,ev->xmotion.y);
      break;
   case(Expose):
      /* need to redraw */
      draw_sb(info);
      break;
   }
}

void init_choose(ChooseInst *ci,Window parent,AppInst *inst) {
   memset(ci,0,sizeof(sizeof(ChooseInst)));
   ci->w=XCreateSimpleWindow(dpy,parent,0,0,
			     ci->width=64,ci->height=64,
			     1,BlackPixel(dpy,screen),
			     CTLC(ChooseBg));
   XSelectInput(dpy,ci->w,CH_EVMASK);
   add_dh(ci->w,choosedhf,inst,ci);
   ci->wscroll=XCreateSimpleWindow(dpy,ci->w,0,0,
				   8,8,
				   1,BlackPixel(dpy,screen),
				   CTLC(PressedCtl));
   XSelectInput(dpy,ci->wscroll,SC_EVMASK);
   add_dh(ci->wscroll,scrolldhf,inst,ci);
   XMapWindow(dpy,ci->wscroll);
   XSetForeground(dpy,gc,CTLC(ChooseFg));
   XSetBackground(dpy,gc,CTLC(ChooseBg));
   chksize(ci);
}

void free_choose(ChooseInst *ci) {
   /* destroy scrollbar, if we have one */
   if(ci->wscroll!=None) {
      remove_dh(ci->wscroll);
      XDestroyWindow(dpy,ci->wscroll);
   }
   ci->wscroll=None;
   /* destroy main window */
   if(ci->w!=None) {
      remove_dh(ci->w);
      XDestroyWindow(dpy,ci->w);
   }
   ci->w=None;
}

// Local Variables:
// c-basic-offset: 3
// End:
