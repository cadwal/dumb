/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/token.c: Reading tokens from the source file.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/exitcode.h"
#include "libdumbutil/safem.h"
#include "libdumbutil/strgrow.h"

#include "token.h"

static FILE *fin = NULL;
static char *finname = NULL;
static unsigned long line = 0;

static int tok_ungot = 0;
static struct strgrow tokbuf;

static int getc_or_perror(void);
static void parse_hash_line(const char *token);


static int
getc_or_perror(void)
{
   int i = getc(fin);
   if (i == EOF && ferror(fin)) {
      perror(finname);
      exit(DUMB_EXIT_IO_ERROR);
   } else if (i == '\0') {
      /* this will save grief later */
      err(_("NUL characters are not allowed"));
   }
   return i;
}

void
begin_file(FILE *f, const char *name)
{
   assert(!fin);
   assert(!finname);
   fin = f;
   finname = safe_strdup(name);
   line = 0;
   strgrow_init(&tokbuf);
}

void
end_file(void)
{
   assert(fin);
   assert(finname);
   strgrow_fini(&tokbuf);
   safe_free(finname);
   /* Don't close the file; it might be stdin. */
   fin = NULL;
   finname = NULL;
}

enum TokenClass
class_of_token(const char *token)
{
   if (token == NULL)
      return TOKENCL_EOF;
   else if (token[0] == '\n')
      return TOKENCL_NEWLINE;
   else if (token[0] == '"')
      return TOKENCL_STRING;
   else
      return TOKENCL_NAME;
}

const char *
next_token(void)
{
   /* i must be able to hold EOF, so don't make it a char */
   int i;
   if (line == 0)
      line = 1;
   if (tok_ungot) {
      tok_ungot = 0;
      return tokbuf.str;	/* assumed to be already null-terminated */
   }
   strgrow_clear(&tokbuf);
   /* Skip initial whitespace */
   do {
      i = getc_or_perror();
      if (i == EOF)
	 return NULL;
   } while (isspace(i) && i != '\n');
   /* Now parse tokens according to the first character */
   switch (i) {
   case '\n':
      /* Easy */
      line++;
      strgrow_grow(&tokbuf, '\n');
      break;
   case '#':
      /* Collect characters until newline */
      do {
	 strgrow_grow(&tokbuf, i);
	 i = getc_or_perror();
	 if (i == EOF)
	    err(_("Unexpected EOF"));
      } while (i != '\n');
      strgrow_grow(&tokbuf, '\0');
      parse_hash_line(tokbuf.str);
      /* Returning "\n" directly would look nice but cause problems
         later if the caller decides to unget the token.  */
      strgrow_clear(&tokbuf);
      strgrow_grow(&tokbuf, '\n');
      break;
   case '"':
      /* Collect characters until end of string */
      {
	 unsigned long string_begin_line = line;
	 int backslashed = 0;
	 /* We don't really parse backslash escapes here, but we need
            to detect them so things like "\"\\" work. */
	 strgrow_grow(&tokbuf, i);
	 do {
	    backslashed = (!backslashed && i=='\\');
	    i = getc_or_perror();
	    if (i == EOF) {
	       fprintf(stderr, "%s:%lu: %s\n",
		       finname, string_begin_line,
		       _("In string which begins here:"));
	       err(_("Unexpected EOF"));
	    }
	    if (i == '\n')
	       line++;
	    strgrow_grow(&tokbuf, i);
	 } while (i != '"' || backslashed);
      }
      break;
   default:
      /* Collect characters until whitespace or EOF */
      do {
	 strgrow_grow(&tokbuf, i);
	 i = getc_or_perror();
      } while (i != EOF && !isspace(i));
      ungetc(i, fin);		/* ungetc(EOF,f) is a no-op */
      break;
   }
   strgrow_grow(&tokbuf, '\0');
   return tokbuf.str;
}

static void
parse_hash_line(const char *token)
{
   /* '#' [SPACES] LINENUMBER SPACES ['"' FILENAME '"'] [IGNORED-JUNK] */
   unsigned long new_line;
   const char *tail, *fnbegin, *fnend;
   size_t fnlen;
   /* parse line number (strtoul skips spaces) */
   errno = 0;
   new_line = strtoul(token + 1, (char **) &tail, 10);
   if (errno == ERANGE)
      synerr(_("Line number is too large"));
   else if (tail == token+1)
      synerr(_("Line number expected"));
   /* skip spaces between LINENUMBER and '"' */
   while (isspace(*tail))
      tail++;
   /* now there should be the '"' or end of string */
   if (*tail == '\0') {
      line = new_line;
      return;
   }
   if (*tail != '\"')
      synerr(_("`\"' expected before filename"));
   /* ok, it's there.  find the end of FILENAME */
   fnbegin = tail + 1;
   fnend = strchr(fnbegin, '"');
   if (!fnend)
      synerr(_("`\"' expected after filename"));
   /* got it! */
   fnlen = fnend - fnbegin;
   safe_free(finname);
   if (fnlen == 0)
      finname = safe_strdup("stdin");
   else {
      finname = (char *) safe_malloc(fnlen + 1);
      memcpy(finname, fnbegin, fnlen);
      finname[fnlen] = '\0';
   }
   line = new_line;
}

void
unget_token(void)
{
   assert(!tok_ungot);
   tok_ungot = 1;
}

void
print_error_prefix(void)
{
   fprintf(stderr, "%s:%lu: ", finname, line);
}

void
warn(const char *format, ...)
{
   va_list ap;
   va_start(ap, format);
   print_error_prefix();
   fputs(_("Warning: "), stderr);
   vfprintf(stderr, format, ap);
   putc('\n', stderr);
   va_end(ap);
}

void
synerr(const char *format, ...)
{
   va_list ap;
   va_start(ap, format);
   print_error_prefix();
   vfprintf(stderr, format, ap);
   putc('\n', stderr);
   print_error_prefix();
   /* Error messages don't have trailing dots, but these are details
      meant to clarify the message which was already printed.  */
   switch (class_of_token(tokbuf.str)) {
   case TOKENCL_EOF:
      fprintf(stderr, _("End of file.\n"));
      break;
   case TOKENCL_NEWLINE:
      fprintf(stderr, _("Token was a newline.\n"));
      break;
   case TOKENCL_STRING:
      if (strlen(tokbuf.str) > 80) {
	 fprintf(stderr, _("Token was a long string.\n"));
	 break;
      }	/* else fallthru */
   default:
      fprintf(stderr, _("Token was: %s\n"), tokbuf.str);
      break;
   }
   va_end(ap);
   exit(DUMB_EXIT_INVALID_DATA);
}

void
err(const char *format, ...)
{
   va_list ap;
   va_start(ap, format);
   print_error_prefix();
   vfprintf(stderr, format, ap);
   putc('\n', stderr);
   va_end(ap);
   exit(DUMB_EXIT_INVALID_DATA);
}

// Local Variables:
// c-basic-offset: 3
// End:
