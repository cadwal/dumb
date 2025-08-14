/* DUMB: A Doom-like 3D game engine.
 *
 * tool/dark2trans.c: Converts dark pixels to #000001 which means transparent.
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#ifdef __cplusplus
extern "C" {
# include DUMB_CONFIG_PPM_H
}
#else
# include DUMB_CONFIG_PPM_H
#endif

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"

static void print_help(FILE *dest);
static void print_version(void);
static long parse_long(const char *str, long min, long max);
static void exit_invalid_args(void);

static const char *argv0;	/* initialised by main() */

int
main(int argc, char **argv)
{
   pixel **pix;
   int cols, rows;
   pixval maxval;
   int x, y;
   /* MINVAL is actually always taken from the parameters but here we
    * initialize it as -1 to comfort the compiler.  */
   int minval = -1;
   argv0 = argv[0];
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   /* Compatibility mode: if there is only one parameter and it
    * isn't an option, treat it as the minimum brightness.  */
   if (argc == 2
       && argv[1][0] != '-') {
      minval = parse_long(argv[1], 0, PPM_MAXMAXVAL) * 3;
      fprintf(stderr, _("%s: Deprecated usage.  Assuming you meant:\n"),
	      argv0);
      fprintf(stderr, "%s --min=%d\n", argv0, (int) minval / 3);
   } else {
      static const struct option long_options[] = {
	 { "min", required_argument, NULL, 'm' },
	 { "help", no_argument, NULL, 'h' }, /* no -h */
	 { "version", no_argument, NULL, 'V' },	/* no -V */
	 { NULL, 0, NULL, '\0' }
      };
      int minval_given = 0;
      for (;;) {
	 int c = getopt_long(argc, argv, "m:", long_options, NULL);
	 if (c == -1)
	    break;
	 switch (c) {
	 case 'm':		/* -m, --min=BRIGHTNESS */
	    minval = parse_long(optarg, 0, PPM_MAXMAXVAL) * 3;
	    minval_given = 1;
	    break;
	 case 'h':		/*     --help */
	    print_help(stdout);
	    exit(EXIT_SUCCESS);
	 case 'V':		/*     --version */
	    print_version();
	    exit(EXIT_SUCCESS);
	 case '?':		/* invalid option */
	    exit_invalid_args();
	 } /* switch */
      }	/* for ever */
      if (!minval_given) {
	 fprintf(stderr, _("%s: You must use `--min=BRIGHTNESS'\n"), argv0);
	 exit_invalid_args();
      }
   } /* incompatibility mode */
   pix = ppm_readppm(stdin, &cols, &rows, &maxval);
   for (y = 0; y < rows; y++)
      for (x = 0; x < cols; x++) {
	 pixel *p = pix[y] + x;
	 if (p->r + p->g + p->b < minval) {
	    p->r = p->g = 0;
	    p->b = 1;
	 }
      }
   ppm_writeppm(stdout, pix, cols, rows, maxval, 0);
   exit(EXIT_SUCCESS);
}

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s --min=BRIGHTNESS <INPUT.ppm >OUTPUT.ppm\n"
	     "Read a PPM-format picture from standard input and convert all dark pixels\n"
	     "to R=0,G=0,B=1 which ppmtodumb recognizes as meaning transparent.  Write the\n"
	     "resulting image to standard output.\n"
	     "\n"), argv0);
   fputs(_("  -m, --min=BRIGHTNESS  If a pixel is darker than BRIGHTNESS, it is made\n"
	   "                        transparent.  BRIGHTNESS must be an integer.\n"
	   "                        Its range depends on the input data.\n"
	   "      --help            Display this help and exit.\n"
	   "      --version         Output version information and exit.\n"
	   "\n"), dest);
   print_bugaddr_message(dest);
}

static void
print_version(void)
{
   static const struct copyright copyrights[] = {
      { "1998", "Josh Parsons" },
      { "1998-1999", "Kalle Niemitalo" },
      COPYRIGHT_END
   };
   fputs("dark2trans (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

/* This function can be used to parse int and short int values too: if
 * min and max are in the range of the type you want, so will the
 * return value. 
 *
 * TODO: move to libdumbutil?  But then argv0 and exit_invalid_args()
 * should be there too.  */ 
static long
parse_long(const char *str, long min, long max)
{
   char *tail;
   long value;
   int strtol_errno;
   errno = 0;
   value = strtol(str, &tail, 0);
   strtol_errno = errno;
   if (tail == str || *tail != '\0') {
      /* strtol() couldn't parse it at all, or there is some garbage
       * after it.  Note that str may be empty, in which case
       * *tail=='\0' but tail==str.  */
      fprintf(stderr, _("%s: invalid integer `%s'\n"), argv0, str);
      exit_invalid_args();
   }
   if (strtol_errno != 0
       || value < min || value > max) {
      fprintf(stderr, _("%s: integer `%s' should be between %ld and %ld\n"),
	      argv0, str, min, max);
      exit_invalid_args();
   }
   return value;
}

static void
exit_invalid_args(void)
{
   fprintf(stderr, _("Try `%s --help' for more information.\n"), argv0);
   exit(DUMB_EXIT_INVALID_ARGS);
}

// Local Variables:
// c-basic-offset: 3
// End:
