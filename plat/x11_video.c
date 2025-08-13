#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* DGA doesn't work all that well at present */
#define NO_DGA
/* uncomment this to disable shared memory */
/*#define NO_XSHM*/

#ifndef NO_DGA
#include <X11/extensions/xf86dga.h>
#endif

#ifndef NO_XSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

#include "lib/log.h"
#include "lib/safem.h"
#include "input.h"
#include "video.h"

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
   {NULL}
};
#define use_dga (video_conf[0].intval)
#define use_shm (video_conf[1].intval)

ConfItem input_conf[]={{NULL}};

static Display *dpy=NULL;
static Window win=None;
#define scrn DefaultScreen(dpy)
#define rootw RootWindow(dpy,scrn)
#define visual DefaultVisual(dpy,scrn)

static Colormap cmap=None;
static char *fb=NULL,*fballoc=NULL;
static int width,height,pagelen,memlen,npages;
static XImage *image=NULL;

static XShmSegmentInfo xshminfo;

static int need_cmapinst=0,need_map=1;

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

void init_video(int *_width,int *_height,int *_bpp) {
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
   
#ifdef NO_DGA
   if(0) ;
#else
   /* init DGA */
   if(use_dga&&XF86DGAQueryExtension(dpy,&i,&i)) {
      XGetGeometry(dpy,rootw,&win,&i,&i,_width,_height,&ui,_bpp);
      *_bpp/=8;
      width=*_width;
      height=*_height;
      win=None;
      pagelen=memlen=0;
      XF86DGAGetVideo(dpy,scrn,&fb,&width,&pagelen,&memlen);
      if(pagelen<=0) {
	 XCloseDisplay(dpy);
	 dpy=NULL;
	 logprintf(LOG_ERROR,'V',"X server doesn't support DGA");
	 use_dga=0;
      }
      else {
	 npages=memlen/pagelen;
	 /* switch to direct vid */
	 XF86DGADirectVideo(dpy,scrn,XF86DGADirectGraphics);
	 XF86DGASetVidPage(dpy,scrn,0);
	 XF86DGAForkApp(scrn);
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
	    /*XShmDetach(dpy,&xshminfo);*/
	    shmdt(xshminfo.shmaddr);
	    shmctl(xshminfo.shmid,IPC_RMID,0);
	    XFree(image);
	    image=NULL;
	    memset(&xshminfo,0,sizeof(xshminfo));
         }
	 else logprintf(LOG_INFO,'V',"using XShm");
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
#ifndef NO_DGA
      if(use_dga) XF86DGADirectVideo(dpy,scrn,0);
#endif
      if(win!=None) XDestroyWindow(dpy,win);
      if(cmap!=None) XFreeColormap(dpy,cmap);
#ifndef NO_XSHM
      if(use_shm) {
	 XShmDetach(dpy,&xshminfo);
	 shmdt(xshminfo.shmaddr);
	 shmctl(xshminfo.shmid,IPC_RMID,0);
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
   return fb;
};
void video_updateframe(void *v) {
   if(need_map) {
      XMapRaised(dpy,win);
      need_map=0;
   };
   if(need_cmapinst) {
      need_cmapinst=0;
      logprintf(LOG_DEBUG,'V',"Installing colormap");
      if(win) {
	 XSetWindowColormap(dpy,win,cmap);
	 XInstallColormap(dpy,cmap);
      };
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

     XSetWMProperties(dpy, win, 
		      &w_name_prop, &i_name_prop, 
		      argv, 1,
                      &size_hints, NULL, &class_hint);
};


/* 
   It doesn't really make sense to seperate the video and input 
   modules for X.  Everything after this point is input.
*/ 

static PlayerInput inp_state;
static int run_state,strafe_state;

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
      int tmp;
      XEvent event;
      KeySym key;
      int speed=run_state?(2*UNIT_SPEED):UNIT_SPEED,pressed=1;
      XNextEvent(dpy,&event);
      //logprintf(LOG_DEBUG,'I',"Got XEvent");
      key=XLookupKeysym(&event.xkey,0);
      if(event.type==KeyRelease) { 
	 //logprintf(LOG_DEBUG,'I',"Got KeyRelease");
	 pressed=speed=0;
      }
      else if(event.type==KeyPress); //logprintf(LOG_DEBUG,'I',"Got KeyPress");
      else {
	 logprintf(LOG_ERROR,'I',"Got some weird event type: %d",event.type);
	 continue;
      };
      switch(key) {
      case(XK_Q):
      case(XK_Escape):
	 inp_state.quit=pressed;
	 break;
      case(XK_space):
	 inp_state.action=pressed;
	 break;
      case(XK_KP_7):
      case(XK_KP_Home):
      case(XK_Home):
	 inp_state.jump=pressed*UNIT_SPEED;
	 break;
      case(XK_Control_L):
      case(XK_Control_R):
      case(XK_Return):
	 inp_state.shoot=pressed;
	 break;
      case(XK_Tab):
	 inp_state.w_sel=pressed;
	 break;
	 
      case(XK_KP_4):
      case(XK_KP_Left):
      case(XK_Left):
	 if(!pressed) inp_state.sideways=inp_state.rotate=0;
	 else if(strafe_state) inp_state.sideways=speed;
	 else inp_state.rotate=speed;
	 break;

      case(XK_KP_6):
      case(XK_KP_Right):
      case(XK_Right):
	 if(!pressed) inp_state.sideways=inp_state.rotate=0;
	 else if(strafe_state) inp_state.sideways=-speed;
	 else inp_state.rotate=-speed;
	 break;

      case(XK_KP_8):
      case(XK_KP_Up):
      case(XK_Up):
	 inp_state.forward=speed;
	 break;
      case(XK_KP_2):
      case(XK_KP_Down):
      case(XK_Down):
	 inp_state.forward=-speed;
	 break;

      case(XK_KP_9):
      case(XK_KP_Page_Up):
      case(XK_Page_Up):
	 inp_state.lookup=pressed?UNIT_SPEED:0;
	 break;
      case(XK_KP_3):
      case(XK_KP_Page_Down):
      case(XK_Page_Down):
	 inp_state.lookup=pressed?-UNIT_SPEED:0;
	 break;

      case(XK_Shift_L):
      case(XK_Shift_R):
	  if ((run_state=pressed)) {	
	      inp_state.forward*=2;
	      inp_state.sideways*=2;
	      inp_state.rotate*=2;
  
	  } else {
	      inp_state.forward/=2;
	      inp_state.sideways/=2;
	      inp_state.rotate/=2;
	  }
	 break;
      case(XK_Alt_L):
      case(XK_Alt_R):
      case(XK_Meta_L):
      case(XK_Meta_R):
      case(XK_slash):
	 tmp=inp_state.sideways;
	 inp_state.sideways=inp_state.rotate;
	 inp_state.rotate=tmp;
	 strafe_state=pressed;
	 break;

#define NK(x) case(XK_##x): inp_state.select[x]=pressed; break
      NK(0);NK(1);NK(2);NK(3);NK(4);NK(5);NK(6);NK(7);NK(8);NK(9);
#undef NK
      };
   };
   /* return current state */
   memcpy(in,&inp_state,sizeof(inp_state));
};

void init_input(void) {
   memset(&inp_state,0,sizeof(inp_state));
   run_state=strafe_state=0;
};
void reset_input(void) {
};


