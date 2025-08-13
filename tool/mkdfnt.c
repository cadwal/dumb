/* DUMB: A Doom-like 3D game engine.
 *
 * tool/mkdfnt.c: Convert X11 fonts to DUMB format.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"

static void print_help(FILE *dest);
static void print_version(void);
static void exit_invalid_args(void) __attribute__((noreturn));
static long parse_long(const char *str, long min, long max);

Display *dpy = NULL;
int screen;
Window root;
Pixmap pix;
GC gc;
XFontStruct *font;
int anti_alias;
const char *argv0;

void
img2pgm(FILE *fout, XImage *img, int dx, int dy)
{
   int x, y;
   fprintf(fout, "P5\n%d %d\n1\n", dx, dy);
   for (y = 0; y < dy; y++)
      for (x = 0; x < dx; x++)
	 putc(XGetPixel(img, x, y), fout);
}

void
img2pgm_aa(FILE *fout, XImage *img, int dx, int dy, int aaf)
{
   int x, y;
   fprintf(fout, "P5\n%d %d\n%d\n", dx / aaf, dy / aaf, aaf);
   for (y = 0; y < dy; y += aaf)
      for (x = 0; x < dx; x += aaf) {
	 int t = 0, ix, iy;
	 for (iy = 0; iy < aaf; iy++)
	    for (ix = 0; ix < aaf; ix++)
	       t += XGetPixel(img, x + ix, y + iy);
	 if (t > aaf)		/* FIXME: should t be first divided by aaf? */
	    putc(aaf, fout);
	 else
	    putc(t, fout);
      }
}

void
do_char(FILE *fout, char ch)
{
   XImage *img;
   XCharStruct xcs;
   int ascent, descent, direction;
   XTextExtents(font, &ch, 1, &direction, &ascent, &descent, &xcs);
   XDrawImageString(dpy, pix, gc, 0, ascent, &ch, 1);
   img = XGetImage(dpy, pix,
		   0, 0,
		   xcs.width, ascent + descent,
		   XYPixmap, 1);
   if (img == NULL)
      fprintf(stderr, _("trouble with XGetImage\n"));
   else if (anti_alias < 2)
      img2pgm(fout, img, xcs.width, ascent + descent);
   else
      img2pgm_aa(fout, img, xcs.width, ascent + descent, anti_alias);
}

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s [OPTION]... --character=C >OUTPUT.pgm\n"
	     "Converts one glyph of an X font to PGM format and outputs it on stdout.\n"
	     "\n"), argv0);
   fputs(_("  -f, --font=FONT  Use X font FONT, which should be quoted to protect\n"
	   "                   it from the shell.  The default is\n"
	   "                   \"-bitstream-charter-bold-r-*-*-60-*-*-*-*-*-iso8859-1\".\n"
	   "  -a, --anti-alias=FACTOR\n"
	   "                   Shrink the bitmap by this factor before saving it.  This\n"
	   "                   results in FACTOR-1 gray levels between black and white.\n"
	   "                   FACTOR must be a positive integer.\n"
	   "  -d, --display=DISPLAY\n"
	   "                   Connect to X server DISPLAY.\n"
	   "  -c, --character=C\n"
	   "                   The character to convert.\n"
	   "      --help       Display this help and exit.\n"
           "      --version    Output version information and exit.\n"
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
   fputs("mkdfnt (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

static void
exit_invalid_args(void)
{
   fprintf(stderr, _("Try `%s --help' for more information.\n"), argv0);
   exit(DUMB_EXIT_INVALID_ARGS);
}

int
main(int argc, char **argv)
{
   const char *dpyname = NULL;
   const char *fontname;
   char c = '\0';		/* actually the default value is never used */

   argv0 = argv[0];

#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */

   if ((argc >= 3 && argc <= 4)
       && argv[1][0] != '-'
       && argv[2][0] != '-'
       && (argc <= 3 || argv[3][0] != '-')) {
      /* Compatibility mode: if there are 2 or 3 parameters, and none
       * of them are options, treat them as FONT C [FACTOR].  */
      fontname = argv[1];
      if (strlen(argv[2]) != 1) {
	 fprintf(stderr, _("%s: Convert one character at a time.\n"), argv0);
	 exit_invalid_args();
      }
      c = argv[2][0];
      if (3 < argc)
	 anti_alias = (int) parse_long(argv[3], 1, 100);
      else
	 anti_alias = 1;
      fprintf(stderr, _("%s: Deprecated usage.  Assuming you meant:\n"),
	      argv0);
      fprintf(stderr, "%s --font=\"%s\"", argv0, fontname);
      if (3 < argc)
	 fprintf(stderr, " --anti-alias=%d", anti_alias);
      /* Find a proper way to quote the character for the shell.  */
      if (c == '\'')
	 fprintf(stderr, " --character=\"'\"\n");
      else
	 fprintf(stderr, " --character='%c'\n", c);
   } else {
      static struct option long_options[] = {
	 { "font", required_argument, NULL, 'f' },
	 { "anti-alias", required_argument, NULL, 'a' },
	 { "display", required_argument, NULL, 'd' },
	 { "character", required_argument, NULL, 'c' },
	 { "help", no_argument, NULL, 'h' }, /* no -h */
	 { "version", no_argument, NULL, 'V' },	/* no -V */
	 { NULL, 0, NULL, '\0' }
      };
      int c_given = 0;
      /* defaults */
      fontname = "-bitstream-charter-bold-r-*-*-60-*-*-*-*-*-iso8859-1";
      anti_alias = 1;
      for (;;) {
	 int opt = getopt_long(argc, argv, "f:a:c:", long_options, NULL);
	 if (opt == -1)
	    break;
	 switch (opt) {
	 case 'f':		/* -f, --font=FONT */
	    fontname = optarg;
	    break;
	 case 'a':		/* -a, --anti-alias=FACTOR */
	    anti_alias = (int) parse_long(optarg, 1, 100);
	    break;
	 case 'd':		/* -d, --display=DISPLAY */
	    dpyname = optarg;
	    break;
	 case 'c':		/* -c, --character=C */
	    if (c_given || strlen(optarg) != 1) {
	       fprintf(stderr, _("%s: Convert one character at a time.\n"), argv0);
	       exit_invalid_args();
	    }
	    c = optarg[0];
	    c_given = 1;
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
      if (!c_given) {
	 fprintf(stderr, _("%s: You must use `--character=C'.\n"), argv0);
	 exit_invalid_args();
      }
   } /* incompatibility mode */

   /* start Xlib */
   dpy = XOpenDisplay(dpyname);
   if (dpy == NULL) {
      printf(_("Can't open display %s"), dpyname ? dpyname : _("<default>"));
      exit(DUMB_EXIT_FOPEN_FAIL); /* close enough */
   }
   root = DefaultRootWindow(dpy);
   screen = DefaultScreen(dpy);
   pix = XCreatePixmap(dpy, root, 320, 320, 1);	/* FIXME: get size from font */
   gc = XCreateGC(dpy, pix, 0, NULL);
   font = XLoadQueryFont(dpy, fontname);
   if (font == NULL) {
      fprintf(stderr, _("%s: font `%s' does not exist\n"), argv0, fontname);
      exit(DUMB_EXIT_FOPEN_FAIL);
   }
   XSetFont(dpy, gc, font->fid);
   XSetForeground(dpy, gc, 1);
   XSetBackground(dpy, gc, 0);

   //fprintf(stderr,"pix=%d font=%d\n",pix,font->fid);

   do_char(stdout, c);

   /* close Xlib */
   XFreePixmap(dpy, pix);
   XCloseDisplay(dpy);

   /* done */
   exit(EXIT_SUCCESS);
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

// Local Variables:
// c-basic-offset: 3
// End:
