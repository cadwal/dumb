/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/dumb.c: what do you think?
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/confdef.h"
#include "libdumbutil/confargs.h"
#include "libdumbutil/confeng.h"
#include "libdumbutil/conffile.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbutil/timer.h"
#include "libdumbwad/wadio.h"
#include "libdumb/dsound.h"
#include "libdumb/sound.h"
#include "libdumb/texture.h"
#include "banner.h"
#include "bogothing.h"
#include "draw.h"
#include "fbrerend.h"
#include "game.h"
#include "gettable.h"
#include "input.h"
#include "keymap.h"
#include "keymapconf.h"
#include "levinfo.h"
#include "linetype.h"
#include "net.h"
#include "netplay.h"
#include "render.h"
#include "things.h"
#include "updmap.h"
#include "video.h"

/*#define WATCHDOG */

#ifdef __cplusplus
#define BANNER (PACKAGE "++ " VERSION)
#else
#define BANNER (PACKAGE " " VERSION)
#endif

#ifdef WATCHDOG
static jmp_buf alarm_jb;

static RETSIGTYPE
alarm_handler(int i)
{
   logprintf(LOG_ERROR, 'D',
	     _("got SIGALRM: renderer must have gotten stuck..."));
   longjmp(alarm_jb, 1);
}

#endif

ConfItem mainconf[] =
{
   CONFB("auto-save-cfg", NULL, 0, N_("save configuration after every run")),
   CONFB_NS("save-cfg", NULL, 'S', N_("save configuration this time only")),
   CONFL("wad", NULL, 'w', N_("specify wadfiles to load")),
   CONFL("wadpath", NULL, 'W', N_("directories in which to look for wads")),
   CONFS("doom-wad", NULL, 0, N_("filename of doom.wad"), "doom.wad"),
   CONFS("doom2-wad", NULL, 0, N_("filename of doom2.wad"), "doom2.wad"),
   CONFS("doom4dum-wad", NULL, 0, N_("filename of doom4dum.wad"), "doom4dum.wad"),
   CONFS_L("map", NULL, 'm', N_("set starting map"), "E1M1", 9),
   CONFB("quiet", NULL, 'q', N_("don't log to the screen")),
   CONFB("preview", NULL, 0, N_("don't create any objects, just preview the map")),
   CONFI("difficulty", NULL, 0, N_("set difficulty level"), 3),
   CONFB("silent", NULL, 's', N_("disable sound")),
   CONFI("width", NULL, 'x', N_("video width"), 320),
   CONFI("height", NULL, 'y', N_("video height"), 200),
   CONFI("depth", NULL, 'd', N_("video depth (in bytes/pixel)"), 1),
   CONFS_L("log", NULL, 'l', N_("log to this file"), NULL, 255),
   CONFS_L("record", NULL, 0, N_("record player actions to file"), NULL, 255),
   CONFS_L("play", NULL, 0, N_("playback player actions from file"), NULL, 255),
   CONFB("uncrowd", NULL, 0, N_("hide gettables and banners")),
   CONFB("crosshair", NULL, 0, N_("show a crosshair in the center of the view")),
   CONFI("xmul", NULL, 'X', N_("multiply the pixels x size"), 1),
   CONFI("ymul", NULL, 'Y', N_("multiply the pixels y size"), 1),
   CONFI("xlace", NULL, 0, N_("skip pixels when multiplying x size"), 0),
   CONFI("ylace", NULL, 0, N_("skip pixels when multiplying y size"), 0),
   CONFB("preload", NULL, 0, N_("load textures before game starts")),
   CONFI("view-angle", NULL, 0, N_("angle from forward of player view"), 0),
   CONFI("view-offset", NULL, 0, N_("offset from center of player view"), 0),
   CONFL("slave", NULL, 0, N_("network play: slave mode")),
   CONFL("master", NULL, 0, N_("network play: master mode")),
   CONFB("single", NULL, 0, N_("force single player mode")),
   CONFI("view-rotate", NULL, 0, N_("angle to rotate view by"), 0),
   CONFI("max-frames", NULL, 0, N_("maximum frames to render"), 0),
   CONFITEM_END
};
#define cnf_auto_save (mainconf[0].intval)
#define cnf_save (mainconf[1].intval)
#define cnf_wad (mainconf[2].listval)
#define cnf_wadpath_entry (mainconf[3])
#define cnf_wadpath ((const char *const *) cnf_wadpath_entry.listval)
#define cnf_doom_wad (mainconf[4].strval)
#define cnf_doom2_wad (mainconf[5].strval)
#define cnf_doom4dum_wad (mainconf[6].strval)
#define cnf_map (mainconf[7].strval)
#define cnf_quiet (mainconf[8].intval)
#define cnf_preview (mainconf[9].intval)
#define cnf_difficulty (mainconf[10].intval)
#define cnf_silent (mainconf[11].intval)
#define cnf_width (mainconf[12].intval)
#define cnf_height (mainconf[13].intval)
#define cnf_bpp (mainconf[14].intval)
#define cnf_log (mainconf[15].strval)
#define cnf_record (mainconf[16].strval)
#define cnf_play (mainconf[17].strval)
#define cnf_uncrowd (mainconf[18].intval)
#define cnf_xhair (mainconf[19].intval)
#define cnf_xmul (mainconf[20].intval)
#define cnf_ymul (mainconf[21].intval)
#define cnf_xlace (mainconf[22].intval)
#define cnf_ylace (mainconf[23].intval)
#define cnf_preload (mainconf[24].intval)
#define cnf_vt_angle (mainconf[25].intval)
#define cnf_vt_offset (mainconf[26].intval)
#define cnf_slave (mainconf[27].listval)
#define cnf_master (mainconf[28].listval)
#define cnf_single (mainconf[29].intval)
#define cnf_vt_rotate (mainconf[30].intval)
#define cnf_maxframes (mainconf[31].intval)

ConfModule dumbconf[] =
{
   {input_conf, "input", N_("Input Driver")},
   {video_conf, "vid", N_("Video Driver")},
   {sound_conf, "sound", N_("Sound Driver")},
   {net_conf, "net", N_("Network Driver")},
   {netplay_conf, "netplay", N_("Network Play")},
   {mainconf, "dumb", N_("General Game Control")},
   {playconf, "player", N_("Player Interface")},
   {keymapconf, "keymap", N_("Keyboard Mapping")},
   {NULL, NULL, NULL}
};


/* this should all get moved into game.c */
static int master = 0, slave = 0;
static LevData ld[1];

static int width = 0, height = 0, vwidth = 0, vheight = 0, bpp = 0,
 xmul = 0, ymul = 0, xlace = 0, ylace = 0, rend2fb = 0, real_width = 0;

static int banner = -1, wbanner = -1, bfont = -1;

static void
do_gmsg(const char *s)
{
   add_to_banner(banner, NULL, width, 0);
   add_str_to_banner(banner, bfont, s);
}

void
gmsg(int pl, const char *s)
{
   if (pl < 0 && !slave)
      do_gmsg(s);
   if (pl == ld->localplayer)
      do_gmsg(s);
   else
      send_message(pl, s);
}

static int want_new_lvl = 0, want_quit = 0;

void
game_want_newlvl(int secret)
{
   want_new_lvl = 1 + secret;
}

void
game_want_quit(int x)
{
   (void) x;
   want_quit = 1;
}

void
do_preload(LevData *ld, int bpp)
{
   int i;
   logprintf(LOG_INFO, 'D', _("Preloading wall textures"));
   for (i = 0; i < ldnsides(ld); i++) {
      const SideDyn *sd = ldsided(ld) + i;
      if (sd->mtex)
	 load_texels(sd->mtex, bpp);
      if (sd->ltex)
	 load_texels(sd->ltex, bpp);
      if (sd->utex)
	 load_texels(sd->utex, bpp);
   }
   logprintf(LOG_INFO, 'D', _("Preloading floor/ceiling textures"));
   for (i = 0; i < ldnsectors(ld); i++) {
      const SectorDyn *sd = ldsectord(ld) + i;
      if (sd->ctex)
	 load_texels(sd->ctex, bpp);
      if (sd->ftex)
	 load_texels(sd->ftex, bpp);
   }
   logprintf(LOG_INFO, 'D', _("Preloading sprites"));
   for (i = 0; i < ldnthings(ld); i++) {
      const ProtoThing *proto = ldthingd(ld)[i].proto;
      int j;
      if (proto == NULL || proto->sprite[0] == 0)
	 continue;
      for (j = '1'; j <= '8'; j++) {
	 Texture *t;
	 t = find_phase_sprite(proto, ldthingd(ld)[i].phase, j);
	 if (t)
	    cond_load_texels(t, 1);
      }
   }
}

/* control how much we're allowed to speed up the action */
#define MAXTICKS (500/MSEC_PER_TICK)

int
main(int argc, char **argv)
{
   char *conf_file;
   void *fb, *rendfb = NULL, *fbptr;
   View view;
   ViewTrans viewtrans;
   ThingDyn *td;
   const SectorDyn *sd;
   int tt, frames = 0;
   int i;
   int follow;
   FILE *frec = NULL, *fplay = NULL;
   int want_sound = 1;
   int seen_too_fast_msg = 0;
   int tickspassed = 1;
   int crowd = 1, showgetts = 1;
#ifdef __cplusplus
   void *(*fbrerender) (...);
#else
   void *(*fbrerender) ();
#endif
   int load_failed = 1;
   Texture *txh = NULL;

   /* some default places to look for wads
      this will help make rpm installation easier */
   static const char *builtin_wadpath[]={
      "/usr/share/dumb",
      "/usr/local/share/dumb",
#ifdef DUMB_CONFIG_DOOM_PATH
      DUMB_CONFIG_DOOM_PATH,
#endif
#ifdef DUMB_CONFIG_HERETIC_PATH
      DUMB_CONFIG_HERETIC_PATH,
#endif
      NULL}; 
   const char *const *wadpath;

   INIT_RENDERER_FN *init_renderer = NULL;
   RENDER_FN *render = NULL;

   /* variable definitions end here */

#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */

   /* Always log to stdout until the configuration has been parsed.  */
   log_stdout();

   video_preinit();

#ifdef __MSDOS__
   conf_file = conf_file_name("dumb.cfg");
#else
   conf_file = conf_file_name(".dumbrc");
#endif
   load_failed = load_conf(dumbconf, conf_file);
   if (conf_args(dumbconf, argc, argv))
      return 1;

   /* Finished parsing the configuration.  The stdout log will
    * probably be opened again very soon.  */
   log_close_all();

   if (!cnf_quiet) {
      /*setlinebuf(stdout); *//* if stdout is a socket, we'll want this 
                                 (eg. if you're running xdumb with rsh) */
      log_stdout();
   }
   logprintf(LOG_BANNER, 'D', BANNER);

   if (!load_failed)
      logprintf(LOG_INFO, 'D', _("Read configuration from %s"), conf_file);

   if (cnf_uncrowd)
      crowd = showgetts = 0;
   if (cnf_silent)
      want_sound = 0;
   if (cnf_log)
      log_file(cnf_log, LOG_ALL, NULL);
   if (cnf_record)
      frec = fopen(cnf_record, "wb");
   if (cnf_play)
      fplay = fopen(cnf_record, "rb");
   xmul = cnf_xmul;
   ymul = cnf_ymul;
   xlace = cnf_xlace;
   ylace = cnf_ylace;
   width = cnf_width;
   height = cnf_height;
   viewtrans.angle = (FIXED_PI * cnf_vt_angle) / 180;
   viewtrans.offset = cnf_vt_offset << 12;
#ifdef __alpha
   width -= width % 4;	/* Causes unaligned accesses if not a multiple of 4. */
#endif
   vwidth = width * xmul;
   vheight = height * ymul;
   bpp = cnf_bpp;

   /* if no wadpath specified, use default */
   if(cnf_wadpath==NULL) wadpath=builtin_wadpath;
   else wadpath=cnf_wadpath;

   if (cnf_wad) {
      char **s = cnf_wad;
      init_iwad(*(s++), wadpath);
      while (*s)
	 init_pwad(*(s++), wadpath);
   } else if (*cnf_map == 'E' || *cnf_map == 'e') {
      init_iwad(cnf_doom_wad, wadpath);
      init_pwad(cnf_doom4dum_wad, wadpath);
   } else {
      init_iwad(cnf_doom2_wad, wadpath);
      init_pwad(cnf_doom4dum_wad, wadpath);
   }
   init_wadhashing();

   if (cnf_slave) {
      logprintf(LOG_INFO, 'D', _("network mode: slave"));
      slave = 1;
      net_init();
      net_initmaster(*cnf_slave);
   } else if (cnf_master) {
      char **s = cnf_master;
      logprintf(LOG_INFO, 'D', _("network mode: master"));
      master = 1;
      net_init();
      while (*s) {
	 net_initslave(*s++);
	 master++;
      }
   }
   if (want_sound) {
      init_sound(11025);
      init_dsound();
   }
   init_textures();
   init_linetypes();
   init_levinfo();

   if (slave) {
      wait_slaveinfo(ld);
      load_level(ld, slave_info.mapname,
		 slave_info.difficulty,
		 slave_info.mplayer);
      ld->localplayer = slave_info.plnum;
   } else if (cnf_single)
      load_level(ld, cnf_map, cnf_difficulty, 0);
   else
      load_level(ld, cnf_map, cnf_difficulty, master);
   if (cnf_preload)
      do_preload(ld, 1);
   if (master)
      wait_slaveinit(ld);

   if (vwidth == width && vheight == height) {
      init_video(&vwidth, &vheight, &bpp, &real_width);
      width = vwidth;
      height = vheight;
   } else
      init_video(&vwidth, &vheight, &bpp, &real_width);
   if (xmul == 1 && ymul == 1) {
      rend2fb = 1;
      xlace = ylace = 0;
   } else {
      rendfb = safe_malloc(real_width / xmul * height * bpp);
      if (xlace >= xmul)
	 xlace = xmul - 1;
      if (ylace >= ymul)
	 ylace = ymul - 1;
   }
   video_winstuff(cnf_map, vwidth, vheight);
   cnf_width = vwidth / xmul;
   cnf_height = vheight / ymul;
   cnf_xlace = xlace;
   cnf_ylace = ylace;
   cnf_bpp = bpp;
   switch (bpp) {
#ifdef DUMB_CONFIG_8BPP
   case (1):
      init_renderer = init_renderer8;
      render = render8;
      fbrerender = fbrerender8;
      break;
#endif
#ifdef DUMB_CONFIG_16BPP
   case (2):
      init_renderer = init_renderer16;
      render = render16;
      fbrerender = fbrerender16;
      break;
#endif
#ifdef DUMB_CONFIG_32BPP
   case (4):
      init_renderer = init_renderer32;
      render = render32;
      fbrerender = fbrerender32;
      break;
#endif
   default:
      logfatal('D', _("Unsupported BPP=%d"), bpp);
   }
   keymap_init();
   keymapconf_after_load();
   init_input();
   set_playpal(0, video_setpal);

   if (cnf_preview)
      for (i = 0; i < ldnthings(ld); i++)
	 if (i != ld->player[ld->localplayer])
	    ldthingd(ld)[i].proto = NULL;

   td = ldthingd(ld) + (follow = ld->player[ld->localplayer]);

   init_view(&view);
   view.angle = 0;
   view.x = td->x;
   view.y = td->y;
   if (td->sector == -1)
      logprintf(LOG_ERROR, 'D', _("Can't find sector for player"));
   view.sector = td->sector;
   sd = ldsectord(ld) + td->sector;
   init_renderer(width, height, real_width / xmul, height);
   init_draw(width, height, bpp, real_width / xmul);

   init_gettables();

   banner = init_banner(64, 0, width, 10);
   wbanner = init_banner(height - 12, 0, width, 3);
   if (have_lump("FONTB01"))
      bfont = init_font("FONTB%02d", 64, '!' - 1);
   else
      bfont = init_font("STCFN%03d", 128, 0);
   if (crowd && have_lump("DUMBLOGO"))
      add_to_banner(banner, get_misc_texture("DUMBLOGO"), width, 64);
   if (crowd && have_lump("WAFFLE")) {
      LumpNum w = getlump("WAFFLE");
      if (w == 0)
	 goto skip;		/* This is needed for Digital Unix but not
				   for Linux, weird! */
      add_to_banner(wbanner, NULL, width, 0);
      add_text_to_banner(wbanner, bfont,
			 (const char *) load_lump(w),
			 get_lump_len(w));
   }
 skip:
   if (cnf_xhair)
      txh = get_font_texture(bfont, '+');

   /*
    * if(testarg) {
    * 	 tdraw=get_misc_texture(testarg);
    * 	 logprintf(LOG_DEBUG, 'D', _("texture %s: real=%dx%d log=%dx%d"),
    * 		   tdraw->name, tdraw->width, tdraw->height,
    * 		   1 << tdraw->log2width,
    * 		   1 << tdraw->log2height);
    * }
    */

   /* fill all video pages with our startup screen */
   /*
    * if(have_lump("TITLEPIC")&&have_lump("DUMBLOGO")) {
    * 	 int i;
    * 	 Texture *t1=get_misc_texture("TITLEPIC");
    * 	 Texture *t2=get_misc_texture("DUMBLOGO");
    * 	 for(i=0;i<4;i++) {
    * 	    fb=video_newframe();
    * 	    draw_center(fb,t1);
    * 	    draw_center(fb,t2);
    * 	    video_updateframe(fb);
    * 	 }
    * }
    */

   logprintf(LOG_INFO, 'D', _("starting game"));
   want_quit = 0;
   tt = time(NULL);
   init_timer();
#ifdef WATCHDOG
   if (!setjmp(alarm_jb)) {
      signal(SIGALRM, alarm_handler);
#endif
      while (1) {
	 PlayerInput in;
	 int player_alive = td->hits > 0;
	 if (ldthingd(ld)[follow].proto == NULL)
	    follow = ld->player[ld->localplayer];
	 thing_to_view(ld, follow, &view, &viewtrans);
	 dsound_setview(&view);
	 fb = video_newframe();
	 if (rend2fb)
	    fbptr = fb;
	 else
	    fbptr = rendfb;
	 render(fbptr, ld, &view);
	 if (showgetts)
	    draw_gettables(ld, ld->localplayer, fbptr, width, height);
	 if (tickspassed)
	    update_banners(fbptr, tickspassed);
	 if (txh)
	    draw(fbptr, txh,
		 (width - txh->width) / 2,
		 (height - txh->height) / 2);
	 if (crowd)
	    draw_bogothings(ld, fbptr, width, height);
	 if (!rend2fb)
	    fbptr = fbrerender(rendfb, fb, real_width / xmul, height,
			       xmul, ymul, xlace, ylace);
	 video_updateframe(fb);
	 if (want_sound)
	    poll_sound();
	 if (fplay) {
	    if (feof(fplay)) {
	       logprintf(LOG_INFO, 'D', _("Demo ran out..."));
	       fclose(fplay);
	       fplay = NULL;
	    } else
	       fread(&in, sizeof(in), 1, fplay);
	 }
	 if (fplay == NULL)
	    get_input(&in);
	 if (frec) {
	    fwrite(&in, sizeof(in), 1, frec);
	    fflush(frec);
	 }
	 tickspassed = read_timer();
	 linetype_ticks_passed(tickspassed);

	 if (slave)
	    slave_input(ld, &in, tickspassed);
	 else
	    process_input(ld, &in, tickspassed, ld->localplayer);

	 if (want_quit)
	    break;

	 if (want_new_lvl) {
	    /* we don't want this level's doors to close in the next
	     * level...  */
	    unqueue_all_events(ld);
	    /* tell our slaves to go to the next level */
	    if(master) send_newlvl(want_new_lvl-1);
	    /* go there ourselves */
	    levinfo_next(ld, want_new_lvl > 1);
	    reset_local_gettables(ld);
	    td = ldthingd(ld) + (follow = ld->player[ld->localplayer]);
	    video_winstuff(ld->name, vwidth, vheight);
	    want_new_lvl = 0;
	    continue;
	 }
	 /* check for special functions */
	 if (in.select[9]) {
	    int x, y;
	    unsigned int i;
	    const unsigned char *f = (const unsigned char *) fb;
	    FILE *fout;
	    logprintf(LOG_INFO, 'D', _("saving snapshot..."));
	    fout = fopen("snapshot.ppm", "wb");
	    fprintf(fout, "P6\n%d %d\n%d\n",
		    width, height, bpp == 2 ? 63 : 255);
	    for (y = 0; y < height; y++)
	       for (x = 0; x < width; x++)
		  switch (bpp) {
		  case (4):
		     putc(f[0], fout);
		     putc(f[1], fout);
		     putc(f[2], fout);
		     f += 4;
		     break;
		  case (2):
		     i = f[0] + ((unsigned int) (f[1]) << 8);
		     putc(((i >> 10) & 0x3e) | 1, fout);
		     putc((i >> 5) & 0x3f, fout);
		     putc(((i & 0x1f) << 1) | 1, fout);
		     f += 2;
		     break;
		  case (1):
		     putc(0, fout); /* FIXME */
		     putc(0, fout);
		     putc(0, fout);
		     f++;
		     break;
		  }
	    fclose(fout);
	    logprintf(LOG_INFO, 'D', _("snapshot done"));
	 }
	 /* back to ordinary main loop */
	 if (tickspassed > MAXTICKS) {
	    logprintf(LOG_DEBUG, 'D', _("%d ticks passed with MAXTICKS=%d"),
		      tickspassed, MAXTICKS);
	    tickspassed = MAXTICKS;
	 }
	 if (!slave) {
	    int p;
	    for (p = 0; p < MAXPLAYERS; p++)
	       if (ld->player[p] >= 0)
		  thing_wake_others(ld, ld->player[p], tickspassed);
	 }
	 if (cnf_vt_rotate) {
	    viewtrans.angle += ((FIXED_PI * cnf_vt_rotate * tickspassed)
				/ (180 * 1000 / MSEC_PER_TICK));
	    /* so vt_rotate is in degrees/sec */
	    NORMALIZE_ANGLE(viewtrans.angle);
	 }
	 if (tickspassed > 0) {
	    reset_timer();
	    if (!slave)
	       update_things(ld, tickspassed);
	    update_map(ld, tickspassed);
	    update_gettables(ld, tickspassed);
	    if (master)
	       generate_updates(ld);
	    if (slave)
	       send_sync(1, ld->map_ticks);
	    if (master || slave)
	       netplay_poll(ld);
	 } else if (!(seen_too_fast_msg++)) {
	    logprintf(LOG_ERROR, 'D',
		      _("WARNING: this system is running faster"
			" than the timer resolution"));
	    logprintf(LOG_ERROR, 'D',
		      _("Try running DUMB in a larger window."));
	 }
	 if (player_alive && td->hits <= 0) {
	    gmsg(ld->localplayer, _("YOU DIED.  TOO BAD..."));
	    if (ld->plwep[ld->localplayer] >= 0)
	       ldthingd(ld)[ld->plwep[ld->localplayer]].proto = NULL;
	 }
	 if (master || slave)
	    net_bufflush();
	 frames++;
	 if (cnf_maxframes && frames > cnf_maxframes)
	    break;
      }
#ifdef WATCHDOG
   }
   signal(SIGALRM, SIG_IGN);
#endif
   tt = time(NULL) - tt;
   if (tt)
      logprintf(LOG_INFO, 'D', _("%d frames, %d seconds, %f fps"),
		frames, tt, (double) frames / tt);

   reset_input();
   reset_video();
   if (want_sound) {
      reset_sound();
      reset_dsound();
   }
   reset_gettables();
   free_level(ld);
   reset_levinfo();
   reset_linetypes();
   reset_textures();
   if (master)
      send_slavequit();
   if (master || slave)
      net_reset();
   reset_wad();
   if (fplay)
      fclose(fplay);
   if (frec)
      fclose(frec);
   if (!rend2fb)
      safe_free(rendfb);
   if (cnf_save || cnf_auto_save) {
      logprintf(LOG_INFO, 'D', _("Saving config to %s"), conf_file);
      keymapconf_before_save();
      save_conf(dumbconf, conf_file, DIRT_ARGS);
   }
   safe_free(conf_file);
   log_exit();
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
