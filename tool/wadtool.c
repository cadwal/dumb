/* DUMB: A Doom-like 3D game engine.
 *
 * tool/wadtool.c: Copy lumps between wads and other files.
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

/* FIXME: This needs a lot more error checking.  Also, the command
   line parsing should use getopt_long.  See print_help() and
   ./WADTOOL.PLAN for ideas. */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "libdumbwad/wadwr.h"

/* There should be a --verbose, but because there isn't, this is always 1.  */
int verbose_flag = 1;

static void print_help(FILE *dest);
static void print_version(void);
static const char *basename(const char *);
static void catwad(WADWR *wr, FILE *fin, const char *fin_name);

typedef enum {
   None, ReadWad, WriteIWad, WritePWad, WriteDir, PatchWad, CatWad,
   WriteLump, RenameLump,
   WriteRaw, WriteRawCurrent, SpitLump, ExtractLump	/*, ExtractLumpAs */
} Mode;

static void
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

static void
print_version(void)
{
   static const struct copyright copyrights[] = {
      { "1998", "Josh Parsons" },
      { "1998-1999", "Kalle Niemitalo" },
      COPYRIGHT_END
   };
   fputs("wadtool (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

/* Return the part of STR after the last slash,
   or STR if there's no slash.  */
static const char *
basename(const char *str)
{
   const char *slash = strrchr(str, '/');
   if (slash)
      return slash + 1;
   else
      return str;
}

/* This function should be rewritten.  All the header parsing belongs
   in libdumbwad.  */
static void
catwad(WADWR *wr, FILE *fin, const char *fin_name)
{
   WadHeader hdr;
   WadDirEntry *dir;
   unsigned lumpnum;
   char namebuf[12];		/* FIXME: why 12? */
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
	 printf(_("copying %s(%s) -> %s(%s)\n"),
		fin_name, namebuf, wr->fname, namebuf);
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
   /* Regardless of what egcs-2.91.66 says, `i' is not used
      uninitialized.  The first use is in a `for' loop which
      initializes the variable.  */
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
	 /* case('X'): mode=ExtractLumpAs; break; */
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
	    if (!wr)
	       logprintf(LOG_FATAL, 'W', _("Output WAD not chosen"));
	    else {
	       const char *in_fname = argv[i];
	       char lumpname[LUMPNAMELEN+1];
	       char *s;
	       strncpy(lumpname, basename(in_fname), LUMPNAMELEN);
	       lumpname[LUMPNAMELEN] = '\0';
	       s = strrchr(lumpname, '.');
	       if (s)
		  *s = '\0';
	       for (s = lumpname; *s; s++)
		  *s = toupper(*s);
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W', _("copying %s -> %s(%s)"),
			    in_fname, wr->fname, lumpname);
	       wadwr_lump(wr, lumpname);
	    }
	    /* nobreak */
	 case (WriteRawCurrent):
	    if (!wr)
	       logprintf(LOG_FATAL, 'W', _("Output WAD not chosen"));
	    else {
	       char buf[BUFSIZ];
	       FILE *f = fopen(argv[i], "rb");
	       if (!f)
		  logprintf(LOG_FATAL, 'W', "%s: %s", argv[i], strerror(errno));
	       while (!feof(f)) {
		  size_t got = fread(buf, 1, sizeof(buf), f);
		  if (ferror(f))
		     logprintf(LOG_FATAL, 'W', "%s: %s", argv[i], strerror(errno));
		  wadwr_write(wr, buf, got);
	       }
	       if (fclose(f) != 0)
		  logprintf(LOG_FATAL, 'W', "%s: %s", argv[i], strerror(errno));
	    }
	    break;
	 case (SpitLump):
	    if (!wr)
	       logprintf(LOG_FATAL, 'W', _("Output WAD not chosen"));
	    wadwr_lump(wr, argv[i]);
	    break;
	 case (WriteLump):
	    if (!wr)
	       logprintf(LOG_FATAL, 'W', _("Output WAD not chosen"));
	    else {
	       const char *lumpname = argv[i];
	       LumpNum ln = getlump(lumpname);
	       if (!LUMPNUM_OK(ln))
		  logprintf(LOG_FATAL, 'W', _("Lump `%s' not found"),
			    lumpname);
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W', _("copying %s(%s) -> %s(%s)"),
			    get_lump_filename(ln), lumpname,
			    wr->fname, lumpname);
	       wadwr_lump(wr, lumpname);
	       wadwr_write(wr, load_lump(ln), get_lump_len(ln));
	       free_lump(ln);
	    }
	    break;
	 case (ExtractLump):
	    {
	       const char *lumpname = argv[i];
	       char *fout_name;
	       FILE *fout;
	       LumpNum ln = getlump(lumpname);
	       if (!LUMPNUM_OK(ln))
		  logprintf(LOG_FATAL, 'W', _("Lump `%s' not found"),
			    lumpname);
	       fout_name = (char *) safe_malloc(strlen(lumpname) + 5 + 1);
	       sprintf(fout_name, "%s.lump", lumpname);
	       fout = fopen(fout_name, "wb");
	       /* The following conditions can be combined because
                  we're using LOG_FATAL.  Otherwise, this would leak.  */
	       if (fout == NULL
		   || fwrite(load_lump(ln), get_lump_len(ln), 1, fout) != 1
		   || fclose(fout) != 0)
		  logprintf(LOG_FATAL, 'W', "%s: %s",
			    fout_name, strerror(errno));
	       free_lump(ln);
	    }
	    break;
	 case (RenameLump):
	    if (!wr)
	       logprintf(LOG_FATAL, 'W', _("Output WAD not chosen"));
	    else if (i + 1 >= argc)
	       logprintf(LOG_FATAL, 'W', _("Not enough arguments"));
	    else {
	       const char *old_name = argv[i];
	       const char *new_name = argv[++i];
	       LumpNum ln = getlump(old_name);
	       if (!LUMPNUM_OK(ln))
		  logprintf(LOG_FATAL, 'W', _("Lump `%s' not found"),
			    old_name);
	       if (verbose_flag)
		  logprintf(LOG_INFO, 'W', _("copying %s(%s) -> %s(%s)"),
			    get_lump_filename(ln), old_name,
			    wr->fname, new_name);
	       wadwr_lump(wr, new_name);
	       wadwr_write(wr, load_lump(ln), get_lump_len(ln));
	       free_lump(ln);
	    }
	    break;
	 case (CatWad):
	    if (!wr)
	       logprintf(LOG_FATAL, 'W', _("Output WAD not chosen"));
	    else {
	       const char *fin_name = argv[i];
	       FILE *fin = fopen(fin_name, "rb");
	       if (!fin)
		  logprintf(LOG_FATAL, 'W', "%s: %s",
			    fin_name, strerror(errno));
	       catwad(wr, fin, fin_name);
	       if (fclose(fin) != 0)
		  logprintf(LOG_FATAL, 'W', "%s: %s",
			    fin_name, strerror(errno));
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
// c-file-offsets: ((case-label . 0))
// End:
