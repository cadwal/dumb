/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/ptcomp.c: Compiler for .pt files.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

/* If you add/change keywords, please update the list at the bottom of
 * ../docs/README.ptcomp.  You don't have to write an explanation if
 * you don't want.  */

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

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "animcomp.h"
#include "gettcomp.h"
#include "licomp.h"
#include "ltcomp.h"
#include "phasecomp.h"
#include "protocomp.h"
#include "soundcomp.h"

static void show_help(FILE *dest);
static void show_version(void);

/* initialized at the beginning of main() */
static const char *argv0;

void
ptcomp(void)
{
   while (1) {
      const char *s = next_token();
      if (s == NULL)
	 break;
      else if (*s == '\n');
      else if (!strcasecmp(s, "Proto"))
	 protocomp();
      else if (!strcasecmp(s, "PhaseTable"))
	 phasecomp();
      else if (!strcasecmp(s, "Gettable"))
	 gettcomp();
      else if (!strcasecmp(s, "LineType"))
	 ltcomp(0);
      else if (!strcasecmp(s, "SectorType"))
	 ltcomp(1);
      else if (!strcasecmp(s, "SoundType"))
	 soundcomp();
      else if (!strcasecmp(s, "AnimTexture"))
	 animcomp(0);
      else if (!strcasecmp(s, "SwitchTexture"))
	 animcomp(1);
      else if (!strcasecmp(s, "SwTexture"))
	 animcomp(1);
      else if (!strcasecmp(s, "Level"))
	 licomp();
      else if (!strcasecmp(s, "TimeUnits"))
	 change_time_units();
      else if (!strcasecmp(s, "DefaultSpeed"))
	 change_default_speed();
      else
	 synerr(NULL);
   }
}

static void
show_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s -d DIRECTORY\n"
	     "Compile .pt files to various lumps used by DUMB.  Ptcomp reads .pt source\n"
	     "from standard input, and creates the following files in DIRECTORY:\n"
	     "PHASES.lump, PROTOS.lump, GETTABLE.lump, LINETYPE.lump, SECTTYPE.lump,\n"
	     "SOUNDS.lump, ANIMTEX.lump and LEVINFO.lump.\n"
	     "\n"), argv0);
   fputs(_("  -d, --output-directory=DIRECTORY  write the output files to DIRECTORY\n"
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
      { "1998", "Kalle O. Niemitalo" },
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
   FILE *fout;
   char *foutname;
   const char *dir = NULL;
   int errors = 0;
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
      dir = argv[1];
      fprintf(stderr, _("%s: Deprecated usage.  Assuming you meant:\n"),
	      argv0);
      fprintf(stderr, "%s -o %s\n", argv0, dir);
   } else {
      static const struct option long_options[] = {
	 { "output-directory", required_argument, NULL, 'd' },
	 { "help", no_argument, NULL, 'h' }, /* no -h */
	 { "version", no_argument, NULL, 'V' }, /* no -V */
	 { NULL, 0, NULL, '\0' }
      };
      for (;;) {
	 int c = getopt_long(argc, argv, "d:", long_options, NULL);
	 if (c == -1)
	    break;
	 switch (c) {
	 case 'd':			/* -d, --output-directory */
	    dir = optarg;
	    break;
	 case 'h':			/*     --help */
	    show_help(stdout);
	    exit(EXIT_SUCCESS);
	 case 'V':			/*     --version */
	    show_version();
	    exit(EXIT_SUCCESS);
	 case '?':		/* invalid option */
	    fprintf(stderr, _("Try `%s --help' for more information.\n"),
		    argv0);
	    exit(DUMB_EXIT_INVALID_ARGS);
	 } /* switch */
      }	/* for ever */
      if (!dir) {
	 fprintf(stderr, _("%s: You must use `-o DIRECTORY'\n"), argv0);
	 fprintf(stderr, _("Try `%s --help' for more information.\n"),
		 argv0);
	 exit(DUMB_EXIT_INVALID_ARGS);
      }
   } /* incompatibility mode */
   log_stdout();		/* for error messages from safem */
   foutname = (char *) safe_malloc(strlen(dir) + strlen("/LINETYPE.lump") + 1);
   init_animcomp();
   init_gettcomp();
   init_licomp();
   init_ltcomp();
   init_phasecomp();
   init_protocomp();
   init_soundcomp();

   begin_file(stdin, _("stdin"));
   printf(_("%s: compiling...\n"), argv0);
   ptcomp();
   end_file();

   printf(_("%s: writing...\n"), argv0);
   fout = fopen(strcat(strcpy(foutname, dir), "/PHASES.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrphases(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/PROTOS.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrprotos(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/GETTABLE.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrgetts(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/LINETYPE.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrlts(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/SECTTYPE.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrsts(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/SOUNDS.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrsounds(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/ANIMTEX.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wranims(fout);
      fclose(fout);
   }
   fout = fopen(strcat(strcpy(foutname, dir), "/LEVINFO.lump"), "wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrlinfos(fout);
      fclose(fout);
   }
   return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}

// Local Variables:
// c-basic-offset: 3
// End:
