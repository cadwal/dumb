/* DUMB: A Doom-like 3D game engine.
 *
 * tool/mkpnames.c: Compile a pnames script to PNAMES and TEXTURE1 lumps.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/endiantypes.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/align.h"
#include "libdumbwad/wadstruct.h"

/* source_fname is initially NULL, which means to use stdin.  But
 * after source_file has been opened, source_fname is changed to
 * _("stdin") for use in error messages.  */
const char *source_fname = NULL;
const char *pnames_fname = NULL;
const char *textures_fname = NULL;
FILE *source_file, *pnames_file, *textures_file;

static const char *argv0;	/* argv[0] initialized by parse_args() */

int source_line;

PNameTable *pnames;
TextureData **td;
int ntexs;

int debug_flag = 0;

static void parse_args(int argc, char **argv);
static void exit_invalid_args(void) __attribute__((noreturn));
static void print_help(FILE *stdout_or_stderr);
static void print_version(void);
int get_pnum(const char *s);
void add_texture(const char *s, int x, int y);
void add_patch(const char *s, int x, int y);
int eatws(FILE *f);
int scan(char **name, char *sep, int *x, int *y);
void process(void);
void io_error(const char *filename, int errornum) __attribute__((noreturn));
FILE *check_fopen(const char *filename, const char *opentype);
void check_fwrite(const void *data, size_t size, FILE *stream,
		  const char *filename_for_errors);
void check_fclose(FILE *stream, const char *filename_for_errors);
void check_fseek(FILE *stream, const char *filename_for_errors,
		 long int offset, int whence);
int main(int argc, char **argv);

static void
parse_args(int argc, char **argv)
{
   static const struct option long_options[] = {
      { "debug", no_argument, NULL, 'd' },
      { "output-pnames", required_argument, NULL, 'p' },
      { "output-textures", required_argument, NULL, 't' },
      { "help", no_argument, NULL, 'h' },
      { "version", no_argument, NULL, 'V' },
      { NULL, 0, NULL, '\0' }
   };
   argv0 = argv[0];
   /* Compatibility mode: if there are exactly three parameters, and
    * none of them are options, interpret them as SOURCE PNAMES
    * TEXTURES.  A fourth parameter used to be allowed and meant
    * --debug but I don't believe anyone used it from scripts.  */
   if (argc == 4
       && argv[1][0] != '-'
       && argv[2][0] != '-'
       && argv[3][0] != '-') {
      source_fname = argv[1];
      pnames_fname = argv[2];
      textures_fname = argv[3];
      fprintf(stderr, _("%s: Deprecated usage.  Assuming you meant:\n"),
	      argv0);
      fprintf(stderr, "%s -p %s -t %s %s\n",
	      argv0, pnames_fname, textures_fname, source_fname);
      return;
   }
   for (;;) {
      int c = getopt_long(argc, argv, "dp:t:", long_options, NULL);
      if (c == -1)
	 break;			/* end of options */
      switch (c) {
      case 'd':			/* -d, --debug */
	 /* The long option could be handled with { "debug",
	  * no_argument, &debug_flag, 1 } in long_options[] but the
	  * short option must be handled here anyway.  */
	 debug_flag = 1;
	 break;
      case 'p':			/* -p, --output-pnames */
	 if (pnames_fname) {
	    fprintf(stderr, _("%s: PNAMES filename was already given\n"),
		    argv0);
	    exit_invalid_args();
	 }
	 pnames_fname = optarg;
	 break;
      case 't':			/* -t, --output-textures */
	 if (textures_fname) {
	    fprintf(stderr, _("%s: TEXTURES filename was already given\n"),
		    argv0);
	    exit_invalid_args();
	 }
	 textures_fname = optarg;
	 break;
      case 'h':			/* --help */
	 print_help(stdout);
	 exit(EXIT_SUCCESS);
      case 'V':			/* --version */
	 print_version();
	 exit(EXIT_SUCCESS);
      case '?':
	 /* getopt_long() printed the error message already */
	 exit_invalid_args();
      default:
	 abort();
      }
   } /* for ever */
   if (!pnames_fname) {
      fprintf(stderr, _("%s: PNAMES filename was not given\n"), argv0);
      exit_invalid_args();
   }
   if (!textures_fname) {
      fprintf(stderr, _("%s: TEXTURES filename was not given\n"), argv0);
      exit_invalid_args();
   }
   /* then the source file */
   if (argc > optind)
      source_fname = argv[optind++];
   else
      source_fname = NULL;	/* use stdin */
   if (argc > optind) {
      fprintf(stderr, _("%s: too many parameters\n"), argv0);
      exit_invalid_args();
   }
}

static void
exit_invalid_args(void)
{
   fprintf(stderr, _("Try `%s --help' for more information.\n"), argv0);
   exit(DUMB_EXIT_INVALID_ARGS);
}

static void
print_help(FILE *dest)
{
   fprintf(dest,
	   _("Usage: %s [-d] -p PNAMES -t TEXTURES [SOURCE]\n"
	     "Compiles metatexture information from SOURCE to PNAMES and TEXTURES lumps\n"
	     "used by DUMB.  SOURCE defaults to standard input.\n"
	     "\n"), argv0);
   fputs(_("  -d, --debug                     print debugging info\n"
	   "  -p, --output-pnames=PNAMES      write the pnames lump to the file PNAMES\n"
	   "  -t, --output-textures=TEXTURES  write the textures lump to the file TEXTURES\n"
	   "      --help                      display this help and exit\n"
	   "      --version                   output version information and exit\n"
	   "\n"), dest);
   fputs(_("The input consists of entries such as the following (from dumb-game-0.13.1):\n"),
	 dest);
   fputs("LPLANKS : 42 128\n"	/* do not internationalize this excerpt */
	 "LPLNK @ 0 0\n"
	 "LPLNK.R @ 0 22\n"
	 "\n", dest);
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
   fputs("mkpnames (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

int
get_pnum(const char *s)
{
   int i;
   for (i = 0; i < pnames->num; i++)
      if (!strncasecmp(s, pnames->name + 8 * i, 8))
	 return i;
   /* if we made it to here, pname wasn't found */
   pnames->num += 1;
   strncpy(pnames->name + 8 * i, s, 8);
   return i;
}

#define TDSIZE(t) ((t)->npatches*10+22)

void
add_texture(const char *s, int x, int y)
{
   if (debug_flag)
      printf("add_texture(%s,%d,%d)\n", s, x, y);
   /* shrink last texture data */
   if (ntexs > 0)
      td[ntexs - 1] = realloc(td[ntexs - 1], TDSIZE(td[ntexs - 1])); /* FIXME: errors */
   /* make a new texture */
   td[ntexs] = (TextureData *) calloc(1, sizeof(TextureData));
   strncpy(td[ntexs]->name, s, 8);
   td[ntexs]->dx = x;
   td[ntexs]->dy = y;
   ntexs++;
}

void
add_patch(const char *s, int x, int y)
{
   TextureData *t = td[ntexs - 1];
   if (debug_flag)
      printf("add_patch(%s,%d,%d)\n", s, x, y);
   if (ntexs <= 0) {
      fprintf(stderr, _("%s:%d: patch with no texture (ignored)\n"),
	      source_fname, source_line);
      return;
   }
   t->patch[t->npatches].x = x;
   t->patch[t->npatches].y = y;
   t->patch[t->npatches].pnum = get_pnum(s);
   t->npatches += 1;
}

/* return 1 if there's a problem */
int
eatws(FILE *f)
{
   int c;
   do {
      c = getc(f);
      if (c == EOF)
	 return 1;
   } while (c == ' ');
   ungetc(c, f);
   return 0;
}

/* return 1 if there's more to come, 0 if done */
int
scan(char **name, char *sep, int *x, int *y)
{
   static char buf[16];
   int i = 0, ch;
   *name = NULL;
   /* scan for name */
   if (eatws(source_file))
      return 0;
   while (i < 8) {
      buf[i] = ch = getc(source_file);
      if (ch == EOF)
	 return 0;
      if (ch == ' ')
	 break;
      if (ch == '\n')
	 return 1;
      if (ch == '#') {
	 while (!feof(source_file) && getc(source_file) != '\n');
	 return 1;
      }
      i++;
   }
   buf[i] = '\0';
   /* scan for seperator */
   if (eatws(source_file))
      return 0;
   *sep = getc(source_file);
   /* scan for coords */
   if (eatws(source_file))
      return 0;
   fscanf(source_file, "%d %d", x, y);
   /* scan for newline */
   while (!feof(source_file) && getc(source_file) != '\n');
   *name = buf;
   return 1;
}

void
process(void)
{
   char *name, sep;
   int x, y;
   source_line = 1;
   while (scan(&name, &sep, &x, &y)) {
      if (name == NULL || *name == 0)
	 continue;
      if (sep == ':')
	 add_texture(name, x, y);
      else if (sep == '@')
	 add_patch(name, x, y);
      else
	 fprintf(stderr, _("%s:%d: syntax error (ignoring)\n"),
		 source_fname, source_line);
      source_line++;
   }
   /* scan() returned 0, which means something went wrong.
    * Was it just an EOF, or was there an I/O error?  */
   if (ferror(source_file))
      io_error(source_fname, errno);
}

void
io_error(const char *filename, int errornum)
{
   fprintf(stderr, "%s: %s: %s\n", argv0, filename, strerror(errornum));
   exit(DUMB_EXIT_IO_ERROR);
}

FILE *
check_fopen(const char *filename, const char *opentype)
{
   FILE *ret = fopen(filename, opentype);
   if (ret)
      return ret;
   else {
      /* Do not use io_error() -- this has a separate exit code
       * because file names are generally specified by the user.  */
      fprintf(stderr, "%s: %s: %s", argv0, filename, strerror(errno));
      exit(DUMB_EXIT_FOPEN_FAIL);
   }
}

void
check_fwrite(const void *data, size_t size, FILE *stream,
	     const char *filename)
{
   size_t ret = fwrite(data, 1, size, stream);
   if (ret == size)
      return;
   else
      io_error(filename, errno);
}

void
check_fclose(FILE *stream, const char *filename)
{
   int ret = fclose(stream);
   if (ret == 0)
      return;
   else
      io_error(filename, errno);
}

void
check_fseek(FILE *stream, const char *filename,
	    long int offset, int whence)
{
   int ret = fseek(stream, offset, whence);
   if (ret == 0)
      return;
   else
      io_error(filename, errno);
}

int
main(int argc, char **argv)
{
   int i, o;
   LE_int32 lebuf;
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   parse_args(argc, argv);
   /* allocate some huge arrays */
   td = malloc(8 * 65536);	/* FIXME: error checking and arbitrary limits */
   ntexs = 0;
   pnames = malloc(8 * 65536);
   pnames->num = 0;
   /* go! */
   if (source_fname == NULL) {
      source_file = stdin;
      source_fname = _("stdin");
   } else
      source_file = check_fopen(source_fname, "r");
   process();
   check_fclose(source_file, source_fname);
   /* squirt out lumps */
   pnames_file = check_fopen(pnames_fname, "wb");
   textures_file = check_fopen(textures_fname, "wb");
   check_fwrite(pnames, sizeof(int) + 8 * pnames->num,
		pnames_file, pnames_fname);
   lebuf = ntexs;
   check_fwrite(&lebuf, sizeof(int),
		textures_file, textures_fname);
   o = (ntexs + 1) * sizeof(int);
   for (i = 0; i < ntexs; i++) {
      o = ALIGN(o, ALIGN_TEXTURE1);
      lebuf = o;
      check_fwrite(&lebuf, sizeof(int),
		   textures_file, textures_fname);
      o += TDSIZE(td[i]);
   }
   for (i = 0; i < ntexs; i++) {
      fseek(textures_file, ALIGN(ftell(textures_file), ALIGN_TEXTURE1), SEEK_SET);
      check_fwrite(td[i], TDSIZE(td[i]),
		   textures_file, textures_fname);
   }

   printf(_("%s: %ld pnames, %d textures\n"), argv0,
	  (long) pnames->num, ntexs);

   /* clean up */
   check_fclose(textures_file, textures_fname);
   check_fclose(pnames_file, pnames_fname);
   exit(EXIT_SUCCESS);
}

// Local Variables:
// c-basic-offset: 3
// End:
