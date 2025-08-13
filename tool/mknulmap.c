/* DUMB: A Doom-like 3D game engine.
 *
 * tool/mknulmap.c: Generate a map consisting of just one room.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadwr.h"
#include "libdumbwad/wadstruct.h"

static void print_help(FILE *dest);
static void print_version(void);
static void exit_invalid_args(void);

const char *argv0;

/* a quick hack, if ever there was one */

#define NVERS 4
VertexData ver[NVERS] =
{
   {0, 0},
   {0, 512},
   {512, 512},
   {512, 0}
};

#define NLINES 4
LineData line[NLINES] =
{
   {0, 1, LINE_IMPASSIBLE, 0, -1, {0, -1}},
   {1, 2, LINE_IMPASSIBLE, 0, -1, {0, -1}},
   {2, 3, LINE_IMPASSIBLE, 0, -1, {0, -1}},
   {3, 0, LINE_IMPASSIBLE, 0, -1, {0, -1}}
};

#define NSIDES 4
SideData side[NSIDES] =
{
   {0, 0, "", "", "STARG3", 0},
   {0, 0, "", "", "STARG3", 0},
   {0, 0, "", "", "STARG3", 0},
   {0, 0, "", "", "STARG3", 0}
};

#define NSECTS 1
SectorData sect[NSECTS] =
{
   {0, 128, "FLAT19", "FLAT19", 256, 0, -1}
};

#define NTHINGS 1
ThingData thing[NTHINGS] =
{
   {128, 128, 0, 1, 7}
};

BlockMapHdr bmh = {0, 0, 4, 4, {0}};

int
main(int argc, char **argv)
{
   WADWR *w;
   short *bm;
   int i, bmsize;
   unsigned short j;
   const char *filename = NULL;
   const char *mapname = "E1M1";
   argv0 = argv[0];
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   if (argc == 3
       && argv[1][0] != '-'
       && argv[2][0] != '-') {
      /* Compatibility mode: if there are exactly 2 parameters, and
       * neither of them are options, treat them as FILENAME MAPNAME.  */
      filename = argv[1];
      mapname = argv[2];
      fprintf(stderr, _("%s: Deprecated usage.  Assuming you meant:\n"),
	      argv0);
      fprintf(stderr, "%s -m %s -o %s\n", argv0, mapname, filename);
   } else {
      static const struct option long_options[] = {
	 { "map", required_argument, NULL, 'm' },
	 { "output-wad", required_argument, NULL, 'o' },
	 { "help", no_argument, NULL, 'h' }, /* no -h */
	 { "version", no_argument, NULL, 'V' },	/* no -V */
	 { NULL, 0, NULL, '\0' }
      };
      for (;;) {
	 int c = getopt_long(argc, argv, "m:o:", long_options, NULL);
	 if (c == -1)
	    break;
	 switch (c) {
	 case 'm':		/* -m, --map=MAPNAME */
	    mapname = optarg;
	    break;
	 case 'o':		/* -o, --output-wad=FILENAME */
	    if (filename) {
	       fprintf(stderr, _("%s: Specify only one output filename.\n"), argv0);
	       exit_invalid_args();
	    }
	    filename = optarg;
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
      if (!filename) {
	 fprintf(stderr, _("%s: You must specify an output filename.\n"), argv0);
	 exit_invalid_args();
      }
   } /* incompatibility mode */
   log_stdout();
   w = wadwr_open(filename, 'P');
   wadwr_lump(w, mapname);
   wadwr_lump(w, "THINGS");
   wadwr_write(w, thing, sizeof(ThingData) * NTHINGS);
   wadwr_lump(w, "LINEDEFS");
   wadwr_write(w, line, sizeof(LineData) * NLINES);
   wadwr_lump(w, "SIDEDEFS");
   wadwr_write(w, side, sizeof(SideData) * NSIDES);
   wadwr_lump(w, "VERTEXES");
   wadwr_write(w, ver, sizeof(VertexData) * NVERS);
   wadwr_lump(w, "SECTORS");
   wadwr_write(w, sect, sizeof(SectorData) * NSECTS);
   /* make a bogus blockmap */
   wadwr_lump(w, "BLOCKMAP");
   /* FIXME: assuming sizeof(short)==2 is not portable */
   bm = safe_malloc(bmsize = (2 * (NLINES + 2)));
   bm[0] = 0;
   bm[NLINES + 1] = -1;
   for (i = 0; i < NLINES; i++)
      bm[i + 1] = i;
   wadwr_write(w, &bmh, 8);
   j = 4 + bmh.numx * bmh.numy;
   for (i = 0; i < bmh.numx * bmh.numy; i++)
      wadwr_write(w, &j, 2);
   wadwr_write(w, bm, bmsize);
   /* write it all */
   wadwr_close(w);
   return 0;
}

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s [-m MAPNAME] -o FILENAME\n"
	     "Creates a map consisting of just one 512x512x128 room and the player object.\n"
	     "\n"), argv0);
   fputs(_("  -m, --map=MAPNAME          Name of map to save in the WAD.  Defaults to E1M1.\n"
	   "  -o, --output-wad=FILENAME  Save the map in this file.\n"
	   "      --help                 Display this help and exit.\n"
	   "      --version              Output version information and exit.\n"
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
   fputs("mknulmap (DUMB) " VERSION "\n", stdout);
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

// Local Variables:
// c-basic-offset: 3
// End:
