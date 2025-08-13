#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* uncomment this to disable DGA */
/*#define NO_DGA*/
/* uncomment this to disable shared memory */
/*#define NO_XSHM*/
/* uncomment this to disable XKB */
/*#define NO_XKB */

#ifndef NO_DGA
#include <X11/extensions/xf86dga.h>
#endif

#ifndef NO_XSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

#ifndef NO_XKB
#include <X11/XKBlib.h>
#endif

#include "lib/log.h"
#include "lib/safem.h"
#include "input.h"
#include "video.h"
#include "ctlkey_input.h"

#ifndef __cplusplus
#define c_class class
#endif

ConfItem video_conf[]={
#ifdef NO_DGA
   CONFNB("use-dga",NULL,0,"<disabled at compile time>"),
#else
   CONFNB("use-dga",NULL,0,"allow use of XFree86's DGA extension"),
#endif
#ifdef NO_XSHM
   CONFNB("use-xshm",NULL,0,"<disabled at compile time>"),
#else
   CONFNB("use-xshm",NULL,0,"allow use of the MIT shared memory extension"),
#endif
   CONFNB("grab-focus",NULL,0,"grab the input focus while DUMB runs"),
   {NULL}
};
#define use_dga (video_conf[0].intval)
#define use_shm (video_conf[1].intval)
#define grab_focus (video_conf[2].intval)

ConfItem input_conf[]={
#ifdef NO_XKB
   CONFNB("use-xkb",NULL,0,"<disabled at compile time>"),
#else
   CONFNB("use-xkb",NULL,0,"allow use of the XKeyboard extension"),
#endif
   {NULL}
};
#define use_xkb (input_conf[0].intval)

static Display *dpy=NULL;
static Window win=None;
#define scrn DefaultScreen(dpy)
#define rootw RootWindow(dpy,scrn)
#define visual DefaultVisual(dpy,scrn)

static Colormap cmap=None;
static char *fb=NULL,*fballoc=NULL;
static int width,height,pagelen,memlen,npages,real_width;
static XImage *image=NULL;

/* this is used for DGA double buffering */
static int page2ofs=0,page2line=0,curpage=0;

static grab_worked=0;

static XShmSegmentInfo xshminfo;

static int need_cmapinst=0,need_map=1;

#ifndef NO_XKB
static void init_xkb(void);
#endif

static int xerr;
static int errh(Display *d,XErrorEvent *ev) {xerr++;return 0;};
static void trap_errs(void) {
   xerr=0;
   XSetErrorHandler(errh);
};
static int untrap_errs(void) {
   XSync(dpy,0);
   XSetErrorHandler(NULL);
   return xerr;
};

static int chk_shm(void) {
   struct shmid_ds d;
   shmctl(xshminfo.shmid,IPC_STAT,&d);
   //logprintf(LOG_INFO,'V',"Shm Attaches=%d",d.shm_nattch);
   return d.shm_nattch<2;
};

void init_video(int *_width,int *_height,int *_bpp,int *_real_width) {
#ifndef NO_DGA
   int i;
   unsigned int ui;
#endif   
   /* init Xlib */
   if(dpy!=NULL) reset_video();

   memset(&xshminfo,0,sizeof(xshminfo));
   dpy=XOpenDisplay(NULL);
   if(dpy==NULL) logfatal('V',"x11_video: Can't open default display");
   cmap=None;
   npages=0;
   grab_worked=0;

#ifdef NO_DGA
   if(0) ;
#else
   /* init DGA */
   if(use_dga&&XF86DGAQueryExtension(dpy,&i,&i)) {
      XGetGeometry(dpy,rootw,&win,&i,&i,_width,_height,&ui,_bpp);
      win=None;
      XF86DGAGetViewPortSize(dpy,scrn,_width,_height);
      XF86DGAGetVideo(dpy,scrn,&fb,_real_width,&pagelen,&memlen);
      *_bpp/=8;
      width=*_width;
      height=*_height;
      real_width=*_real_width;
      page2ofs=(*_bpp)*real_width*height;
      page2line=height;
      logprintf(LOG_INFO,'V',"DGA viewport=%d,%d,%d",width,height,*_bpp);
      if(pagelen<=0) {
	 XCloseDisplay(dpy);
	 dpy=NULL;
	 logprintf(LOG_ERROR,'V',"X server doesn't support DGA");
	 use_dga=0;
      }
      else {
	 npages=memlen/pagelen;
	 use_shm=0;
	 /* switch to direct vid */
	 XF86DGADirectVideo(dpy,scrn,XF86DGADirectGraphics);
	 XF86DGASetVidPage(dpy,scrn,0);
	 XF86DGAForkApp(scrn);
	 if(grab_focus)
	   if(XGrabKeyboard(dpy,rootw,False,
			    GrabModeSync,GrabModeAsync,
			    CurrentTime))
	     logprintf(LOG_ERROR,'V',"DGA: keyboard grab failed!");
	 grab_worked=1;
	 XSelectInput(dpy,rootw,KeyPressMask|KeyReleaseMask);
	 logprintf(LOG_INFO,'V',"using XF86-DGA");
      };
   }
#endif /* NO_DGA */   
   
   /* no dga? we'll need a window, then */
   else {
      use_dga=0;
      *_bpp=DefaultDepth(dpy,scrn)/8;
      if(*_bpp==3) *_bpp=4; /* for Matrox */
      height=*_height;
      width=*_width;
      *_real_width=real_width=width;
      win=XCreateSimpleWindow(dpy,rootw,0,0,
			      width,height,2,
			      BlackPixel(dpy,scrn),BlackPixel(dpy,scrn));
      if(win==None) logfatal('V',"XCreateWindow failed");
      else logprintf(LOG_DEBUG,'V',"win-id=%ld",win);
      need_map=1; /* defer mapping so that it happens after XSetWMProps */
      pagelen=memlen=*_bpp*width*height;
      npages=1;
      XSelectInput(dpy,win,KeyPressMask|KeyReleaseMask);

#ifdef NO_XSHM
      use_shm=0;
#else      
      /* do we have shm to use? */
      if(!XShmQueryExtension(dpy)) use_shm=0;
      if(use_shm) {
	 int r;
	 trap_errs();
	 if(*_bpp==4) 
	    image=XShmCreateImage(dpy,visual,24,ZPixmap,NULL,&xshminfo,
				  width,height);
	 else
	    image=XShmCreateImage(dpy,visual,*_bpp*8,ZPixmap,NULL,&xshminfo,
				  width,height);
	 if(untrap_errs()||image==NULL) 
	    logfatal('V',"XShmCreateImage failed");
	 xshminfo.shmid=shmget(IPC_PRIVATE,pagelen,IPC_CREAT|0777);
	 if(xshminfo.shmid<0) logfatal('V',"shmget failed");
	 fb=image->data=(char *)shmat(xshminfo.shmid,0,0);
	 /*logprintf(LOG_INFO,'V',"XShm: shared memory at %lx",
	   (unsigned long)fb);*/
	 xshminfo.shmaddr=fb;
	 xshminfo.readOnly=False;
	 trap_errs();
	 r=XShmAttach(dpy,&xshminfo);
	 if(untrap_errs()||!r||chk_shm()) {
	    logprintf(LOG_INFO,'V',"XShm disabled: are we not local?");
	    use_shm=0;
	    /*XShmDetach(dpy,&xshminfo); not needed as XShmAttach failed */
	    shmdt(xshminfo.shmaddr);
	    shmctl(xshminfo.shmid,IPC_RMID,0);
	    XFree(image);
	    image=NULL;
	    memset(&xshminfo,0,sizeof(xshminfo));
         } else {
	    logprintf(LOG_INFO,'V',"using XShm");
	    /* Mark the segment for deletion.
	     * So if the program crashes for some reason, causing both
	     * itself and the X server to detach from the segment, the
	     * operating system will automatically delete it.  Until
	     * that, it can be attached to & used just fine.
	     */
	    shmctl(xshminfo.shmid, IPC_RMID, 0);
	 }
      };
#endif /* NO_XSHM */
      
      /* no xshm */
      if(use_shm==0) {
	 fb=fballoc=safe_malloc(pagelen);
	 image=XCreateImage(dpy,visual,*_bpp*8,ZPixmap,0,fb,width,height,8,0);
	 if(image==NULL) logfatal('V',"XCreateImage failed");
	 logprintf(LOG_INFO,'V',"using Xlib Images (slow)");
      };
   };
      
   /* get a colormap */   
   if(visual->c_class==PseudoColor&&cmap==None) {
      cmap=XCreateColormap(dpy,rootw,visual,AllocAll);
      if(cmap==None) logprintf(LOG_ERROR,'V',"X colormap came back as None.  Probably some strange visual.");
      else logprintf(LOG_DEBUG,'V',"cmap-id=%ld",cmap);
   };
};

    
void reset_video(void) {
  if(dpy) {
    if(grab_worked) 
        XUngrabKeyboard(dpy,CurrentTime);
#ifndef NO_DGA
    if(use_dga) XF86DGADirectVideo(dpy,scrn,0);
#endif
    if(win!=None) XDestroyWindow(dpy,win);
    if(cmap!=None) XFreeColormap(dpy,cmap);
#ifndef NO_XSHM
    if(use_shm) {
      XShmDetach(dpy,&xshminfo);
      shmdt(xshminfo.shmaddr);
      /* shmctl(xshminfo.shmid,IPC_RMID,0); already done in init */
      memset(&xshminfo,0,sizeof(xshminfo));
    };
#endif      
    if(fballoc) safe_free(fballoc);
    if(image) XFree(image);
    image=NULL;
    fballoc=fb=NULL;
    cmap=None;
    win=None;
    XCloseDisplay(dpy);
    dpy=NULL;
  };
};

void video_setpal(unsigned char idx,
		  unsigned char red,
		  unsigned char green,
		  unsigned char blue) {
   XColor xc={idx,red<<8,green<<8,blue<<8,DoRed|DoGreen|DoBlue,0};   
   if(cmap!=None) {
      XStoreColor(dpy,cmap,&xc);
      need_cmapinst=1;
   };
};

void *video_newframe(void) {
  if(curpage) return fb+page2ofs;
  return fb;
};
void video_updateframe(void *v) {
#ifndef NO_DGA
  if(use_dga&&page2line) {
    XF86DGASetViewPort(dpy,scrn,0,curpage*page2line);
    if(curpage) curpage=0;
    else curpage=1;
  };
#endif
  if(need_map) {
    if(win!=None)
      XMapRaised(dpy,win);
    XSync(dpy,False);
    need_map=0;
  };
  if(grab_focus&&win!=None&&!grab_worked)
    grab_worked=!XGrabKeyboard(dpy,win,False,
			       GrabModeAsync,GrabModeAsync,
			       CurrentTime);
  if(need_cmapinst) {
    need_cmapinst=0;
    logprintf(LOG_DEBUG,'V',"Installing colormap");
    if(win!=None) {
      XSetWindowColormap(dpy,win,cmap);
      XInstallColormap(dpy,cmap);
    }
#ifndef NO_DGA
    else XF86DGAInstallColormap(dpy,scrn,cmap);
#endif
  };
  if(image) {
#ifndef NO_XSHM      
      if(use_shm) 
	XShmPutImage(dpy,win,DefaultGC(dpy,scrn),
		     image,0,0,0,0,width,height,0);
      else 
#endif	
	XPutImage(dpy,win,DefaultGC(dpy,scrn),
		  image,0,0,0,0,width,height);
   };
  /* For the reasoning behind this, see comment in get_input() */
  if(use_shm||use_dga) XSync(dpy,0);
  else XFlush(dpy);
};

void video_preinit(void) {};

void video_winstuff(const char *desc,int xdim,int ydim) {
  char *argv[2] = { "dumb", NULL };
  static char buf[32];
  char *win_name = buf;
  XTextProperty w_name_prop, i_name_prop;
  XSizeHints size_hints;
  XClassHint class_hint={"dumb","dumb"};
  
  sprintf(buf,"dumb: %s",desc);
  
  XStringListToTextProperty(&win_name, 1, &w_name_prop);
  XStringListToTextProperty(&win_name, 1, &i_name_prop);
  
  size_hints.flags = PMinSize|PMaxSize;
  size_hints.width= size_hints.min_width = size_hints.max_width =
    xdim;
  size_hints.height= size_hints.min_height = size_hints.max_height =
    ydim;
  
  if(win!=None)
    XSetWMProperties(dpy, win, 
		     &w_name_prop, &i_name_prop, 
		     argv, 1,
		     &size_hints, NULL, &class_hint);
};


/* 
   It doesn't really make sense to seperate the video and input 
   modules for X.  Everything after this point is input.
*/ 

void get_input(PlayerInput *in) {
   int n;
   /* 
      rationale: if we're potentially running elsewhere from the server,
      we can make use of those milliseconds it takes for our XPutImage to
      get through to update objects and so on.  However, it's important
      that we're synced before trying to read any events.  And, if we're
      sharing memory with the server, we should not be writing to it before
      the server's finished putting up the last frame for the player to see.
      So, we should always flush after each frame, but sync after 
      each frame only if we're sharing memory; otherwise we can delay syncing
      until just before we get events.
   */
   if(!use_shm&&!use_dga) XSync(dpy,0);
   /* process all pending events */
   n=XEventsQueued(dpy,QueuedAfterReading);
   while(n--) {
      XEvent event;
      KeySym key;
      int pressed = 1;
      enum ctlkey ctlkey;
      XNextEvent(dpy,&event);
      //logprintf(LOG_DEBUG,'I',"Got XEvent");
      key=XLookupKeysym(&event.xkey,0);
      if(event.type==KeyRelease) { 
	 //logprintf(LOG_DEBUG,'I',"Got KeyRelease");
	 pressed=0;
      }
      else if(event.type==KeyPress); //logprintf(LOG_DEBUG,'I',"Got KeyPress");
      else {
	 logprintf(LOG_ERROR,'I',"Got some weird event type: %d",event.type);
	 continue;
      };
      switch (key) {
      case XK_Escape:
	 ctlkey = CTLKEY_QUIT;
	 break;
	
      case XK_Up:
      case XK_KP_8:
      case XK_KP_Up:
	 ctlkey = CTLKEY_MOVE_FORWARD;
	 break;
      case XK_Down:
      case XK_KP_2:
      case XK_KP_Down:
	 ctlkey = CTLKEY_MOVE_BACKWARD;
	 break;

      case XK_Left:
      case XK_KP_4:
      case XK_KP_Left:
	 ctlkey = CTLKEY_TURN_LEFT;
	 break;
      case XK_Right:
      case XK_KP_6:
      case XK_KP_Right:
	 ctlkey = CTLKEY_TURN_RIGHT;
	 break;
	
      case XK_comma:
      case XK_KP_1:
      case XK_KP_End:
	 ctlkey = CTLKEY_MOVE_LEFT;
	 break;
      case XK_period:
      case XK_KP_3:
      case XK_KP_Page_Down:
	 ctlkey = CTLKEY_MOVE_RIGHT;
	 break;

      case XK_A:
      case XK_a:
	 ctlkey = CTLKEY_MOVE_UP;
	 break;
      case XK_Z:
      case XK_z:
	 ctlkey = CTLKEY_MOVE_DOWN;
	 break;
	
      case XK_Page_Up:
	 ctlkey = CTLKEY_LOOK_UP;
	 break;
      case XK_Page_Down:
	 ctlkey = CTLKEY_LOOK_DOWN;
	 break;
      case XK_KP_9:
      case XK_KP_Page_Up:
	 ctlkey = CTLKEY_AIM_UP;
	 break;
      case XK_KP_7:
      case XK_KP_Home:
	 ctlkey = CTLKEY_AIM_DOWN;
	 break;
      case XK_KP_5:
      case XK_KP_Begin:
	 ctlkey = CTLKEY_CENTER_VIEW;
	 break;
	
      case XK_Shift_L:
      case XK_Shift_R:
	 ctlkey = CTLKEY_RUN;
	 break;
      case XK_Alt_L:
      case XK_Alt_R:
      case XK_Meta_L:
      case XK_Meta_R:
      case XK_Mode_switch:
      case XK_slash:
	 ctlkey = CTLKEY_STRAFE;
	 break;
	
      case XK_Control_L:
      case XK_Control_R:
	 ctlkey = CTLKEY_SHOOT;
	 break;
      case XK_X:
      case XK_x:
	 ctlkey = CTLKEY_SHOOT_SPECIAL;
	 break;
	
      case XK_Q:
      case XK_q:
	 ctlkey = CTLKEY_PREVIOUS_WEAPON;
	 break;
      case XK_Tab:
      case XK_W:
      case XK_w:
	 ctlkey = CTLKEY_NEXT_WEAPON;
	break;
#define NK(x) \
      case XK_##x: \
	 ctlkey = CTLKEY_WEAPON_##x; \
	 break;
      NK(0) NK(1) NK(2) NK(3) NK(4) NK(5) NK(6) NK(7) NK(8) NK(9)
#undef NK

      default:
	 // logprintf(LOG_DEBUG, 'I', "unknown keysym: %#08lx\n", (long) key);
      	 ctlkey = CTLKEY_NONE;
         break;
      }
      if (ctlkey != CTLKEY_NONE)
	 ctlkey_press(ctlkey, pressed);
   } /* while(n--) */
   ctlkey_calc_tick();
   ctlkey_get_player_input(in);
};

void init_input(void) {
#ifndef NO_XKB
   /* I rely on video being initialized before input.  */
   init_xkb();
#endif
   ctlkey_init();
};

#ifndef NO_XKB
static void
init_xkb(void)
{
   int major_ver=XkbMajorVersion, minor_ver=XkbMinorVersion;
   int dar_supported;
   if (!use_xkb) {
      /* disabled in command line */
      return;
   }
   if (!XkbLibraryVersion(&major_ver, &minor_ver)) {
      /* shared library is incompatible with the one this was compiled for */
      use_xkb = 0;
      logprintf(LOG_ERROR, 'I', 
		"XKB %d.%d in Xlib is incompatible with our %d.%d",
		major_ver, minor_ver, XkbMajorVersion, XkbMinorVersion);
      return;
   }
   major_ver = XkbMajorVersion;
   minor_ver = XkbMinorVersion;
   if (!XkbQueryExtension(dpy, NULL, NULL, NULL, &major_ver, &minor_ver)) {
      /* X server is not compatible with our version of XKB */
      use_xkb = 0;
      logprintf(LOG_ERROR, 'I', 
		"XKB %d.%d in X server is incompatible with our %d.%d",
		major_ver, minor_ver, XkbMajorVersion, XkbMinorVersion);
      return;
   }
   XkbSetDetectableAutoRepeat(dpy, True, &dar_supported);
   if (!dar_supported)
      logprintf(LOG_INFO, 'I',
		"X server does not support DetectableAutorepeat");
   else
      logprintf(LOG_INFO, 'I',
		"DetectableAutorepeat enabled via XKB");
}
#endif /* ndef NO_XKB */

void reset_input(void) {
   ctlkey_reset();
};

// Local Variables:
// c-basic-offset: 3
// End:
