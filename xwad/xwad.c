/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/xwad.c: The level editor.
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
#include <dircfg.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <locale.h>

#ifdef HAVE_FORK
#include <signal.h>
#include <sys/wait.h>
#endif /* HAVE_FORK */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"

#include "xwad.h"
#include "colour.h"
#include "disphash.h"

#define MAX_WADS 16

GC mapgc, mapgc2, msggc;
Cursor drag_map_cursor, drag_obj_cursor;
XFontStruct *msgfont, *detailfont, *lilfont;

static XWadInstance inst[1];

/* argv[0] saved near the beginning of main() */
static const char *argv0;

static void print_help(FILE *dest);
static void print_version(void);

#ifdef HAVE_FORK
RETSIGTYPE
sigchld_handler(int i)
{
   int status;
   wait(&status);
   /* logprintf(LOG_DEBUG, 'C', _("child returned %d"), status); */
   qmessage(inst, _("Level Saved."));
   XFlush(dpy);
}
#endif /* HAVE_FORK */

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s [OPTION]... [WADFILE]...\n"
	     "Interactively edit DUMB levels in X11.\n"
	     "\n"), argv0);
   fputs(_("  -w, --load-wad=FILE    load FILE as a WAD.  WADFILE argument does the same.\n"
	   "  -m, --map=MAPNAME      start editing map MAPNAME (default E1M1)\n"
	   "  -d, --display=DISPLAY  use X display DISPLAY\n"
	   "  -l, --log-to=FILE      save messages to FILE\n"
	   "  -v, --verbose          log to the screen\n"
	   "      --help             display this help and exit\n"
	   "      --version          output version information and exit\n"
	   "\n"), dest);
   fputs(_("If no WADFILE nor --load-wad=FILE arguments are given, XWad loads\n"
	   "`doom.wad' if the map name begins with `E' and `doom2.wad' otherwise.\n"
	   "\n"), dest);
   print_bugaddr_message(dest);
}

static void
print_version(void)
{
   static const struct copyright copyrights[] = {
      { "1998", "Josh Parsons" },
      { "1998", "Kalle Niemitalo" },
      COPYRIGHT_END
   };
   fputs("XWad (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

static int
wadmapcmp(const char *wname, const char *mname)
{
   const char *s = strrchr(wname, '/');
   int l = strlen(mname);
   if (s == NULL)
      s = wname;
   else
      s++;
   return strncasecmp(s, mname, l) || strcasecmp(s + l, ".wad");
}

int
main(int argc, char **argv)
{
   int nwads = 0;
   const char *wadf[MAX_WADS];
   int verbose_flag = 0;
   const char *mapname = "E1M1";
   char *dpyname = NULL;
   static const struct option long_options[] =
   {
      { "load-wad", required_argument, NULL, 'w' },
      { "map", required_argument, NULL, 'm' },
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
      int c = getopt_long(argc, argv, "w:m:d:l:v", long_options, NULL);
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
      case 'm':			/* -m, --map=MAPNAME */
	 mapname = optarg;
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
   logprintf(LOG_BANNER, 'M', "XWAD");

   /* start Xlib */
   dpy = XOpenDisplay(dpyname);
   if (dpy == NULL)
      logfatal('M', _("Can't open display %s"), dpyname ? dpyname : _("<default>"));
   root = DefaultRootWindow(dpy);
   screen = DefaultScreen(dpy);
   mapgc = XCreateGC(dpy, root, 0, NULL);
   mapgc2 = XCreateGC(dpy, root, 0, NULL);
   msggc = XCreateGC(dpy, root, 0, NULL);
   init_controls(dpy,
     msgfont = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-*-*-10-*"),
	  lilfont = XLoadQueryFont(dpy, "-misc-fixed-medium-r-*-*-8-*"));
   init_choosers(msgfont);
   detailfont = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-*-*-8-*");
   XSetFont(dpy, mapgc, detailfont->fid);
   XSetFont(dpy, mapgc2, detailfont->fid);
   XSetFont(dpy, msggc, msgfont->fid);
   drag_map_cursor = XCreateFontCursor(dpy, 38);
   drag_obj_cursor = XCreateFontCursor(dpy, 52);

   /* load wads */
   if (nwads > 0) {
      int i = 0;
      init_iwad(wadf[i++], NULL);
      while (i < nwads) {
	 /* if this PWAD has the same name as the level, back it up */
	 /* TODO: delay this until saving for the first time */
	 if (mapname && !wadmapcmp(wadf[i], mapname)) {
	    char *buf = (char *) safe_malloc(strlen(wadf[i]) + 1 + 1);
	    sprintf(buf, "%s~", wadf[i]);
	    rename(wadf[i], buf);
	    init_pwad(buf, NULL);
	    free(buf);
	 }
	 /* OK, load the PWAD */
	 else
	    init_pwad(wadf[i], NULL);
	 i++;
      }
   } else if (*mapname == 'E' || *mapname == 'e')
      init_iwad("doom.wad", NULL);	/* FIXME: from .dumbrc */
   else
      init_iwad("doom2.wad", NULL);

   /* init colormaps */
   init_colour();
   XSetForeground(dpy, msggc, CTLC(MapMessageFg));
   XSetBackground(dpy, msggc, CTLC(MapBg));

   /* init windows & map */
   init_instance(inst);
   if (mapname && *mapname)
      load_instance(inst, mapname);

#ifdef HAVE_FORK
   /* handle dying children */
   signal(SIGCHLD, sigchld_handler);
#endif /* HAVE_FORK */

   /* main loop */
   while (!inst->want_quit) {
      XEvent ev;
      XNextEvent(dpy, &ev);
      /* if there are any notifies lurking, we want the latest */
      if (ev.type == MotionNotify)
	 while (XCheckWindowEvent(dpy, ev.xmotion.window,
			     ButtonMotionMask | PointerMotionMask, &ev));
      /* dispatch events to window */
      dispatch(&ev);
   }

   /* close windows, free map */
   free_instance(inst);

   /* close Xlib */
   XFreeGC(dpy, mapgc);
   XFreeGC(dpy, mapgc2);
   XFreeGC(dpy, msggc);
   reset_choosers();
   reset_controls();
   XFreeFont(dpy, msgfont);
   XFreeFont(dpy, lilfont);
   XFreeFont(dpy, detailfont);
   if (dpy)
      XCloseDisplay(dpy);

   /* done */
   log_exit();
   return 0;
}

static void
mapdhf(XEvent *ev, XWadInstance *inst, void *info)
{
   switch (ev->type) {
   case (ButtonPress):
   case (ButtonRelease):
      /* button event */
      map_button(&ev->xbutton, inst);
      break;
   case (KeyPress):
   case (KeyRelease):
      /* key event */
      break;
   case (MotionNotify):
      /* pointer motion event */
      map_motion(&ev->xmotion, inst);
      break;
   case (Expose):
      /* pull any later exposes from the queue */
      if (ev->xexpose.count > 0)
	 break;
      while (XCheckWindowEvent(dpy, ev->xany.window, ExposureMask, ev));
      /* OK, now we can redraw the map */
      draw_map(inst, inst->map, 0);
      /* and draw any queued messages to the user */
      if (inst->qmsg) {
	 message(inst, inst->qmsg);
	 inst->qmsg = NULL;
      }
      break;
   }
}

static void
framedhf(XEvent *ev, XWadInstance *inst, void *info)
{
   switch (ev->type) {
   case (ConfigureNotify):
      /* frame may have been resized, update internal geometry */
      update_intgeo(inst);
      break;
   case (KeyPress):
      switch (XLookupKeysym(&ev->xkey, 0)) {
      case (XK_V):
	 enter_mode(inst, VerMode);
	 break;
      case (XK_L):
	 enter_mode(inst, LineMode);
	 break;
      case (XK_S):
	 enter_mode(inst, SectMode);
	 break;
      case (XK_T):
	 enter_mode(inst, ThingMode);
	 break;
	 break;
      }
   }
}

#define MAP_BUTMASK ButtonPressMask|ButtonReleaseMask|ButtonMotionMask
#define MAP_MSCMASK ExposureMask
#define MAP_EVMASK MAP_BUTMASK|MAP_MSCMASK

void
init_instance(XWadInstance *inst)
{
   /* clean slate */
   memset(inst, 0, sizeof(XWadInstance));

   /* initial scale = 1:4 */
   inst->scale = 2;

   /* initial gridsize = 64 */
   inst->gridsize = 64;

   /* no selection */
   inst->curselect = -1;

   /* allocate big tables */
#ifdef DUMB_CONFIG_LDWB
   dumblevel_init(&inst->level);
#else  /* !DUMB_CONFIG_LDWB */
   inst->ver = (VertexData *) safe_vmalloc(sizeof(VertexData) * MAXENTS);
   inst->thing = (ThingData *) safe_vmalloc(sizeof(ThingData) * MAXENTS);
   inst->line = (LineData *) safe_vmalloc(sizeof(LineData) * MAXENTS);
   inst->side = (SideData *) safe_vmalloc(sizeof(SideData) * MAXENTS);
   inst->sect = (SectorData *) safe_vmalloc(sizeof(SectorData) * MAXENTS);
#endif /* !DUMB_CONFIG_LDWB */
   inst->enttbl = (EntFlags *) safe_vcalloc(sizeof(EntFlags) * MAXENTS);

   /* create frame & map viewer */
   inst->mapframe = XCreateSimpleWindow(dpy, root, 0, 0,
					8, 8,
					0, BlackPixel(dpy, screen),
					CTLC(Background));

   inst->map = XCreateSimpleWindow(dpy, inst->mapframe, 0, 0,
				   8, 8,
				   1, BlackPixel(dpy, screen),
				   CTLC(MapBg));
   add_dh(inst->map, mapdhf, inst, NULL);
   add_dh(inst->mapframe, framedhf, inst, NULL);

   /* create controls */
   init_cseti(&inst->genctls, inst, inst->mapframe, gen_cset, 1);
   init_cseti(&inst->mapctls, inst, inst->mapframe, map_cset, 1);
   init_cseti(&inst->modectls, inst, inst->mapframe, mode_csets, NumModes);

   /* make it all visible */
   update_intgeo(inst);
   update_wmtitle(inst);
   XMapRaised(dpy, inst->mapframe);
   XSelectInput(dpy, inst->mapframe, KeyPressMask | StructureNotifyMask);
   XMapWindow(dpy, inst->map);
   XSelectInput(dpy, inst->map, MAP_EVMASK);

   /* init Texture chooser */
   init_tchoose(inst);
}

void
free_instance(XWadInstance *inst)
{
   free_tchoose(inst);
   free_cseti(&inst->genctls);
   free_cseti(&inst->mapctls);
   free_cseti(&inst->modectls);
   XDestroyWindow(dpy, inst->mapframe);
#ifdef DUMB_CONFIG_LDWB
   dumblevel_fini(&inst->level);
#else  /* !DUMB_CONFIG_LDWB */
   safe_vfree(inst->ver, sizeof(VertexData) * MAXENTS);
   safe_vfree(inst->thing, sizeof(ThingData) * MAXENTS);
   safe_vfree(inst->line, sizeof(LineData) * MAXENTS);
   safe_vfree(inst->side, sizeof(SideData) * MAXENTS);
   safe_vfree(inst->sect, sizeof(SectorData) * MAXENTS);
#endif /* !DUMB_CONFIG_LDWB */
   safe_vfree(inst->enttbl, sizeof(EntFlags) * MAXENTS);
}

void
load_instance(XWadInstance *inst, const char *mapname)
{
   strncpy(inst->mapname, mapname, 8); /* FIXME: LUMPNAMELEN? */
   inst->mapname[8] = 0;
   strcpy(inst->loadname, inst->mapname);
#ifdef DUMB_CONFIG_LDWB
   dumblevel_fini(&inst->level);
   dumblevel_init_doom(&inst->level, mapname, 3, 0);
#else  /* !DUMB_CONFIG_LDWB */
   inst->thing_ln = safe_lookup_lump("THINGS", mapname, NULL, LOG_FATAL);
   inst->ver_ln = safe_lookup_lump("VERTEXES", mapname, NULL, LOG_FATAL);
   inst->side_ln = safe_lookup_lump("SIDEDEFS", mapname, NULL, LOG_FATAL);
   inst->line_ln = safe_lookup_lump("LINEDEFS", mapname, NULL, LOG_FATAL);
   inst->sect_ln = safe_lookup_lump("SECTORS", mapname, NULL, LOG_FATAL);
   inst->nthings = get_lump_len(inst->thing_ln) / sizeof(ThingData);
   inst->nvers = get_lump_len(inst->ver_ln) / sizeof(VertexData);
   inst->nsides = get_lump_len(inst->side_ln) / sizeof(SideData);
   inst->nlines = get_lump_len(inst->line_ln) / sizeof(LineData);
   inst->nsects = get_lump_len(inst->sect_ln) / sizeof(SectorData);
   memcpy(inst->thing, load_lump(inst->thing_ln),
	  inst->nthings * sizeof(ThingData));
   memcpy(inst->ver, load_lump(inst->ver_ln),
	  inst->nvers * sizeof(VertexData));
   memcpy(inst->side, load_lump(inst->side_ln),
	  inst->nsides * sizeof(SideData));
   memcpy(inst->line, load_lump(inst->line_ln),
	  inst->nlines * sizeof(LineData));
   memcpy(inst->sect, load_lump(inst->sect_ln),
	  inst->nsects * sizeof(SectorData));
   free_lump(inst->ver_ln);
   free_lump(inst->thing_ln);
   free_lump(inst->sect_ln);
   free_lump(inst->line_ln);
   free_lump(inst->side_ln);
#endif /* !DUMB_CONFIG_LDWB */
   update_wmtitle(inst);
#ifdef DUMB_CONFIG_LDWB
   inst->xoffset = inst->level.vertices[0].x;
   inst->yoffset = inst->level.vertices[0].y;
#else  /* !DUMB_CONFIG_LDWB */
   inst->xoffset = inst->ver[0].x;
   inst->yoffset = inst->ver[0].y;
#endif /* !DUMB_CONFIG_LDWB */
   /* force redo *everything* */
   inst->mode = NumModes;
   enter_mode(inst, VerMode);
}

void
update_wmtitle(XWadInstance *inst)
{
   char *argv[2] = { "XWad", NULL };
   static char buf[32];
   char *win_name = buf;
   XTextProperty w_name_prop, i_name_prop;
   XSizeHints size_hints;
   XClassHint class_hint = { "xwad", "XWad" };

   /* inst->mapname is char[10], so buf is big enough. */
   sprintf(buf, "XWad: %s", inst->mapname);

   XStringListToTextProperty(&win_name, 1, &w_name_prop);
   XStringListToTextProperty(&win_name, 1, &i_name_prop);

   size_hints.flags = PMinSize;
   size_hints.width = size_hints.min_width = size_hints.max_width =
       inst->min_width;
   size_hints.height = size_hints.min_height = size_hints.max_height =
       inst->min_height;

   XSetWMProperties(dpy, inst->mapframe,
		    &w_name_prop, &i_name_prop,
		    argv, 1,
		    &size_hints, NULL, &class_hint);

}

#define XSPACE 12
#define YSPACE 8

void
update_intgeo(XWadInstance *inst)
{
   int mapc_width, mapc_height;
   int genc_width, genc_height;
   int modec_width, modec_height;
   int map_width, map_height;
   int frame_x, frame_y;
   unsigned int frame_width, frame_height, frame_depth, frame_border;
   Window frame_root;

   /* get control panel sizes */
   get_cset_size(inst->mapctls.cset, &mapc_width, &mapc_height);
   get_cset_size(inst->genctls.cset, &genc_width, &genc_height);
   get_cset_size(inst->modectls.cset, &modec_width, &modec_height);

   /* work out minimum frame size from them */
   inst->min_width = mapc_width + genc_width + modec_width + XSPACE * 4;
   inst->min_height = MAX(MAX(modec_height, genc_height),
			  mapc_height + 64) + YSPACE * 2;

   /* find out actual current frame size */
   XGetGeometry(dpy, inst->mapframe, &frame_root,
		&frame_x, &frame_y, &frame_width, &frame_height,
		&frame_border, &frame_depth);

   /* if frame is too small, grow it */
   if (frame_width < inst->min_width || frame_height < inst->min_height) {
      if (frame_width < inst->min_width)
	 frame_width = inst->min_width;
      if (frame_height < inst->min_height)
	 frame_height = inst->min_height;
      XResizeWindow(dpy, inst->mapframe, frame_width, frame_height);
   }
   /* arrange subwindows */
   if (frame_height > genc_height + modec_height + YSPACE * 3) {
      int middle = XSPACE * 2 + MAX(genc_width, modec_width);
      XMoveResizeWindow(dpy, inst->genctls.w,
			XSPACE, YSPACE,
			genc_width, genc_height);
      XMoveResizeWindow(dpy, inst->modectls.w,
			XSPACE, YSPACE * 2 + genc_height,
			modec_width, modec_height);
      map_width = frame_width - (middle + XSPACE);
      map_height = frame_height - (mapc_height + YSPACE * 3);
      if (!inst->bigmap)
	 XMoveResizeWindow(dpy, inst->map,
			   middle, YSPACE,
			   map_width,
			   map_height);
      XMoveResizeWindow(dpy, inst->mapctls.w,
	    frame_width - (mapc_width + XSPACE), map_height + YSPACE * 2,
			mapc_width, mapc_height);
   } else {
      int middle = XSPACE * 3 + genc_width + modec_width;
      XMoveResizeWindow(dpy, inst->genctls.w,
			XSPACE, YSPACE,
			genc_width, genc_height);
      XMoveResizeWindow(dpy, inst->modectls.w,
			XSPACE * 2 + genc_width, YSPACE,
			modec_width, modec_height);
      map_width = frame_width - (middle + XSPACE);
      map_height = frame_height - (mapc_height + YSPACE * 3);
      if (!inst->bigmap)
	 XMoveResizeWindow(dpy, inst->map,
			   middle, YSPACE,
			   map_width,
			   map_height);
      XMoveResizeWindow(dpy, inst->mapctls.w,
	    frame_width - (mapc_width + XSPACE), map_height + YSPACE * 2,
			mapc_width, mapc_height);
   }
   if (inst->bigmap) {
      XMoveResizeWindow(dpy, inst->map,
			0, 0,
			inst->map_width = frame_width,
			inst->map_height = frame_height);
   } else {
      inst->map_width = map_width;
      inst->map_height = map_height;
   }
}

int
maxsel(const XWadInstance *inst)
{
   switch (inst->mode) {
#ifdef DUMB_CONFIG_LDWB
   case VerMode:
      return inst->level.vertex_alloc.inited;
   case SectMode:
      return inst->level.sector_alloc.inited;
   case LineMode:
      return inst->level.line_alloc.inited;
   case ThingMode:
      return 0;			/* FIXME */
#else  /* !DUMB_CONFIG_LDWB */
   case (VerMode):
      return inst->nvers;
   case (SectMode):
      return inst->nsects;
   case (LineMode):
      return inst->nlines;
   case (ThingMode):
      return inst->nthings;
#endif /* !DUMB_CONFIG_LDWB */
   }
   return 0;
}

void
enter_mode(XWadInstance *inst, XWadMode mode)
{
   if (inst->mode == mode)
      return;
   memset(inst->enttbl, 0, sizeof(EntFlags) * maxsel(inst));
   inst->mode = mode;
   inst->modectls.cset = mode_csets + mode;
   update_intgeo(inst);
   XClearArea(dpy, inst->map, 0, 0, 0, 0, True);
   XClearArea(dpy, inst->modectls.w, 0, 0, 0, 0, True);
   rdlights_cseti(&inst->genctls, inst);
   new_selection(-1, inst, 0);
}

void
new_selection(int which, XWadInstance *inst, int extend)
{
   int i, n = maxsel(inst);
   char want_redraw = 0, full_redraw = 0;
   /* deal with extend-select */
   if (extend) {
      if (inst->curselect >= 0
	  && !(inst->enttbl[inst->curselect] & ENT_SELECTED)) {
	 inst->enttbl[inst->curselect] |= ENT_SELECTED;
	 want_redraw = 1;
      }
   }
   /* no extend? unselect whatever's selected now */
   else
      for (i = 0; i < n; i++)
	 if (inst->enttbl[i] & ENT_SELECTED) {
	    full_redraw = 1;
	    inst->enttbl[i] ^= ENT_SELECTED;
	 }
   /* new current selection */
   if (which != inst->curselect) {
      if (inst->curselect < 0)
	 want_redraw = 1;
      else
	 full_redraw = 1;
      inst->curselect = which;
      rdoutp_cseti(&inst->modectls, inst);
      rdlights_cseti(&inst->modectls, inst);
   }
   /* want redraw? */
   if (full_redraw)
      XClearArea(dpy, inst->map, 0, 0, 0, 0, True);
   else if (want_redraw)
      draw_map(inst, inst->map, 0);
}

// Local Variables:
// c-basic-offset: 3
// End:
