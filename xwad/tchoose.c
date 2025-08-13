#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "xtexture.h"
#include "controls.h"
#include "choose.h"
#include "colour.h"
#include "disphash.h"
#include "xwad.h"

#define scrn DefaultScreen(dpy)
#define rootw RootWindow(dpy,scrn)
#define visual DefaultVisual(dpy,scrn)

static void preview(XWadInstance *inst) {
   Texture *t;
   char buf[32];
   xtexture(dpy,inst->tch_preview,
	    t=get_texture_bynum(inst->tch_type,inst->tch.curitem),0);
   sprintf(buf,"%dx%d",t->width,t->height);
   XDrawImageString(dpy,inst->tch_preview,msggc,0,8,buf,strlen(buf));
}

static const char *text(int item,AppInst *inst) {
   Texture *t=get_texture_bynum(inst->tch_type,item);
   return t->name;
}
static int find(const char *prefix,AppInst *inst) {
   return 0;
}
static void chcur(int item,AppInst *inst) {
   XClearArea(dpy,inst->tch_preview,0,0,0,0,True);
}
static void choose(int item,AppInst *inst) {
   Texture *t;
   if(item>=0) {
      /* trouble: this only sets the cur-selected wall's texture
	 any other selected walls don't get it */
      t=get_texture_bynum(inst->tch_type,item);
      if(inst->tch_buf&&t)
	 strncpy(inst->tch_buf,t->name,8);
      inst->tch_buf=NULL;
      rdoutp_cseti(&inst->modectls,inst);
   }
   XUnmapWindow(dpy,inst->tch_frame);
}


#define XSPACE 12
#define YSPACE 8
static void setwmstuff(XWadInstance *inst,int set) {
   char *argv[2] = { "XWad", NULL };
   static char buf[64];
   char *win_name = buf;
   XTextProperty w_name_prop, i_name_prop;
   XSizeHints size_hints;
   XClassHint class_hint={"xwad","XWad"};
     
   int ctl_width,ctl_height;
   get_cset_size(inst->tchctls.cset,&ctl_width,&ctl_height);

   switch(inst->tch_type) {
   case(TT_FLAT): strcpy(buf,"Flat"); break;
   case(TT_WALL): strcpy(buf,"Wall"); break;
   case(TT_SPRITE): strcpy(buf,"Sprite"); break;
   case(TT_PATCH): strcpy(buf,"Patch"); break;
   default: strcpy(buf,"???"); break;
   }
   strcat(buf," Texture Browser");

   XStringListToTextProperty(&win_name, 1, &w_name_prop);
   XStringListToTextProperty(&win_name, 1, &i_name_prop);
   
   size_hints.flags = PMinSize;
   size_hints.width= size_hints.min_width = size_hints.max_width =
      ctl_width+64+XSPACE*3;
   size_hints.height= size_hints.min_height = size_hints.max_height =
      ctl_height+64+YSPACE*3;
   
   XSetWMProperties(dpy, inst->tch_frame, 
		    &w_name_prop, &i_name_prop, 
		    argv, 1,
		    &size_hints, NULL, &class_hint);
   if(set) XResizeWindow(dpy,inst->tch_frame,
			 size_hints.min_width,size_hints.min_height);
}
static void chksize(AppInst *inst) {
   int frame_width,frame_height,ctl_width,ctl_height;
   int frame_x,frame_y,frame_depth,frame_border;
   Window frame_root;
   XGetGeometry(dpy,inst->tch_frame,&frame_root,
		&frame_x,&frame_y,&frame_width,&frame_height,
		&frame_border,&frame_depth);
   get_cset_size(inst->tchctls.cset,&ctl_width,&ctl_height);
   
   /* place choose widget */
   XMoveResizeWindow(dpy,inst->tch.w,
		     XSPACE,YSPACE,
		     ctl_width,frame_height-YSPACE*3-ctl_height);

   /* place buttons */
   XMoveResizeWindow(dpy,inst->tchctls.w,
		     XSPACE,frame_height-YSPACE-ctl_height,
		     ctl_width,ctl_height);

   /* place preview window */
   XMoveResizeWindow(dpy,inst->tch_preview,
		     XSPACE*2+ctl_width,YSPACE,
		     frame_width-XSPACE*3-ctl_width,frame_height-YSPACE*2);

}
static void framedhf(XEvent *ev,AppInst *inst,void *info) {
   switch(ev->type) {
   case(KeyPress):
      {
	 KeySym key=XLookupKeysym(&ev->xkey,0);
	 choose_keycmd(&inst->tch,key,inst);
      }
      break;
   case(ConfigureNotify):
      chksize(inst);
      break;
   }
}

static void prevdhf(XEvent *ev,AppInst *inst,void *info) {
   if(ev->type==Expose&&inst->tch_do_preview) {
      XEvent ev2;
      /* make sure that we're up to date first */
      while(XCheckWindowEvent(dpy,inst->tch_frame,
			      KeyPressMask,&ev2)) dispatch(&ev2);
      while(XCheckWindowEvent(dpy,inst->tch.w,
			      KeyPressMask|ButtonPressMask,&ev2)) 
	 dispatch(&ev2);
      /* and then trash any future exposes */
      while(XCheckWindowEvent(dpy,inst->tch_preview,
			      ExposureMask,&ev2));
      /* now redraw */
      preview(inst);
   }
}

static CTLACTION(act_ok) {choose_cur(&inst->tch,inst->tch.curitem,inst);}
static CTLACTION(act_cancel) {choose_cur(&inst->tch,-1,inst);}
static CTLACTION(tog_prev) {
   inst->tch_do_preview=!inst->tch_do_preview;
   XClearArea(dpy,inst->tch_preview,0,0,0,0,True);
}
static CTLPRED(is_prev) {return inst->tch_do_preview;}
static const Control tch_ctl[3]={
IBUTTON("OK!",act_ok,0),
IBUTTON("Cancel",act_cancel,CTLF_DANGER),
LBUTTON("Preview",tog_prev,is_prev,0)
};
static const ControlSet tch_ctls={3,1,tch_ctl};

void init_tchoose(XWadInstance *inst) {
   /* init textures */
   init_textures();
   /* set safe/default values */
   inst->tch_buf=NULL;
   inst->tch_type=TT_BAD;
   inst->tch_do_preview=1;
   /* create frame */
   inst->tch_frame=XCreateSimpleWindow(dpy,root,0,0,
			     8,8,
			     1,BlackPixel(dpy,screen),
			     CTLC(Background));
   XSelectInput(dpy,inst->tch_frame,KeyPressMask|StructureNotifyMask);
   add_dh(inst->tch_frame,framedhf,inst,NULL);
   /* create preview */
   inst->tch_preview=XCreateSimpleWindow(dpy,inst->tch_frame,0,0,
			     8,8,
			     1,BlackPixel(dpy,screen),
			     BlackPixel(dpy,screen));
   set_xtexture_cmap(dpy,inst->tch_preview);
   XSelectInput(dpy,inst->tch_preview,ExposureMask);
   add_dh(inst->tch_preview,prevdhf,inst,NULL);
   XMapWindow(dpy,inst->tch_preview);
   /* create controls */
   init_cseti(&inst->tchctls,inst,inst->tch_frame,&tch_ctls,1);
   /* create chooser widget */
   init_choose(&inst->tch,inst->tch_frame,inst);
   XMapWindow(dpy,inst->tch.w);
   inst->tch.text=text;
   inst->tch.find=find;
   inst->tch.choose=choose;
   inst->tch.chcur=chcur;
   /* */
   setwmstuff(inst,1);
}

void do_tchoose(XWadInstance *inst,TextureType tt,char *buf) {
   inst->tch_buf=buf;
   inst->tch_type=tt;
   inst->tch.nitems=count_textures(tt);
   if(buf) choose_setcur(&inst->tch,get_texture_num(tt,buf),inst);
   setwmstuff(inst,0);
   XMapRaised(dpy,inst->tch_frame);
}

void tchoose_wall(XWadInstance *inst,char *buf) {
   do_tchoose(inst,TT_WALL,buf);
}
void tchoose_flat(XWadInstance *inst,char *buf) {
   do_tchoose(inst,TT_FLAT,buf);
}
void tchoose_sprite(XWadInstance *inst,char *buf) {
   do_tchoose(inst,TT_SPRITE,buf);
}
void tchoose_patch(XWadInstance *inst,char *buf) {
   do_tchoose(inst,TT_PATCH,buf);
}

void free_tchoose(XWadInstance *inst) {
   free_cseti(&inst->tchctls);
   free_choose(&inst->tch);
   inst->tch_buf=NULL;
   inst->tch_type=TT_BAD;
}
