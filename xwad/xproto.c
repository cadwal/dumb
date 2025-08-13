/* DUMB: A Doom-like 3D game engine.
 *
 * tool/xproto.c: XProtoThing, a program for viewing ProtoThings.
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef HAVE_SELECT
/* Since you don't have select(), we'll
 * have to longjmp() out of SIGALRM.  */
# define USE_LONGJMP
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#ifdef USE_LONGJMP
# include <signal.h>
# include <setjmp.h>
#endif /* USE_LONGJMP */
#include <locale.h>

#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "libdumb/dsound.h"
#include "libdumb/protoinwad.h"
#include "libdumb/sound.h"

#include "xproto.h"
#include "colour.h"
#include "disphash.h"
#include "xtexture.h"
#ifndef USE_LONGJMP
# include "timewait.h"
#endif

static XFontStruct *font, *lilfont, *chofont;

static XPInstance inst[1];

/* 1 means sound is allowed.  Cleared by -s, --no-sound. */
static int sound_flag = 1;

#define RCOUNT 3

#ifdef USE_LONGJMP
static jmp_buf alarm_jb;
#endif

/* argv[0] saved near the beginning of main() */
static const char *argv0;

int main(int argc, char **argv);
#ifdef USE_LONGJMP
static RETSIGTYPE sigalarm_handler(int);
#endif
static void tick(void);
static void print_help(FILE *dest);
static void print_version(void);

#define MAX_WADS 16

int
main(int argc, char **argv)
{
   int nwads = 0;
   const char *wadf[MAX_WADS];
   int verbose_flag = 0;
   char *dpyname = NULL;
   static const struct option long_options[] =
   {
      { "load-wad", required_argument, NULL, 'w' },
      /* The following option is not --silent or --quiet because GNU
       * defines those to mean the opposite of --verbose.  */
      { "no-sound", no_argument, NULL, 's' },
      { "display", required_argument, NULL, 'd' },
      { "log-to", required_argument, NULL, 'l' },
      { "verbose", no_argument, NULL, 'v' },
      { "help", no_argument, NULL, 'h' }, /* no -h */
      { "version", no_argument, NULL, 'V' }, /* no -V */
      { NULL, 0, NULL, '\0' }
   };
   argv0 = argv[0];

#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */

   for (;;) {
      int c = getopt_long(argc, argv, "w:sd:l:v", long_options, NULL);
      if (c == -1)
	 break;			/* end of options */
      switch (c) {
      case 'w':			/* -w, --load-wad=FILE */
	 if (nwads >= MAX_WADS) {
	    fprintf(stderr, _("%s: internal limit on wad files exceeded\n"),
		    argv0);
	    exit(DUMB_EXIT_INTERNAL_LIMIT);
	 } else
	    wadf[nwads++] = optarg;
	 break;
      case 's':			/* -s, --no-sound */
	 sound_flag = 0;
	 break;
      case 'd':			/* -d, --display=DISPLAY */
	 dpyname = optarg;
	 break;
      case 'l':			/* -l, --log-to=FILE */
	 log_file(optarg, LOG_ALL, NULL);
	 break;
      case 'v':			/* -v, --verbose */
	 verbose_flag = 1;
	 break;
      case 'h':			/*     --help */
	 print_help(stdout);
	 exit(EXIT_SUCCESS);
      case 'V':			/*     --version */
	 print_version();
	 exit(EXIT_SUCCESS);
      case '?':			/* invalid option */
	 fprintf(stderr, _("Try `%s --help' for more information.\n"), argv0);
	 exit(DUMB_EXIT_INVALID_ARGS);
      default:
	 /* getopt_long() returned an impossible value.  */
	 abort();
      }	/* switch */
   } /* for ever */

   /* process non-option arguments */
   while (optind < argc) {
      if (nwads >= MAX_WADS) {
	 fprintf(stderr, _("%s: internal limit on wad files exceeded\n"),
		 argv0);
	 exit(DUMB_EXIT_INTERNAL_LIMIT);
      } else
	 wadf[nwads++] = argv[optind++];
   }

   /* start logging */
   log_stream(stderr, verbose_flag ? LOG_ALL : LOG_WARNING, NULL);
   logprintf(LOG_BANNER, 'M', "XPROTO");

   /* start Xlib */
   dpy = XOpenDisplay(dpyname);
   if (dpy == NULL)
      logfatal('M', _("Can't open display %s"),
	       dpyname ? dpyname : _("<default>"));
   root = DefaultRootWindow(dpy);
   screen = DefaultScreen(dpy);
   font = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-*-*-10-*");
   lilfont = XLoadQueryFont(dpy, "-misc-fixed-medium-r-*-*-8-*");
   init_controls(dpy, font, lilfont);
   chofont = XLoadQueryFont(dpy, "-misc-fixed-medium-r-*-*-10-*");
   init_choosers(chofont);

   /* load wads */
   if (nwads > 0) {
      int i = 0;
      init_iwad(wadf[i++], NULL);
      while (i < nwads)
	 init_pwad(wadf[i++], NULL);
   } else {
      /* There's no MAPNAME, so the program cannot know whether
       * doom.wad or doom2.wad is meant.  Assume doom2.wad. */
      init_iwad("doom2.wad", NULL);	/* FIXME: from .dumbrc */
      init_pwad("doom4dum.wad", NULL);
   }

   /* init colormaps */
   init_colour();

   /* init windows & map */
   init_textures();
   init_prothings();
   if (sound_flag) {
      init_sound(11025);		/* TODO: make configurable */
      init_dsound();
   }
   init_instance(inst);

   /* main loop */
   while (!inst->want_quit) {
      XEvent ev;
#ifdef USE_LONGJMP
      setjmp(alarm_jb);
      /*if(inst->rotate||inst->animate) */  {
	 /* Hook the signal before triggering it. */
	 signal(SIGALRM, sigalarm_handler);
	 /* Linux 2.0.36 with GNU libc 2.0.7 seems to keep SIGALRM blocked
	  * after the handler calls longjmp().  Unblock it.  */
	 {
	    sigset_t sigs;
	    sigemptyset(&sigs);
	    sigaddset(&sigs, SIGALRM);
	    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
	 }
# ifdef HAVE_SETITIMER
	 {
	    struct itimerval itv;
	    memset(&itv, 0, sizeof(itv));
	    itv.it_value.tv_usec = 100000; /* 1/10 of a second */
	    setitimer(ITIMER_REAL, &itv, NULL);
	 }
# else  /* !HAVE_SETITIMER */
	 alarm(1);		/* 1 second */
# endif /* !HAVE_SETITIMER */
      }
      XNextEvent(dpy, &ev);
      alarm(0);
#else  /* !USE_LONGJMP */
      static struct timeval previous_tick = { 0, 0 };
      struct timeval next_tick = previous_tick;
      next_tick.tv_usec += 1000000/5;
      if (next_tick.tv_usec > 1000000) {
	 next_tick.tv_usec -= 1000000;
	 next_tick.tv_sec  += 1;
      }
      while (!xnextevent_before(dpy, &ev, &next_tick)) {
	 tick();
	 /* Don't just assign previous_tick=next_tick;  */
	 gettimeofday(&previous_tick, NULL);
	 XSync(dpy, False);
      }
#endif /* !USE_LONGJMP */
      /* if there are any notifies lurking, we want the latest */
      if (ev.type == MotionNotify)
	 while (XCheckWindowEvent(dpy, ev.xmotion.window,
			     ButtonMotionMask | PointerMotionMask, &ev));
      /* dispatch events to window */
      dispatch(&ev);
   }


   /* close windows, free map */
   free_instance(inst);
   reset_prothings();
   reset_textures();
   if (sound_flag) {
      reset_dsound();
      reset_sound();
   }

   /* close Xlib */
   reset_choosers();
   reset_controls();
   XFreeFont(dpy, font);
   XFreeFont(dpy, lilfont);
   XFreeFont(dpy, chofont);
   if (dpy)
      XCloseDisplay(dpy);

   /* done */
   log_exit();
   return 0;
}

#ifdef USE_LONGJMP

static RETSIGTYPE
sigalarm_handler(int i)
{
   tick();
   longjmp(alarm_jb, 1);
}

#endif /* !USE_LONGJMP */

static void
tick(void)
{
   int need_redraw = 0;
   static int rcount = RCOUNT;
   /* sound */
   if (sound_flag)
      poll_sound();
   /* rotate */
   if (inst->rotate) {
      rcount--;
      if (rcount <= 0) {
	 inst->currot++;
	 if (inst->currot > 7)
	    inst->currot = 0;
	 need_redraw = 1;
	 rcount = RCOUNT;
      }
   }
   /* animate */
   if (inst->animate) {
      inst->phcount -= 7;
      if (inst->phcount <= 0) {
	 int oldph = inst->curphase;
	 if (CURPHASE.flags & (TPH_DESTROY | TPH_BECOME | TPH_BECOME2)) {
	    inst->animate = 0;
	    need_redraw = 2;
	 } else
	    xproto_enter_phase(inst, CURPHASE.next);
	 if (inst->curphase != oldph)
	    need_redraw = 2;
      }
   }
   /* do redraws */
   if (need_redraw == 2)
      xproto_redraw(inst);
   else if (need_redraw == 1) {
      XClearArea(dpy, inst->wdisp, 0, 0, 0, 0, False);
      redraw_wdisp(inst);
      rdoutp_cseti(&inst->dispctls, inst);
   }
}

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s [OPTION]... [WADFILE]...\n"
	     "Interactively view ProtoThings in X11.\n"
	     "\n"), argv0);
   fputs(_("  -w, --load-wad=FILE    load FILE as a WAD.  WADFILE argument does the same.\n"
	   "  -s, --no-sound         don't try to play sounds\n"
	   "  -d, --display=DISPLAY  use X display DISPLAY\n"
	   "  -l, --log-to=FILE      save messages to FILE\n"
	   "  -v, --verbose          log to the screen\n"
	   "      --help             display this help and exit\n"
	   "      --version          output version information and exit\n"
	   "\n"), dest);
   fputs(_("If no WADFILE nor --load-wad=FILE arguments are given, XProtoThing\n"
	   "loads `doom2.wad'.\n"
	   "\n"), dest);
   print_bugaddr_message(dest);
}

static void
print_version(void)
{
   static const struct copyright copyrights[] = {
      { "1998", "Josh Parsons" },
      COPYRIGHT_END
   };
   fputs("XProtoThing (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

void
redraw_wdisp(XPInstance *inst)
{
   if (CURPROTO.sprite[0]) {
      char rot = '1' + inst->currot;
      Texture *t = find_phase_sprite(&CURPROTO,
				     inst->curphase,
				     rot);
      if (t)
	 xtexture(dpy, inst->wdisp, t,
		  t->name[7] == rot && t->name[6] == CURPHASE.spr_phase);
   }
}

void
xproto_redraw(XPInstance *inst)
{
   XClearArea(dpy, inst->wdisp, 0, 0, 0, 0, False);
   redraw_wdisp(inst);
   rdable_cseti(&inst->actctls, inst);
   rdoutp_cseti(&inst->choctls, inst);
   rdoutp_cseti(&inst->dispctls, inst);
   rdlights_cseti(&inst->choctls, inst);
   rdlights_cseti(&inst->dispctls, inst);
}

void
xproto_enter_phase(XPInstance *inst, int ph)
{
   if (ph < 0)
      ph = inst->curphase + 1;
   inst->curphase = ph;
   inst->phcount = CURPHASE.wait;
   if (CURPHASE.sound >= 0 && sound_flag)
      play_dsound_local(CURPHASE.sound + CURPROTO.sound, 0, 0, 0);
}

void
xproto_sendsig(XPInstance *inst, ThingSignal sig)
{
   int newphase = CURPROTO.signals[sig];
   if (newphase >= 0) {
      xproto_enter_phase(inst, newphase);
      xproto_redraw(inst);
   }
}

static void
framedhf(XEvent *ev, XPInstance *inst, void *info)
{
   switch (ev->type) {
   case (ConfigureNotify):
      /* frame may have been resized, update internal geometry */
      update_intgeo(inst);
      break;
   case (KeyPress):
   case (KeyRelease):
      /* key event */
      break;
   }
}

static void
dispdhf(XEvent *ev, XPInstance *inst, void *info)
{
   switch (ev->type) {
   case (Expose):
      redraw_wdisp(inst);
      break;
   }
}

static const char *
chotext(int i, XPInstance *inst)
{
   static char buf[64];
   sprintf(buf, "%5d %s", inst->protos[i].id, inst->protos[i].sprite);
   return buf;
}

static void
chocur(int i, XPInstance *inst)
{
   int j;
   inst->curphase = 0;
   inst->phase_tbl = find_thingphase(CURPROTO.phase_id, 0);
   for (j = 0; j < NUM_THINGSIGS; j++)
      cseti_enable(&(inst->actctls), j, CURPROTO.signals[j] >= 0);
   xproto_redraw(inst);
}

void
init_instance(XPInstance *inst)
{
   /* clean slate */
   memset(inst, 0, sizeof(XPInstance));

   /* load protos */
   inst->protos_ln = getlump("PROTOS");
   if (!LUMPNUM_OK(inst->protos_ln))
      logfatal('M', _("no PROTOS lump in wad"));
   inst->nprotos = decode_protoinwad_array(&inst->protos,
					   ((const ProtoThing_inwad *)
					    load_lump(inst->protos_ln)),
					   get_lump_len(inst->protos_ln));
   free_lump(inst->protos_ln);
   if (inst->nprotos < 1)
      logfatal('M', _("no protos"));

   /* create frame & map viewer */
   inst->frame = XCreateSimpleWindow(dpy, root, 0, 0,
				     8, 8,
				     0, BlackPixel(dpy, screen),
				     CTLC(Background));

   inst->wdisp = XCreateSimpleWindow(dpy, inst->frame, 0, 0,
				     8, 8,
				     1, BlackPixel(dpy, screen),
				     BlackPixel(dpy, screen));
   set_xtexture_cmap(dpy, inst->wdisp);
   add_dh(inst->frame, framedhf, inst, NULL);
   add_dh(inst->wdisp, dispdhf, inst, NULL);

   /* create controls */
   init_cseti(&inst->choctls, inst, inst->frame, cho_cset, 1);
   init_cseti(&inst->dispctls, inst, inst->frame, disp_cset, 1);
   init_cseti(&inst->actctls, inst, inst->frame, act_cset, 1);

   /* create chooser */
   init_choose(&inst->chooser, inst->frame, inst);
   inst->chooser.nitems = inst->nprotos;
   inst->chooser.text = chotext;
   inst->chooser.chcur = chocur;
   chocur(0, inst);
   XMapWindow(dpy, inst->chooser.w);
   /*logprintf(LOG_DEBUG, 'M', "nitems=%d", inst->chooser.nitems); */

   /* make it all visible */
   update_intgeo(inst);
   update_wmtitle(inst);
   XMapWindow(dpy, inst->wdisp);
   XSelectInput(dpy, inst->wdisp, ExposureMask);
   XMapRaised(dpy, inst->frame);
   XSelectInput(dpy, inst->frame, KeyPressMask | StructureNotifyMask);
}

void
free_instance(XPInstance *inst)
{
   free_cseti(&inst->choctls);
   free_cseti(&inst->actctls);
   free_cseti(&inst->dispctls);
   free_choose(&inst->chooser);
   XDestroyWindow(dpy, inst->frame);
}

void
update_wmtitle(XPInstance *inst)
{
   char *argv[2] =
   {"XProtoThing", NULL};
   char *win_name = "XProtoThing";
   XTextProperty w_name_prop, i_name_prop;
   XSizeHints size_hints;
   XClassHint class_hint =
   {"xprotothing", "XProtoThing"};

   XStringListToTextProperty(&win_name, 1, &w_name_prop);
   XStringListToTextProperty(&win_name, 1, &i_name_prop);

   size_hints.flags = PMinSize;
   size_hints.width = size_hints.min_width = size_hints.max_width =
       inst->min_width;
   size_hints.height = size_hints.min_height = size_hints.max_height =
       inst->min_height;

   XSetWMProperties(dpy, inst->frame,
		    &w_name_prop, &i_name_prop,
		    argv, 1,
		    &size_hints, NULL, &class_hint);

}

#define XSPACE 12
#define YSPACE 8

void
update_intgeo(XPInstance *inst)
{
   int choc_width, choc_height;
   int actc_width, actc_height;
   int dispc_width, dispc_height;
   int frame_width, frame_height;
   int frame_x, frame_y, frame_depth, frame_border;
   Window frame_root;
   int disp_width = 0, disp_height = 128, cho_height = 96;

   /* get control panel sizes */
   get_cset_size(inst->choctls.cset, &choc_width, &choc_height);
   get_cset_size(inst->actctls.cset, &actc_width, &actc_height);
   get_cset_size(inst->dispctls.cset, &dispc_width, &dispc_height);

   /* work out minimum frame size from them */
   inst->min_width = choc_width +
       MAX(MAX(dispc_width, actc_width) + XSPACE * 3, disp_width + XSPACE * 3);
   inst->min_height = MAX((dispc_height + actc_height
			   + YSPACE * 4 + disp_height),
			  (choc_height + YSPACE * 3 + cho_height));

   /* find out actual current frame size */
   XGetGeometry(dpy, inst->frame, &frame_root,
		&frame_x, &frame_y, &frame_width, &frame_height,
		&frame_border, &frame_depth);

   /* if frame is too small, grow it */
   if (frame_width < inst->min_width || frame_height < inst->min_height) {
      if (frame_width < inst->min_width)
	 frame_width = inst->min_width;
      if (frame_height < inst->min_height)
	 frame_height = inst->min_height;
      XResizeWindow(dpy, inst->frame, frame_width, frame_height);
   }
   /* arrange subwindows */
   XMoveResizeWindow(dpy, inst->chooser.w,
		     XSPACE, YSPACE,
		     choc_width,
		   cho_height = frame_height - YSPACE * 3 - choc_height);
   XMoveResizeWindow(dpy, inst->choctls.w,
		     XSPACE, frame_height - YSPACE - choc_height,
		     choc_width, choc_height);

   XMoveResizeWindow(dpy, inst->wdisp,
		     XSPACE * 2 + choc_width, YSPACE,
		     disp_width = frame_width - XSPACE * 3 - choc_width,
		     disp_height = frame_height - YSPACE * 4 -
		     actc_height - dispc_height);
   XMoveResizeWindow(dpy, inst->dispctls.w,
		     XSPACE * 2 + choc_width, YSPACE * 2 + disp_height,
		     dispc_width, dispc_height);
   XMoveResizeWindow(dpy, inst->actctls.w,
		     XSPACE * 2 + choc_width,
		     YSPACE * 3 + disp_height + dispc_height,
		     actc_width, actc_height);
}

// Local Variables:
// c-basic-offset: 3
// End:
