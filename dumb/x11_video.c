/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/x11_video.c: Video and input driver for X11R6.
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1994 by Chris Laurel
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
#include <limits.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* uncomment this to disable DGA */
/*#undef DUMB_CONFIG_XF86DGA */
/* uncomment this to disable shared memory */
/*#undef DUMB_CONFIG_XSHM */
/* uncomment this to disable XKB */
/*#undef DUMB_CONFIG_XKB */

#ifdef DUMB_CONFIG_XF86DGA
#include <X11/extensions/xf86dga.h>
#endif

#ifdef DUMB_CONFIG_XSHM
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#endif

#ifdef DUMB_CONFIG_XKB
#include <X11/XKBlib.h>
#endif

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "input.h"
#include "video.h"
#include "keymap.h"
#include "keyname.h"

#ifndef __cplusplus
#define c_class class
#endif

ConfItem video_conf[] =
{
#ifdef DUMB_CONFIG_XF86DGA
   CONFB("use-dga", NULL, 0, N_("allow use of XFree86's DGA extension")),
#else
   CONFB("use-dga", NULL, 0, N_("<disabled at compile time>")),
#endif
#ifdef DUMB_CONFIG_XSHM
   CONFNB("use-xshm", NULL, 0, N_("allow use of the MIT shared memory extension")),
#else
   CONFNB("use-xshm", NULL, 0, N_("<disabled at compile time>")),
#endif
   CONFITEM_END
};
#define use_dga (video_conf[0].intval)
#define use_shm (video_conf[1].intval)

enum grab_choice_enum {
   GRAB_NEVER,
   GRAB_WHEN_POINTED,
   GRAB_ALWAYS
};

static const ConfEnum grab_choices[] =
{
   {"never", GRAB_NEVER},
   {"when-pointed", GRAB_WHEN_POINTED},
   {"always", GRAB_ALWAYS},
   CONFENUM_END
};

ConfItem input_conf[] =
{
#ifdef DUMB_CONFIG_XKB
   CONFNB("use-xkb", NULL, 0, N_("allow use of the XKeyboard extension")),
#else
   CONFNB("use-xkb", NULL, 0, N_("<disabled at compile time>")),
#endif
CONFE("grab-kbd", NULL, 0, N_("grab the keyboard focus while DUMB runs"),
      GRAB_ALWAYS, grab_choices),
   CONFITEM_END
};
#define use_xkb (input_conf[0].intval)
#define grab_kbd (input_conf[1].intval)

static Display *dpy = NULL;
static Window win = None;
#define scrn DefaultScreen(dpy)
#define rootw RootWindow(dpy,scrn)
#define visual DefaultVisual(dpy,scrn)

static Colormap cmap = None;
static char *fb = NULL, *fballoc = NULL;
static int width, height, pagelen, memlen, npages, real_width;
static XImage *image = NULL;

/* this is used for DGA double buffering */
#ifdef DUMB_CONFIG_XF86DGA
static int page2ofs = 0, page2line = 0, curpage = 0;
#endif

static int grabbed = 0;

static int need_cmapinst = 0, need_map = 1;

#ifdef DUMB_CONFIG_XKB
static void init_xkb(void);
#endif
static void handle_keyevent(XKeyEvent * ev);
static void handle_crossingevent(XCrossingEvent * ev);
static void grab_keys(int pointer_mode, int keyboard_mode);
static void ungrab_keys(void);

#ifdef DUMB_CONFIG_XSHM
static int xerr;

static int
errh(Display * d, XErrorEvent * ev)
{
   xerr++;
   return 0;
}

static void
trap_errs(void)
{
   xerr = 0;
   XSetErrorHandler(errh);
}

static int
untrap_errs(void)
{
   XSync(dpy, 0);
   XSetErrorHandler(NULL);
   return xerr;
}

static XShmSegmentInfo xshminfo;

static int
chk_shm(void)
{
   struct shmid_ds d;
   shmctl(xshminfo.shmid, IPC_STAT, &d);
   //logprintf(LOG_INFO, 'V', _("Shm Attaches=%d"), d.shm_nattch);
   return d.shm_nattch < 2;
}
#endif /* DUMB_CONFIG_XSHM */

void
init_video(int *_width, int *_height, int *_bpp, int *_real_width)
{
#ifdef DUMB_CONFIG_XF86DGA
   int i;
   unsigned int ui;
#endif
   /* init Xlib */
   if (dpy != NULL)
      reset_video();

   dpy = XOpenDisplay(NULL);
   if (dpy == NULL)
      logfatal('V', _("x11_video: Can't open default display"));
   cmap = None;
   npages = 0;
   grabbed = 0;

#ifdef DUMB_CONFIG_XF86DGA
   /* init DGA */
   if (use_dga && XF86DGAQueryExtension(dpy, &i, &i)) {
      XGetGeometry(dpy, rootw, &win, &i, &i, _width, _height, &ui, _bpp);
      win = None;
      XF86DGAGetViewPortSize(dpy, scrn, _width, _height);
      XF86DGAGetVideo(dpy, scrn, &fb, _real_width, &pagelen, &memlen);
      *_bpp /= 8;
      width = *_width;
      height = *_height;
      real_width = *_real_width;
      page2ofs = (*_bpp) * real_width * height;
      page2line = height;
      logprintf(LOG_INFO, 'V', _("DGA viewport=%d,%d,%d"),
		width, height, *_bpp);
      if (pagelen <= 0) {
	 XCloseDisplay(dpy);
	 dpy = NULL;
	 /* FIXME: Is this fatal or not? */
	 logprintf(LOG_ERROR, 'V', _("X server doesn't support DGA"));
	 use_dga = 0;
      } else {
	 npages = memlen / pagelen;
	 use_shm = 0;
	 /* switch to direct vid */
	 XF86DGADirectVideo(dpy, scrn, XF86DGADirectGraphics);
	 XF86DGASetVidPage(dpy, scrn, 0);
	 XF86DGAForkApp(scrn);
	 if (grab_kbd == GRAB_ALWAYS)
	    grab_keys(GrabModeSync, GrabModeAsync);
	 XSelectInput(dpy, rootw, KeyPressMask | KeyReleaseMask);
	 logprintf(LOG_INFO, 'V', _("using XF86-DGA"));
      }
   }
#else  /* !DUMB_CONFIG_XF86DGA */
   if (0);
#endif /* !DUMB_CONFIG_XF86DGA */

   /* no dga? we'll need a window, then */
   else {
      use_dga = 0;
      *_bpp = DefaultDepth(dpy, scrn) / 8;
      if (*_bpp == 3)
	 *_bpp = 4;		/* for Matrox */
      height = *_height;
      width = *_width;
      *_real_width = real_width = width;
      win = XCreateSimpleWindow(dpy, rootw, 0, 0,
				width, height, 2,
			   BlackPixel(dpy, scrn), BlackPixel(dpy, scrn));
      if (win == None)
	 logfatal('V', _("XCreateWindow failed"));
      else
	 logprintf(LOG_DEBUG, 'V', _("win-id=%ld"), win);
      need_map = 1; /* defer mapping so that it happens after XSetWMProps */
      pagelen = memlen = *_bpp * width * height;
      npages = 1;
      XSelectInput(dpy, win,
		   KeyPressMask | KeyReleaseMask
		   | EnterWindowMask | LeaveWindowMask);

#ifdef DUMB_CONFIG_XSHM
      memset(&xshminfo, 0, sizeof(xshminfo));
      /* do we have shm to use? */
      if (!XShmQueryExtension(dpy))
	 use_shm = 0;
      if (use_shm) {
	 int r;
	 trap_errs();
	 if (*_bpp == 4)
	    image = XShmCreateImage(dpy, visual, 24, ZPixmap, NULL,
				    &xshminfo, width, height);
	 else
	    image = XShmCreateImage(dpy, visual, *_bpp * 8, ZPixmap, NULL,
				    &xshminfo, width, height);
	 if (untrap_errs() || image == NULL)
	    logfatal('V', _("XShmCreateImage failed"));
	 xshminfo.shmid = shmget(IPC_PRIVATE, pagelen, IPC_CREAT | 0777);
	 if (xshminfo.shmid < 0)
	    logfatal('V', _("shmget failed"));
	 fb = image->data = (char *) shmat(xshminfo.shmid, 0, 0);
	 /*logprintf(LOG_INFO, 'V', _("XShm: shared memory at %lx"),
	    (unsigned long) fb); */
	 xshminfo.shmaddr = fb;
	 xshminfo.readOnly = False;
	 trap_errs();
	 r = XShmAttach(dpy, &xshminfo);
	 if (untrap_errs() || !r || chk_shm()) {
	    logprintf(LOG_INFO, 'V', _("XShm disabled: are we not local?"));
	    use_shm = 0;
	    /*XShmDetach(dpy,&xshminfo); not needed as XShmAttach failed */
	    shmdt(xshminfo.shmaddr);
	    shmctl(xshminfo.shmid, IPC_RMID, 0);
	    XFree(image);
	    image = NULL;
	    memset(&xshminfo, 0, sizeof(xshminfo));
	 } else {
	    logprintf(LOG_INFO, 'V', _("using XShm"));
	    /* Mark the shared memory segment for deletion if that
	     * doesn't stop programs from attaching to it.  So if DUMB
	     * crashes for some reason, causing both itself and the X
	     * server to detach from the segment, the operating system
	     * will automatically delete it.
	     *
	     * Some operating systems don't let programs attach to
	     * deleted segments.  Under those, we delay the delete
	     * until DUMB exits, because some X server might
	     * occasionally need to detach and reattach the segment.
	     * But if DUMB gets a fatal signal, the segment will be
	     * left there.
	     *
	     * FIXME: Catch SIGSEGV.  */
#ifdef DUMB_CONFIG_SYS_SHMAT_RMID
	    shmctl(xshminfo.shmid, IPC_RMID, 0);
#endif /* DUMB_CONFIG_SYS_SHMAT_RMID */
	 }
      }
#else  /* !DUMB_CONFIG_XSHM */
      use_shm = 0;
#endif /* !DUMB_CONFIG_XSHM */

      /* no xshm */
      if (use_shm == 0) {
	 fb = fballoc = safe_malloc(pagelen);
	 image = XCreateImage(dpy, visual, *_bpp * 8, ZPixmap, 0, fb,
			      width, height, 8, 0);
	 if (image == NULL)
	    logfatal('V', _("XCreateImage failed"));
	 logprintf(LOG_INFO, 'V', _("using Xlib Images (slow)"));
      }
   }

   /* get a colormap */
   if (visual->c_class == PseudoColor && cmap == None) {
      cmap = XCreateColormap(dpy, rootw, visual, AllocAll);
      if (cmap == None)
	 logprintf(LOG_ERROR, 'V',
		   _("X colormap came back as None.  "
		     "Probably some strange visual."));
      else
	 logprintf(LOG_DEBUG, 'V', _("cmap-id=%ld"), cmap);
   }
}

void
reset_video(void)
{
   if (dpy) {
      ungrab_keys();
#ifdef DUMB_CONFIG_XF86DGA
      if (use_dga)
	 XF86DGADirectVideo(dpy, scrn, 0);
#endif
      if (win != None)
	 XDestroyWindow(dpy, win);
      if (cmap != None)
	 XFreeColormap(dpy, cmap);
#ifdef DUMB_CONFIG_XSHM
      if (use_shm) {
	 XShmDetach(dpy, &xshminfo);
	 shmdt(xshminfo.shmaddr);
#ifdef DUMB_CONFIG_SYS_SHMAT_RMID
	 /* already deleted in initialisation */
#else  /* !DUMB_CONFIG_SYS_SHMAT_RMID */
	 shmctl(xshminfo.shmid, IPC_RMID, 0);
#endif /* !DUMB_CONFIG_SYS_SHMAT_RMID */
	 memset(&xshminfo, 0, sizeof(xshminfo));
      }
#endif /* DUMB_CONFIG_XSHM */
      if (fballoc)
	 safe_free(fballoc);
      if (image)
	 XFree(image);
      image = NULL;
      fballoc = fb = NULL;
      cmap = None;
      win = None;
      XCloseDisplay(dpy);
      dpy = NULL;
   }
}

void
video_setpal(unsigned char idx,
	     unsigned char red, unsigned char green, unsigned char blue)
{
   XColor xc =
   {idx, red << 8, green << 8, blue << 8, DoRed | DoGreen | DoBlue, 0};
   if (cmap != None) {
      XStoreColor(dpy, cmap, &xc);
      need_cmapinst = 1;
   }
}

void *
video_newframe(void)
{
#ifdef DUMB_CONFIG_XF86DGA
   if (curpage)
      return fb + page2ofs;
#endif
   return fb;
}

void
video_updateframe(void *v)
{
#ifdef DUMB_CONFIG_XF86DGA
   if (use_dga && page2line) {
      XF86DGASetViewPort(dpy, scrn, 0, curpage * page2line);
      if (curpage)
	 curpage = 0;
      else
	 curpage = 1;
   }
#endif
   if (need_map) {
      if (win != None)
	 XMapRaised(dpy, win);
      XSync(dpy, False);
      need_map = 0;
   }
   if (grab_kbd == GRAB_ALWAYS && win != None)
      grab_keys(GrabModeAsync, GrabModeAsync);
   if (need_cmapinst) {
      need_cmapinst = 0;
      logprintf(LOG_DEBUG, 'V', _("Installing colormap"));
      if (win != None) {
	 XSetWindowColormap(dpy, win, cmap);
	 XInstallColormap(dpy, cmap);
      }
#ifdef DUMB_CONFIG_XF86DGA
      else
	 XF86DGAInstallColormap(dpy, scrn, cmap);
#endif
   }
   if (image) {
#ifdef DUMB_CONFIG_XSHM
      if (use_shm)
	 XShmPutImage(dpy, win, DefaultGC(dpy, scrn),
		      image, 0, 0, 0, 0, width, height, 0);
      else
#endif /* DUMB_CONFIG_XSHM */
	 XPutImage(dpy, win, DefaultGC(dpy, scrn),
		   image, 0, 0, 0, 0, width, height);
   }
   /* For the reasoning behind this, see comment in get_input() */
   if (use_shm || use_dga)
      XSync(dpy, 0);
   else
      XFlush(dpy);
}

void
video_preinit(void)
{
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
   /* FIXME */
   char *argv[2] = {"dumb", NULL};
   static char buf[32];
   char *win_name = buf;
   XTextProperty w_name_prop, i_name_prop;
   XSizeHints size_hints;
   XClassHint class_hint = {"dumb", "dumb"};

   sprintf(buf, "dumb: %s", desc);

   XStringListToTextProperty(&win_name, 1, &w_name_prop);
   XStringListToTextProperty(&win_name, 1, &i_name_prop);

   size_hints.flags = PMinSize | PMaxSize;
   size_hints.width = size_hints.min_width = size_hints.max_width = xdim;
   size_hints.height = size_hints.min_height = size_hints.max_height = ydim;

   if (win != None)
      XSetWMProperties(dpy, win,
		       &w_name_prop, &i_name_prop,
		       argv, 1,
		       &size_hints, NULL, &class_hint);
}


/*
   It doesn't really make sense to seperate the video and input
   modules for X.  Everything after this point is input.
 */

void
get_input(PlayerInput *in)
{
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
   if (!use_shm && !use_dga)
      XSync(dpy, 0);
   /* process all pending events */
   n = XEventsQueued(dpy, QueuedAfterReading);
   while (n--) {
      XEvent event;
      XNextEvent(dpy, &event);
      //logprintf(LOG_DEBUG, 'I', _("Got XEvent"));
      switch (event.type) {
      case KeyPress:
      case KeyRelease:
	 handle_keyevent(&event.xkey);
	 break;
      case EnterNotify:
      case LeaveNotify:
	 handle_crossingevent(&event.xcrossing);
	 break;
      default:
	 logprintf(LOG_ERROR, 'I', _("Got some weird event type: %d"),
		   event.type);
      }
   }
   ctlkey_calc_tick();
   ctlkey_get_player_input(in);
}

static void
handle_keyevent(XKeyEvent * ev)
{
   /* The parameter should be const but XLookupKeysym() isn't const in
    * my headers and I don't want to insert casts.  And it doesn't
    * matter because get_input() doesn't do anything with the event
    * after passing it here.  */
   KeySym key;
   int pressed;
   /* ev->type is either KeyPress or KeyRelease.  Otherwise
    * get_input() wouldn't have called this function.  */
   if (ev->type == KeyPress) {
      // logprintf(LOG_DEBUG, 'I', _("Got KeyPress"));
      pressed = 1;
   } else {
      // logprintf(LOG_DEBUG, 'I', _("Got KeyRelease"));
      pressed = 0;
   }
   key = XLookupKeysym(ev, 0);
   keymap_press_keycode(key, pressed);
}

/* I originally wanted to base this on FocusIn and FocusOut events so
 * that DUMB would capture the keyboard even when the pointer was at
 * the window frame created by the window manager.  But XGrabKeyboard
 * sent FocusIn and FocusOut events of its own and it started ringing
 * and was very slow and unreliable.  So I chose this as a compromise.
 */
static void
handle_crossingevent(XCrossingEvent * ev)
{
   if (grab_kbd == GRAB_WHEN_POINTED) {
      if (ev->type == EnterNotify) {
	 // logprintf(LOG_DEBUG, 'I', _("EnterNotify received\n"));
	 grab_keys(GrabModeAsync, GrabModeAsync);
      } else {
	 // logprintf(LOG_DEBUG, 'I', _("LeaveNotify received\n"));
	 ungrab_keys();
      }
   }
}

void
init_input(void)
{
   if (!dpy)
      logfatal('I', _("x11_input must be initialised after x11_video"));
#ifdef DUMB_CONFIG_XKB
   init_xkb();
#endif
}

void
reset_input(void)
{
}

#ifdef DUMB_CONFIG_XKB
static void
init_xkb(void)
{
   int major_ver = XkbMajorVersion, minor_ver = XkbMinorVersion;
   int dar_supported;
   if (!use_xkb) {
      /* disabled in command line */
      return;
   }
   if (!XkbLibraryVersion(&major_ver, &minor_ver)) {
      /* shared library is incompatible with the one this was compiled for */
      use_xkb = 0;
      logprintf(LOG_ERROR, 'I',
		_("XKB %d.%d in Xlib is incompatible with our %d.%d"),
		major_ver, minor_ver, XkbMajorVersion, XkbMinorVersion);
      return;
   }
   major_ver = XkbMajorVersion;
   minor_ver = XkbMinorVersion;
   if (!XkbQueryExtension(dpy, NULL, NULL, NULL, &major_ver, &minor_ver)) {
      /* X server is not compatible with our version of XKB */
      use_xkb = 0;
      logprintf(LOG_ERROR, 'I',
		_("XKB %d.%d in X server is incompatible with our %d.%d"),
		major_ver, minor_ver, XkbMajorVersion, XkbMinorVersion);
      return;
   }
   XkbSetDetectableAutoRepeat(dpy, True, &dar_supported);
   if (!dar_supported)
      logprintf(LOG_INFO, 'I',
		_("X server does not support DetectableAutorepeat"));
   else
      logprintf(LOG_INFO, 'I',
		_("DetectableAutorepeat enabled via XKB"));
}
#endif /* DUMB_CONFIG_XKB */

static void
grab_keys(int pointer_mode, int keyboard_mode)
{
   static int complain_on_failure = 1;
   if (grabbed)
      return;
   if (XGrabKeyboard(dpy, rootw, False,
		     pointer_mode, keyboard_mode,
		     CurrentTime)) {
      /* Grab failed.  But don't complain all the time. */
      if (complain_on_failure) {
	 logprintf(LOG_ERROR, 'I', _("keyboard grab failed!"));
	 complain_on_failure = 0;
      }
   } else {
      grabbed = 1;
      /* It worked now, so complain next time it doesn't. */
      complain_on_failure = 1;
   }
}

static void
ungrab_keys(void)
{
   if (!grabbed)
      return;
   XUngrabKeyboard(dpy, CurrentTime);
   grabbed = 0;
}

const char *
keymap_keycode_to_keyname(keymap_keycode keycode)
{
   /* X has separate keycodes and keysyms but keymap.h calls them all
    * keycodes.  */
   if (keycode >= CHAR_MIN && keycode <= CHAR_MAX)
      return keyname_of_char((char) keycode);
   else
      return XKeysymToString((KeySym) keycode);
}

void
keymap_free_keyname(const char *keyname)
{
   /* no need to free anything */
}

// Local Variables:
// c-basic-offset: 3
// End:
