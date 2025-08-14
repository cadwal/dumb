/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/ptcomp.c: Compiler for .pt files.
 * Copyright (C) 1998,1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

/* If you add or change keywords, please update ../docs/dumb-pt.texi.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadwr.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "animcomp.h"
#include "fontcomp.h"
#include "fontmapcomp.h"
#include "gettcomp.h"
#include "licomp.h"
#include "ltcomp.h"
#include "msgdomcomp.h"
#include "phasecomp.h"
#include "protocomp.h"
#include "soundcomp.h"

static void show_help(FILE *dest);
static void show_version(void);
static int save_ptlumps_in_dir(const char *outputdir);
static int save_ptlumps_in_wad(const char *wadfname);
static void save_ptlumps_in_wadwr(WADWR *w);

/* initialized at the beginning of main() */
static const char *argv0;

/* declared in globals.h */
int fake_strings_flag = 0;

void
ptcomp(void)
{
   while (1) {
      const char *s = next_token();
      if (s == NULL)
	 break;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "AnimTexture"))
	 animcomp(0);
      else if (!strcasecmp(s, "SwitchTexture")
	       || !strcasecmp(s, "SwTexture"))
	 animcomp(1);
      else if (!strcasecmp(s, "Font"))
	 fontcomp();
      else if (!strcasecmp(s, "FontMapping"))
	 fontmapcomp();
      else if (!strcasecmp(s, "Gettable"))
	 gettcomp();
      else if (!strcasecmp(s, "Level"))
	 licomp();
      else if (!strcasecmp(s, "LineType"))
	 ltcomp(0);
      else if (!strcasecmp(s, "SectorType"))
	 ltcomp(1);
      else if (!strcasecmp(s, "MessageDomain"))
	 msgdomcomp();
      else if (!strcasecmp(s, "PhaseTable"))
	 phasecomp();
      else if (!strcasecmp(s, "Proto"))
	 protocomp();
      else if (!strcasecmp(s, "SoundType"))
	 soundcomp();
      else if (!strcasecmp(s, "SpeedUnits"))
	 change_speed_units();
      else if (!strcasecmp(s, "TimeUnits"))
	 change_time_units();
      else if (!strcasecmp(s, "DefaultSpeed"))
	 change_default_speed();
      else
	 synerr(_("Syntax error"));
   }
}

static void
show_help(FILE *dest)
{
   fprintf(dest,
 _("Usage: %s -o WADFILE\n"
   "   or: %s -d DIRECTORY\n"
   "Compile .pt files to various lumps used by DUMB.  Ptcomp reads .pt source\n"
   "from standard input, and creates the following lumps: PHASES, PROTOS,\n"
   "GETTABLE, LINETYPE, SECTTYPE, SOUNDS, ANIMTEX and LEVINFO.  It saves them\n"
   "either in WADFILE or as *.lump files in DIRECTORY.\n"
   "\n"),
	   argv0, argv0);
   fputs(
 _("  -o, --output-wad=WADFILE          create WADFILE containing the lumps\n"
   "  -d, --output-directory=DIRECTORY  write *.lump files in DIRECTORY\n"
   "      --fake-strings                translate name tokens to strings\n"
   "                                    where needed\n"
   "      --help                        display this help and exit\n"
   "      --version                     output version information and exit\n"
   "\n"), dest);
   print_bugaddr_message(dest);
}

static void
show_version(void)
{
   static const struct copyright copyrights[] = {
      { "1997-1998", "Josh Parsons" },
      { "1998-1999", "Kalle Niemitalo" },
      COPYRIGHT_END
   };
   printf("ptcomp (DUMB) " VERSION "\n");
   print_copyrights(copyrights);
   printf(_("This program is free software; you may redistribute it under the terms of\n"
	    "the GNU General Public License.  This program has absolutely no warranty.\n"));
}

int
main(int argc, char **argv)
{
   const char *outputdir = NULL;
   const char *wadfname = NULL;
   argv0 = argv[0];
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   /* Compatibility mode: if there is only one parameter and it
    * isn't an option, treat it as the output directory name.  */
   if (argc == 2
       && argv[1][0] != '-') {
      outputdir = argv[1];
      fprintf(stderr, _("%s: Deprecated usage.  Assuming you meant:\n"),
	      argv0);
      fprintf(stderr, "%s --output-directory=%s\n", argv0, outputdir);
   } else {
      static const struct option long_options[] = {
	 { "output-wad", required_argument, NULL, 'o' },
	 { "output-directory", required_argument, NULL, 'd' },
	 { "fake-strings", no_argument, NULL, 'f' }, /* no -f */
	 { "help", no_argument, NULL, 'h' }, /* no -h */
	 { "version", no_argument, NULL, 'V' }, /* no -V */
	 { NULL, 0, NULL, '\0' }
      };
      for (;;) {
	 int c = getopt_long(argc, argv, "o:d:", long_options, NULL);
	 if (c == -1)
	    break;
	 switch (c) {
	 case 'o':		/* -o, --output-wad=WADFILE */
	    wadfname = optarg;
	    break;
	 case 'd':		/* -d, --output-directory=DIRECTORY */
	    outputdir = optarg;
	    break;
	 case 'f':		/*     --fake-strings */
	    fake_strings_flag = 1;
	    break;
	 case 'h':		/*     --help */
	    show_help(stdout);
	    exit(EXIT_SUCCESS);
	 case 'V':		/*     --version */
	    show_version();
	    exit(EXIT_SUCCESS);
	 case '?':		/* invalid option */
	    fprintf(stderr, _("Try `%s --help' for more information.\n"),
		    argv0);
	    exit(DUMB_EXIT_INVALID_ARGS);
	 } /* switch */
      }	/* for ever */
      if (!outputdir && !wadfname) {
	 fprintf(stderr,
		 _("%s: You must use `-d DIRECTORY' or `-o WADFILE'\n"),
		 argv0);
	 fprintf(stderr, _("Try `%s --help' for more information.\n"),
		 argv0);
	 exit(DUMB_EXIT_INVALID_ARGS);
      }
   } /* incompatibility mode */
   log_stdout();		/* for error messages from safem */
   init_animcomp();
   init_fontcomp();
   init_fontmapcomp();
   init_gettcomp();
   init_licomp();
   init_ltcomp();
   init_msgdomcomp();
   init_phasecomp();
   init_protocomp();
   init_soundcomp();

   begin_file(stdin, _("stdin"));
   printf(_("%s: compiling...\n"), argv0);
   ptcomp();
   end_file();
   
   {
      int error_flag = 0;

      /* Undocumented feature: -d and -o can be used together.
       * This hasn't been tested, though... */
      if (outputdir != NULL)
	 error_flag |= save_ptlumps_in_dir(outputdir);
      if (wadfname != NULL)
	 error_flag |= save_ptlumps_in_wad(wadfname);

      return error_flag ? EXIT_FAILURE : EXIT_SUCCESS;
   }
}

static int
save_ptlumps_in_dir(const char *outputdir)
{
   WADWR *wadwr = wadwr_open(outputdir, 'd');
   if (!wadwr)
      return 1;
   save_ptlumps_in_wadwr(wadwr);
   return wadwr_close(wadwr);
}

static int
save_ptlumps_in_wad(const char *wadfname)
{
   WADWR *wadwr = wadwr_open(wadfname, 'p');
   if (!wadwr)
      return 1;
   save_ptlumps_in_wadwr(wadwr);
   return wadwr_close(wadwr);
}

static void
save_ptlumps_in_wadwr(WADWR *w)
{
   wranims(w);
   wrfonts(w);
   wrfontmap(w);
   wrgetts(w);
   wrlinfos(w);
   wrlts(w); wrsts(w);
   wrmsgdom(w);
   wrphases(w);
   wrprotos(w);
   wrsounds(w);
}

// Local Variables:
// c-basic-offset: 3
// End:
