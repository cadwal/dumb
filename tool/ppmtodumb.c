/* DUMB: A Doom-like 3D game engine.
 *
 * tool/ppmtodumb.c: Convert ppm-format images to DUMB formats.
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

/* FIXME: more error checking */

#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
#include "libdumbwad/wadstruct.h"

typedef enum {
   NoMode,
   MkFlat, MkPatch,
   MkColormap, MkPlayPal
} Mode;
Mode mode = NoMode;

/* Whether holes are allowed in the texture.  Always changed together
 * with mode. */
int transparent_flag;

/* Flag set by --id-compatible */
int compat_flag = 0;

/* Bytes per pixel: 1, 2 or 4.  0 means undefined and is changed to 1
 * after the arguments have been parsed.  */
int bytes_per_pixel = 0;

/* argv[0] saved by parse_args() */
const char *argv0;

const char *playpal_fn = "data/PLAYPAL.lump";
unsigned char playpal[256 * 3];

static void parse_args(int argc, char **argv);
static void set_mode(Mode new_mode, int new_transflag);
static void set_bytes(int new_bytes);
static void print_help(FILE *dest);
static void print_version();

static void exit_invalid_args(void)
     __attribute__((noreturn));
static void exit_invalid_data(const char *message)
     __attribute__((noreturn));
static void exit_out_of_memory(void)
     __attribute__((noreturn));

static void parse_args(int argc, char **argv)
{
   static const struct option long_options[] = {
      { "flat", no_argument, NULL, 'f' },
      { "patch", no_argument, NULL, 'p' },
      { "opaque-patch", no_argument, NULL, 'P' },
      { "playpal", no_argument, NULL, 'm' },
      { "colormap", no_argument, NULL, 'c' },
      { "use-playpal", required_argument, NULL, 'M' },
      { "bytes-per-pixel", required_argument, NULL, 'b' }, /* no -b */
      { "id-compatible", no_argument, NULL, 'i' },
      { "help", no_argument, NULL, 'h' }, /* no -h */
      { "version", no_argument, NULL, 'V' }, /* no -V */
      { NULL, 0, NULL, '\0' }
   };
   argv0 = argv[0];
   for (;;) {
      int c = getopt_long(argc, argv, "M:mc24fpPi", long_options, NULL);
      if (c == -1)
	 break;			/* end of options */
      switch (c) {
      case 'f':			/* -f, --flat */
	 set_mode(MkFlat, 0);
	 break;
      case 'p':			/* -p, --patch */
	 set_mode(MkPatch, 1);
	 break;
      case 'P':			/* -P, --opaque-patch */
	 set_mode(MkPatch, 0);
	 break;
      case 'm':			/* -m, --playpal */
	 set_mode(MkPlayPal, 0);
	 break;
      case 'c':			/* -c, --colormap */
	 set_mode(MkColormap, 0);
	 break;
      case 'M':			/* -M, --use-playpal=PLAYPAL */
	 playpal_fn = optarg;
	 break;
      case 'b':			/*     --bytes-per-pixel=NUMBER */
	 {
	    char *tail;
	    /* TODO: Switch temporarily back to "C" locale?  */
	    long value = strtol(optarg, &tail, 0);
	    if (*tail != '\0'
		|| !(value==1 || value==2 || value==4)) {
	       fprintf(stderr, _("%s: invalid bytes-per-pixel value `%s';"
				 " must be 1, 2 or 4\n"),
		       argv0, optarg);
	       exit_invalid_args();
	    }
	    set_bytes(value);
	 }
	 break;
      case '2':			/* -2 */
	 set_bytes(2);
	 break;
      case '4':			/* -4 */
	 set_bytes(4);
	 break;
      case 'i':			/* -i, --id-compatible */
	 compat_flag = 1;
	 break;
      case 'h':			/*     --help */
	 print_help(stdout);
	 exit(EXIT_SUCCESS);
      case 'V':			/*     --version */
	 print_version();
	 exit(EXIT_SUCCESS);
      case '?':			/* invalid switch */
	 /* getopt_long() already printed an error message. */
	 exit_invalid_args();
      }	/* switch c */
   } /* for ever */
   if (bytes_per_pixel == 0)
      bytes_per_pixel = 1;
   if (mode == NoMode) {
      fprintf(stderr,		/* as in tar */
	      _("%s: You must specify one of the `-fpPmc' options\n"),
	      argv0);
      exit_invalid_args();
   }
   if (argc > optind) {
      fprintf(stderr, _("%s: Non-option arguments are not allowed\n"),
	      argv0);
      exit_invalid_args();
   }
}

static void
set_mode(Mode new_mode, int new_transflag)
{
   if (mode == NoMode) {
      mode = new_mode;
      transparent_flag = new_transflag;
   } else {
      fprintf(stderr,		/* as in tar */
	      _("%s: You may not specify more than one `-fpPmc' option\n"),
	      argv0);
      exit_invalid_args();
   }
}

static void
set_bytes(int new_bytes)
{
   if (bytes_per_pixel == 0)
      bytes_per_pixel = new_bytes;
   else {
      fprintf(stderr,
	      _("%s: Only one bytes-per-pixel number is allowed\n"),
	      argv0);
      exit_invalid_args();
   }
}

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s [OPTION]... <INPUT.ppm >OUTPUT.lump\n"
	     "Converts graphic files from the portable pixmap format (PPM) to DUMB's\n"
	     "internal formats.\n"
	     "\n"), argv0);
   fprintf(dest,
	   _("  -f, --flat            create a 64x64 flat-format graphic\n"
	     "  -p, --patch           create a transparent patch\n"
	     "  -P, --opaque-patch    create a non-transparent patch\n"
	     "  -m, --playpal         create a playpal\n"
	     "  -c, --colormap        create a colormap\n"
	     "  -M, --use-playpal=PLAYPAL\n"
	     "                        read the playpal data from PLAYPAL\n"
	     "                        (default: %s)\n"
	     "      --bytes-per-pixel=NUMBER\n"
	     "                        how many bytes per pixel: 1 (default), 2 or 4\n"
	     "  -2                    same as --bytes-per-pixel=2\n"
	     "  -4                    same as --bytes-per-pixel=4\n"
	     "  -i, --id-compatible   turn on id compatibility\n"
	     "      --help            display this help and exit\n"
	     "      --version         output version information and exit\n"
	     "\n"), playpal_fn);
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
   fputs("ppmtodumb (DUMB) " VERSION "\n", stdout);
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

static void
exit_invalid_data(const char *message)
{
   fprintf(stderr, "%s: %s\n", argv0, message);
   exit(DUMB_EXIT_INVALID_DATA);
}

static void
exit_out_of_memory(void)
{
   fprintf(stderr, _("%s: out of memory\n"), argv0);
   exit(DUMB_EXIT_OUT_OF_MEMORY);
}

static int
mylog2(int i)
{
   int j = 0;
   while (j < 32 && i > (1 << j))
      j++;
   return j;
}

void
loadpal(void)
{
   FILE *f = fopen(playpal_fn, "rb");
   if (f == NULL) {
      fprintf(stderr, "%s: %s: %s\n", argv0, playpal_fn, strerror(errno));
      exit(DUMB_EXIT_FOPEN_FAIL);
   }
   fread(playpal, 3, 256, f);
   fclose(f);
}

#define SQ(i) (((double)(i))*((double)(i)))
unsigned char
lookup_closest(int r, int g, int b)
{
   int i;
   double mindist = 65536.0 * 3.0;
   int minent = 255;
   for (i = 0; i < 256; i++) {
      const unsigned char *pp = playpal + (i * 3);
      double dist = 0.0;
      dist += SQ(r - pp[0]);
      dist += SQ(g - pp[1]);
      dist += SQ(b - pp[2]);
      if (dist < mindist) {
	 minent = i;
	 mindist = dist;
      }
      if (mindist < 1.0)
	 break;
   }
   return minent;
}

unsigned char
lookup_pal(pixel p, pixval maxval)
{
   int i;
   float mvf = maxval + 1;
   /* check for transparent */
   if (transparent_flag && p.r == 0 && p.g == 0 && p.b == 1)
      return 255;
   /* fix up depth */
   if (maxval != 255) {
      p.r = (int) (((float) p.r / mvf) * 256.0);
      p.g = (int) (((float) p.g / mvf) * 256.0);
      p.b = (int) (((float) p.b / mvf) * 256.0);
   }
   /* normal lookup */
   for (i = 0; i < 256; i++) {
      const unsigned char *pp = playpal + (i * 3);
      if (p.r == pp[0] && p.g == pp[1] && p.b == pp[2])
	 return i;
   }
   /*warn(_("pixel matched none in palette"));
      return 255; */
   return lookup_closest(p.r, p.g, p.b);
}

void
mkflat(void)
{
   int cols, rows, x, y;
   pixval maxval;
   pixel **pix;
   loadpal();
   pix = ppm_readppm(stdin, &cols, &rows, &maxval);
   if (pix == NULL)
      exit_invalid_data(_("bad ppm"));
   if ((cols == 64 && rows == 64)
       || (compat_flag == 0 && cols == 128 && rows == 128))
      for (y = 0; y < rows; y++)
	 for (x = 0; x < cols; x++)
	    putchar(lookup_pal(pix[y][x], maxval));
   else
      exit_invalid_data(_("this ppm is the wrong size for a flat"));
}

void
mkpatch(void)
{
   PictData *p;
   int cols, rows, x, y, o;
   pixval maxval;
   pixel **pix;
   loadpal();
   pix = ppm_readppm(stdin, &cols, &rows, &maxval);
   if (pix == NULL)
      exit_invalid_data(_("bad ppm"));
   p = (PictData *) malloc(8 + cols * 4 + (rows + 4) * cols);
   if (p == NULL)
      exit_out_of_memory();
   p->UMEMB(hdr).width = cols;
   p->UMEMB(hdr).height = rows;
   p->UMEMB(hdr).xoffset = p->UMEMB(hdr).yoffset = 0;
   o = 8 + cols * 4;
   for (x = 0; x < cols; x++) {
      p->UMEMB(hdr).idx[x] = o;
      p->data[o++] = 0;		/* initial row */
      p->data[o++] = rows;	/* # of rows */
      p->data[o++] = 0;
      for (y = 0; y < rows; y++)
	 p->data[o++] = lookup_pal(pix[y][x], maxval);
      p->data[o++] = 0;
      p->data[o++] = 0xff;	/* no more posts in this column */
   }
   fwrite(p, 1, o, stdout);
}

void
mkjpatch(void)
{
   LE_int32 lebuf;
   int cols, rows, x, y, ph;
   pixval maxval;
   pixel **pix;
   loadpal();
   pix = ppm_readppm(stdin, &cols, &rows, &maxval);
   if (pix == NULL)
      exit_invalid_data(_("bad ppm"));
   putchar('J');
   putchar('1');
   putchar(mylog2(cols));
   putchar(ph = mylog2(rows));
   lebuf = cols;
   fwrite(&lebuf, sizeof(int), 1, stdout);
   lebuf = rows;
   fwrite(&lebuf, sizeof(int), 1, stdout);
   ph = 1 << ph;
   for (x = 0; x < cols; x++)
      for (y = 0; y < ph; y++)
	 if (y < rows)
	    putchar(lookup_pal(pix[(rows - 1) - y][(cols - 1) - x], maxval));
	 else
	    putchar(255);
}

void
mkpal(void)
{
   int cols, rows, x, y, i = 0;
   pixval maxval;
   pixel **pix;
   pix = ppm_readppm(stdin, &cols, &rows, &maxval);
   if (pix == NULL)
      exit_invalid_data(_("bad ppm"));
   for (y = 0; y < rows; y++)
      for (x = 0; x < cols; x++) {
	 if (i >= 256)
	    break;
	 putchar((unsigned char) (pix[y][x].r));
	 putchar((unsigned char) (pix[y][x].g));
	 putchar((unsigned char) (pix[y][x].b));
	 i++;
      }
   while (i++ < 256) {
      putchar(0);
      putchar(0);
      putchar(0);
   }
}

static unsigned int
pack_colour(unsigned int r, unsigned int g, unsigned int b)
{
   unsigned int p;
   p = b >> 3;			/* ........ ...BBBBB */
   p |= (g >> 2) << 5;		/* .....GGG GGG..... */
   p |= (r >> 3) << 11;		/* RRRRR... ........ */
   return p;			/* RRRRRGGG GGGBBBBB */
}

#define NCMAPS 30
void
mkcmap()
{
   int i, j;
   if (bytes_per_pixel != 1 &&
       bytes_per_pixel != 2 &&
       bytes_per_pixel != 4)
      abort();			/* should have been checked by parse_args() */
   loadpal();
   for (i = 0; i <= NCMAPS; i++) {
      for (j = 0; j < 256; j++) {
	 unsigned int r = playpal[3 * j];
	 unsigned int g = playpal[3 * j + 1];
	 unsigned int b = playpal[3 * j + 2];
	 r = (r * (NCMAPS - i)) / NCMAPS;
	 g = (g * (NCMAPS - i)) / NCMAPS;
	 b = (b * (NCMAPS - i)) / NCMAPS;
	 switch (bytes_per_pixel) {
	 case 1:
	    putchar(lookup_closest(r, g, b));
	    break;
	 case 2: {
	       unsigned int p = pack_colour(r, g, b);
	       putchar(p & 0xff);
	       putchar(p >> 8);
	    } break;
	 case 4:
	    putchar(b);
	    putchar(g);
	    putchar(r);
	    putchar(0);
	    break;
	 } /* switch */
      }	/* for j */
   } /* for i */
}

int
main(int argc, char **argv)
{
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   parse_args(argc, argv);
   /* don't need to be interactive */
   setvbuf(stdout, NULL, _IOFBF, 4096);
   setvbuf(stdin, NULL, _IOFBF, 4096);
   /* do the conversion */
   switch (mode) {
   case MkFlat:
      mkflat();
      break;
   case MkPatch:
      if (compat_flag)
	 mkpatch();
      else
	 mkjpatch();
      break;
   case MkPlayPal:
      mkpal();
      break;
   case MkColormap:
      mkcmap();
      break;
   default:
      abort();
   }
   exit(EXIT_SUCCESS);
}

// Local Variables:
// c-basic-offset: 3
// End:
