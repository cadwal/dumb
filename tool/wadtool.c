/* DUMB: A Doom-like 3D game engine.
 *
 * tool/wadtool.c: Copy lumps between wads and other files.
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

/* FIXME: This needs a lot more error checking.  */

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "libdumbwad/wadwr.h"

/* There should be a --verbose, but because there isn't, this is always 1.  */
int verbose_flag = 1;

void print_help(FILE *dest);
void print_version(void);

typedef enum {
   None, ReadWad, WriteIWad, WritePWad, WriteDir, PatchWad, CatWad,
   WriteLump, RenameLump,
   WriteRaw, WriteRawCurrent, SpitLump, ExtractLump	/*, ExtractLumpAs */
} Mode;

void
print_help(FILE *dest)
{
#if 0
   fprintf(dest,
	   _("Usage: %s [OPTION FILE [FILE]...]...\n"
	     "Extract, copy and concatenate WAD files.\n"
	     "\n"), argv0);
   fputs(_(
"  -r, --read-iwad=FILE               read this file as an IWAD\n"
"  -p, --read-pwad=FILE [FILE]...     read these files as PWADs\n"
"  -w, --output-pwad=FILE             start writing output to this PWAD\n"
"  -W, --output-iwad=FILE             start writing output to this IWAD\n"
"  -D, --output-directory=DIRECTORY   start writing output to this directory\n"
"  -c, --copy-wad=FILE [FILE]...      copy all lumps in these wadfiles\n"
"                                     (without patching)\n"
"  -l, --copy-lump=LUMP [LUMP]...     copy these lumps to the current wad\n"
"  -L, --empty-lump=LUMP [LUMP]...    spit out an empty lump\n"
"  -x, --extract-lump=LUMP [LUMP]...  copy these lumps to raw files\n"
"  -f, --read-lump=FILE [FILE]...     copy raw files to lumps\n"
"                                     (guessing lump names)\n"
"  -F, --append-lump=FILE [FILE]...   copy raw files to lump\n"
"                                     (adding to current)\n"
"  -n, --rename-lump OLD NEW          copy lump OLD to lump NEW\n"
"      --help                         display this help and exit\n"
"      --version                      output version information and exit\n"
"\n"), dest);
#endif
   fprintf(dest,
	   _("Usage:  wadtool <option> <file> <file>... <option> <file>...\n\n"
	     "Options:\n"
	     " -r <wadfile> : read this file as an IWAD\n"
	     " -p <wadfile> : read these files as PWADs\n"
	     " -w <wadfile> : start writing output to this PWAD\n"
	     " -W <wadfile> : start writing output to this IWAD\n"
	     " -D <dir>     : start writing output to this directory\n"
	     " -c <wadfile> : copy all lumps in this wadfile (without patching)\n"
	     " -l <lump>    : copy these lumps to the current wad\n"
	     " -L <lump>    : spit out an empty lump\n"
	     " -x <lump>    : copy these lumps to raw files\n"
	     /*" -X <s> <d>   : copy lump <s> to raw file <d>\n" */
	     " -f <rawfile> : copy raw file to lump (guessing lumpname)\n"
	     " -F <rawfile> : copy raw file to lump (adding to current)\n"
	     " -n <s> <d>   : copy lump 's' to lump 'd'\n"
	     "\n"));
   print_bugaddr_message(dest);
}

void
print_version(void)
{
   static const struct copyright copyrights[] = {
      { "1998", "Josh Parsons" },
      COPYRIGHT_END
   };
   fputs("wadtool (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

void
catwad(WADWR *wr, FILE *fin)
{
   WadHeader hdr;
   WadDirEntry *dir;
   unsigned lumpnum;
   char namebuf[12];
   rewind(fin);
   fread(&hdr, sizeof(hdr), 1, fin);
   dir = (WadDirEntry *) malloc(sizeof(WadDirEntry) * hdr.nlumps);
   if (dir == NULL)
      return;
   fseek(fin, hdr.diroffset, SEEK_SET);
   fread(dir, sizeof(WadDirEntry), hdr.nlumps, fin);
   for (lumpnum = 0; lumpnum < hdr.nlumps; lumpnum++) {
      strncpy(namebuf, dir[lumpnum].name, 8);
      namebuf[8] = 0;
      if (verbose_flag)
	 printf(_("copying *(%s) -> %s(%s)\n"),
		namebuf, wr->fname, namebuf);
      wadwr_lump(wr, namebuf);
      if (dir[lumpnum].size > 0) {
	 /* alright so this isn't a very efficient way of doing things! */
	 void *buf = malloc(dir[lumpnum].size);
	 fseek(fin, dir[lumpnum].offset, SEEK_SET);
	 fread(buf, dir[lumpnum].size, 1, fin);
	 wadwr_write(wr, buf, dir[lumpnum].size);
	 free(buf);
      }
   }
   free(dir);
}

int
main(int argc, char **argv)
{
   int i;
   LumpNum ln;
   Mode mode = None;
   WADWR *wr = NULL;
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   log_stdout();
   if (argc >= 2) {
      if (!strcmp(argv[1], "--help")) {
	 print_help(stdout);
	 exit(EXIT_SUCCESS);
      }
      if (!strcmp(argv[1], "--version")) {
	 print_version();
	 exit(EXIT_SUCCESS);
      }
   }
   if (argc < 2) {
      print_help(stderr);
      exit(DUMB_EXIT_INVALID_ARGS);
   }
   for (i = 1; i < argc; i++) {
      if (argv[i][0] == '-')
	 switch (argv[i][1]) {
	 case ('r'):
	    mode = ReadWad;
	    break;
	 case ('p'):
	    mode = PatchWad;
	    break;
	 case ('W'):
	    mode = WriteIWad;
	    break;
	 case ('w'):
	    mode = WritePWad;
	    break;
	 case ('D'):
	    mode = WriteDir;
	    break;
	 case ('c'):
	    mode = CatWad;
	    break;
	 case ('l'):
	    mode = WriteLump;
	    break;
	 case ('x'):
	    mode = ExtractLump;
	    break;
	    /*case('X'): mode=ExtractLumpAs; break; */
	 case ('n'):
	    mode = RenameLump;
	    break;
	 case ('f'):
	    mode = WriteRaw;
	    break;
	 case ('F'):
	    mode = WriteRawCurrent;
	    break;
	 case ('L'):
	    mode = SpitLump;
	    break;
	 default:
	    print_help(stderr);
	    exit(DUMB_EXIT_INVALID_ARGS);
      } else
	 switch (mode) {
	 case (ReadWad):
	    init_iwad(argv[i], NULL);
	    break;
	 case (PatchWad):
	    init_pwad(argv[i], NULL);
	    break;
	 case (WriteIWad):
	    if (wr)
	       wadwr_close(wr);
	    wr = wadwr_open(argv[i], 'I');
	    break;
	 case (WritePWad):
	    if (wr)
	       wadwr_close(wr);
	    wr = wadwr_open(argv[i], 'P');
	    break;
	 case (WriteDir):
	    if (wr)
	       wadwr_close(wr);
	    wr = wadwr_open(argv[i], 'd');
	    break;
	 case (WriteRaw):
	    if (wr) {
	       char buf[16];
	       char *s = strrchr(argv[i], '/');
	       if (s)
		  s++;
	       else
		  s = argv[i];
	       strncpy(buf, s, 8);
	       buf[8] = 0;
	       s = strrchr(buf, '.');
	       if (s)
		  *s = 0;
	       for (s = buf; *s; s++)
		  *s = toupper(*s);
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W', _("copying %s -> %s(%s)"),
			    argv[i], wr->fname, buf);
	       wadwr_lump(wr, buf);
	    }
	    /* nobreak */
	 case (WriteRawCurrent):
	    if (wr) {
	       char buf[1024];
	       FILE *f = fopen(argv[i], "rb");
	       while (f && !feof(f))
		  wadwr_write(wr, buf, fread(buf, 1, 1024, f));
	       if (f)
		  fclose(f);
	    }
	    break;
	 case (SpitLump):
	    if (wr)
	       wadwr_lump(wr, argv[i]);
	    break;
	 case (WriteLump):
	    ln = getlump(argv[i]);
	    if (wr && LUMPNUM_OK(ln)) {
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W', _("copying *(%s) -> %s(%s)"),
			    argv[i], wr->fname, argv[i]);
	       wadwr_lump(wr, argv[i]);
	       wadwr_write(wr, load_lump(ln), get_lump_len(ln));
	       free_lump(ln);
	    }
	    break;
	 case (ExtractLump):
	    ln = getlump(argv[i]);
	    if (LUMPNUM_OK(ln)) {
	       char buf[16];
	       FILE *f;
	       strcpy(buf, argv[i]);
	       strcat(buf, ".lump");
	       f = fopen(buf, "wb");
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W',
			    _("copying *(%s) -> %s"), argv[i], buf);
	       if (f) {
		  fwrite(load_lump(ln), get_lump_len(ln), 1, f);
		  fclose(f);
	       }
	       free_lump(ln);
	    }
	    break;
	 case (RenameLump):
	    ln = getlump(argv[i]);
	    if (wr && LUMPNUM_OK(ln) && i + 1 < argc) {
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W',
			    _("copying *(%s) -> %s(%s)"),
			    argv[i], wr->fname, argv[i + 1]);
	       wadwr_lump(wr, argv[i + 1]);
	       wadwr_write(wr, load_lump(ln), get_lump_len(ln));
	       free_lump(ln);
	       i++;
	    }
	    break;
	 case (CatWad):
	    if (wr) {
	       FILE *fin = fopen(argv[i], "rb");
	       if (fin) {
		  catwad(wr, fin);
		  fclose(fin);
	       }
	    }
	    break;
	 case (None):
	    print_help(stderr);
	    exit(DUMB_EXIT_INVALID_ARGS);
	 }
   }
   if (wr)
      wadwr_close(wr);
   reset_wad();
   exit(EXIT_SUCCESS);
}

// Local Variables:
// c-basic-offset: 3
// End:
