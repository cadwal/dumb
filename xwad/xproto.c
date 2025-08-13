#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include <setjmp.h>

#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "libdumb/dsound.h"
#include "libdumb/sound.h"

#include "xproto.h"
#include "colour.h"
#include "disphash.h"
#include "xtexture.h"

static XFontStruct *font,*lilfont,*chofont;

static XPInstance inst[1];

static int silent=0;

#define RCOUNT 3

static jmp_buf alarm_jb;
RETSIGTYPE sigalarm_handler(int i) {
   int need_redraw=0;
   static int rcount=RCOUNT;
   /* sound */
   poll_sound();
   /* rotate */
   if(inst->rotate) {
      rcount--;
      if(rcount<=0) {
	 inst->currot++;
	 if(inst->currot>7) inst->currot=0;
	 need_redraw=1;
	 rcount=RCOUNT;
      }
   }
   /* animate */
   if(inst->animate) {
      inst->phcount-=7;
      if(inst->phcount<=0) {
	 int oldph=inst->curphase;
	 if(CURPHASE.flags&(TPH_DESTROY|TPH_BECOME|TPH_BECOME2)) {
	    inst->animate=0;
	    need_redraw=2;
	 }
	 else xproto_enter_phase(inst,CURPHASE.next);
	 if(inst->curphase!=oldph) need_redraw=2;
      }
   }
   /* do redraws */
   if(need_redraw==2) xproto_redraw(inst);
   else if(need_redraw==1) {
      XClearArea(dpy,inst->wdisp,0,0,0,0,False);
      redraw_wdisp(inst);
      rdoutp_cseti(&inst->dispctls,inst);
   }
   /* done */
   longjmp(alarm_jb,1);
}


static void usage(const char *name)  {
   printf("Usage: %s [options]\n"
	  "Where options are:\n"
	  "\t-v\t\tVerbose: log to the screen.\n"
	  "\t-s\t\tSilent: don't try to play sounds.\n"
	  "\t-w <wadfile> [wadfile] [wadfile]...\n"
	  "\t-l <logfile> [logfile] [logfile]...\n"
	  "\t-d <display>\n",name);
   exit(1);
}

#define usage() usage(argv[0])

#define MAX_WADS 16

int main(int argc,char **argv) {
   int nwads=0;
   const char *wadf[MAX_WADS];
   int i,quiet=1;
   char *dpyname=NULL;
   
   /* scan args */
   for(i=1;i<argc;i++)  {
      char c;
      if(argv[i][0]!='-') usage();
      switch(c=argv[i][1])  {
	 /* q = quiet */
      case('v'):
	 quiet=0;
	 break;
      case('s'):
	 silent=1;
	 break;
	 /* w = wad */
      case('w'):
	 while(++i<argc) {
	    if(argv[i][0]=='-')  {
	       i--;
	       break;
	    }
	    if(nwads<MAX_WADS) 
	       wadf[nwads++]=argv[i];
	 }
	 break;
	 /* l = log */
      case('l'):
	 while(++i<argc) {
	    if(argv[i][0]=='-')  {
	       i--;
	       break;
	    }
	    log_file(argv[i],LOG_ALL,NULL);
	 }
	 break;
	 /* d=display */
      case('d'):
	 if(++i>=argc) usage();
	 dpyname=argv[i];
	 break;
	 /* ? = help */
      case('?'):
      default:
	 usage();
      }
   }

   /* start logging */
   if(!quiet) {
      /*setlinebuf(stdout);*/ /* if stdout is a socket, we'll need this */
      log_stdout();
   }
   logprintf(LOG_BANNER,'M',"XPROTO");
  
   /* start Xlib */
   dpy=XOpenDisplay(dpyname);
   if(dpy==NULL)
      logfatal('M',"Can't open display %s",dpyname?dpyname:"<default>");
   root=DefaultRootWindow(dpy);
   screen=DefaultScreen(dpy);
   init_controls(dpy,
	font=XLoadQueryFont(dpy,"-adobe-helvetica-medium-r-*-*-10-*"),
	lilfont=XLoadQueryFont(dpy,"-misc-fixed-medium-r-*-*-8-*"));
   init_choosers(chofont=XLoadQueryFont(dpy,"-misc-fixed-medium-r-*-*-10-*"));

   /* load wads */
   if(nwads>0) {
      i=0;
      init_iwad(wadf[i++], NULL);
      while(i<nwads) init_pwad(wadf[i++], NULL);
   }
   else {
      init_iwad("doom2.wad", NULL);	/* FIXME: from .dumbrc */
      init_pwad("doom4dum.wad", NULL);
   }

   /* init colormaps */
   init_colour();
   
   /* init windows & map */
   init_textures();
   init_prothings();
   init_sound(11025);
   init_dsound();
   init_instance(inst);

   /* main loop */
   while(!inst->want_quit) {
      XEvent ev;
      setjmp(alarm_jb);
      /*if(inst->rotate||inst->animate)*/ {
#ifdef HAVE_SETITIMER
	    struct itimerval itv;
	    memset(&itv,0,sizeof(itv));
	    itv.it_value.tv_usec=100000; /* 1/10 of a second */
	    setitimer(ITIMER_REAL,&itv,NULL);
#else  /* !HAVE_SETITIMER */
	    alarm(1);
#endif /* !HAVE_SETITIMER */
	    signal(SIGALRM,sigalarm_handler);
      }
      XNextEvent(dpy,&ev);
      alarm(0);
      /* if there are any notifies lurking, we want the latest */
      if(ev.type==MotionNotify)
	 while(XCheckWindowEvent(dpy,ev.xmotion.window,
				 ButtonMotionMask|PointerMotionMask,&ev));
      /* dispatch events to window */
      dispatch(&ev);
   }

   /* close windows, free map */
   free_instance(inst);
   reset_prothings();
   reset_textures();
   reset_dsound();
   reset_sound();

   /* close Xlib */
   reset_choosers();
   reset_controls();
   XFreeFont(dpy,font);
   XFreeFont(dpy,lilfont);
   XFreeFont(dpy,chofont);
   if(dpy) XCloseDisplay(dpy);

   /* done */
   log_exit();
   return 0;
}

void redraw_wdisp(XPInstance *inst) {
   if(CURPROTO.sprite[0]) {
      char rot='1'+inst->currot;
      Texture *t=find_phase_sprite(&CURPROTO,
				   inst->curphase,
				   rot);
      if(t) xtexture(dpy,inst->wdisp,t,
		     t->name[7]==rot&&t->name[6]==CURPHASE.spr_phase);
   }
}
void xproto_redraw(XPInstance *inst) {
   XClearArea(dpy,inst->wdisp,0,0,0,0,False);
   redraw_wdisp(inst);
   rdable_cseti(&inst->actctls,inst);
   rdoutp_cseti(&inst->choctls,inst);
   rdoutp_cseti(&inst->dispctls,inst);
   rdlights_cseti(&inst->choctls,inst);
   rdlights_cseti(&inst->dispctls,inst);
}

void xproto_enter_phase(XPInstance *inst,int ph) {
   if(ph<0) ph=inst->curphase+1;
   inst->curphase=ph;
   inst->phcount=CURPHASE.wait;
   if(CURPHASE.sound>=0&&!silent) 
      play_dsound_local(CURPHASE.sound+CURPROTO.sound,0,0,0);
}

void xproto_sendsig(XPInstance *inst,ThingSignal sig) {
   int newphase=CURPROTO.signals[sig];
   if(newphase>=0) {
      xproto_enter_phase(inst,newphase);
      xproto_redraw(inst);
   }
}

static void framedhf(XEvent *ev,XPInstance *inst,void *info) {
   switch(ev->type) {
   case(ConfigureNotify):
      /* frame may have been resized, update internal geometry */
      update_intgeo(inst); 
      break;
   case(KeyPress):
   case(KeyRelease):
      /* key event */
      break;
   }
}
static void dispdhf(XEvent *ev,XPInstance *inst,void *info) {
   switch(ev->type) {
   case(Expose):
      redraw_wdisp(inst);
      break;
   }
}

static const char *chotext(int i,XPInstance *inst) {
   static char buf[64];
   sprintf(buf,"%5d %s",inst->protos[i].id,inst->protos[i].sprite);
   return buf;
}
static void chocur(int i,XPInstance *inst) {
   int j;
   inst->curphase=0;
   inst->phase_tbl=find_thingphase(CURPROTO.phase_id,0);
   for(j=0;j<NUM_THINGSIGS;j++) 
      cseti_enable(&(inst->actctls),j,CURPROTO.signals[j]>=0);
   xproto_redraw(inst);
}

void init_instance(XPInstance *inst) {
   /* clean slate */
   memset(inst,0,sizeof(XPInstance));

   /* load protos */
   inst->protos_ln=getlump("PROTOS");
   if(!LUMPNUM_OK(inst->protos_ln))
      logfatal('M',"no PROTOS lump in wad");
   inst->nprotos=get_lump_len(inst->protos_ln)/sizeof(ProtoThing);
   if(inst->nprotos<1)
      logfatal('M',"no protos");
   inst->protos=load_lump(inst->protos_ln);

   /* create frame & map viewer*/
   inst->frame=XCreateSimpleWindow(dpy,root,0,0,
				   8,8,
				   0,BlackPixel(dpy,screen),
				   CTLC(Background));

   inst->wdisp=XCreateSimpleWindow(dpy,inst->frame,0,0,
				   8,8,
				   1,BlackPixel(dpy,screen),
				   BlackPixel(dpy,screen));
   set_xtexture_cmap(dpy,inst->wdisp);
   add_dh(inst->frame,framedhf,inst,NULL);
   add_dh(inst->wdisp,dispdhf,inst,NULL);

   /* create controls */
   init_cseti(&inst->choctls,inst,inst->frame,cho_cset,1);
   init_cseti(&inst->dispctls,inst,inst->frame,disp_cset,1);
   init_cseti(&inst->actctls,inst,inst->frame,act_cset,1);

   /* create chooser */
   init_choose(&inst->chooser,inst->frame,inst);
   inst->chooser.nitems=inst->nprotos;
   inst->chooser.text=chotext;
   inst->chooser.chcur=chocur;
   chocur(0,inst);
   XMapWindow(dpy,inst->chooser.w);
   /*logprintf(LOG_DEBUG,'M',"nitems=%d",inst->chooser.nitems);*/

   /* make it all visible */
   update_intgeo(inst);
   update_wmtitle(inst);
   XMapWindow(dpy,inst->wdisp);
   XSelectInput(dpy,inst->wdisp,ExposureMask);
   XMapRaised(dpy,inst->frame);
   XSelectInput(dpy,inst->frame,KeyPressMask|StructureNotifyMask);
}

void free_instance(XPInstance *inst) {
   free_cseti(&inst->choctls);
   free_cseti(&inst->actctls);
   free_cseti(&inst->dispctls);
   free_choose(&inst->chooser);
   XDestroyWindow(dpy,inst->frame);
}

void update_wmtitle(XPInstance *inst) {
     char *argv[2] = { "XProtoThing", NULL };
     char *win_name = "XProtoThing";
     XTextProperty w_name_prop, i_name_prop;
     XSizeHints size_hints;
     XClassHint class_hint={"xprotothing","XProtoThing"};

     XStringListToTextProperty(&win_name, 1, &w_name_prop);
     XStringListToTextProperty(&win_name, 1, &i_name_prop);

     size_hints.flags = PMinSize;
     size_hints.width= size_hints.min_width = size_hints.max_width =
	  inst->min_width;
     size_hints.height= size_hints.min_height = size_hints.max_height =
	  inst->min_height;

     XSetWMProperties(dpy, inst->frame, 
		      &w_name_prop, &i_name_prop, 
		      argv, 1,
                      &size_hints, NULL, &class_hint);

}

#define XSPACE 12
#define YSPACE 8

void update_intgeo(XPInstance *inst) {
   int choc_width,choc_height;
   int actc_width,actc_height;
   int dispc_width,dispc_height;
   int frame_width,frame_height;
   int frame_x,frame_y,frame_depth,frame_border;
   Window frame_root;
   int disp_width=0,disp_height=128,cho_height=96;

   /* get control panel sizes */
   get_cset_size(inst->choctls.cset,&choc_width,&choc_height);
   get_cset_size(inst->actctls.cset,&actc_width,&actc_height);
   get_cset_size(inst->dispctls.cset,&dispc_width,&dispc_height);

   /* work out minimum frame size from them */
   inst->min_width=choc_width+
      MAX(MAX(dispc_width,actc_width)+XSPACE*3,disp_width+XSPACE*3);
   inst->min_height=MAX((dispc_height+actc_height+YSPACE*4+disp_height),
			(choc_height+YSPACE*3+cho_height));

   /* find out actual current frame size */
   XGetGeometry(dpy,inst->frame,&frame_root,
		&frame_x,&frame_y,&frame_width,&frame_height,
		&frame_border,&frame_depth);

   /* if frame is too small, grow it */
   if(frame_width<inst->min_width||frame_height<inst->min_height) {
      if(frame_width<inst->min_width) frame_width=inst->min_width;
      if(frame_height<inst->min_height) frame_height=inst->min_height;
      XResizeWindow(dpy,inst->frame,frame_width,frame_height);
   }

   /* arrange subwindows */
   XMoveResizeWindow(dpy,inst->chooser.w,
		     XSPACE,YSPACE,
		     choc_width,
		     cho_height=frame_height-YSPACE*3-choc_height);
   XMoveResizeWindow(dpy,inst->choctls.w,
		     XSPACE,frame_height-YSPACE-choc_height,
		     choc_width,choc_height);

   XMoveResizeWindow(dpy,inst->wdisp,
		     XSPACE*2+choc_width,YSPACE,
		     disp_width=frame_width-XSPACE*3-choc_width,
		     disp_height=frame_height-YSPACE*4-
		     actc_height-dispc_height);
   XMoveResizeWindow(dpy,inst->dispctls.w,
		     XSPACE*2+choc_width,YSPACE*2+disp_height,
		     dispc_width,dispc_height);
   XMoveResizeWindow(dpy,inst->actctls.w,
		     XSPACE*2+choc_width,YSPACE*3+disp_height+dispc_height,
		     actc_width,actc_height);
}


