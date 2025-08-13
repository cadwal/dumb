/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/conffile.c: Configuration file parser.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libdumbutil/dumb-nls.h"

#include "conffile.h"
#include "confeng.h"
#include "log.h"
#include "strgrow.h"
#include "safem.h"

static void parse_conf_strings(ConfItem *ci, const char *s, int dirt);
static const char *dequote_conf_string(struct strgrow *, const char *);
static void fput_conf_string(FILE *fout, const char *s);

/* Find the configuration file and return its name as a string
 * allocated from the free store.  */
char *
conf_file_name(const char *basename)
{
   char *buf;
   FILE *f;
   const char *home;
   if ((f = fopen(basename, "r")) != NULL) {
      fclose(f);
      return safe_strdup(basename);
   }
   home = getenv("HOME");
   /* Someone might get nasty and set "HOME=" (empty string) */
   if (!home || !home[0])
      return safe_strdup(basename);
   /* Room is needed for:
    * - characters of home[]
    * - '/', if home[] doesn't include it
    * - characters of basename[]
    * - '\0'
    */
   buf = (char *) safe_malloc(strlen(home) + 1 + strlen(basename) + 1);
   strcpy(buf, home);
   if (buf[strlen(buf)-1] != '/')
      strcat(buf, "/");
   strcat(buf, basename);
   return buf;
}

#define LOAD_CONF_BUFLEN 256
int
load_conf(const ConfModule *conf, const char *fn)
{
   FILE *fin = fopen(fn, "r");	/* text */
   if (!fin)
      return 1;
   while (!feof(fin)) {
      ConfItem *ci;
      char buf[LOAD_CONF_BUFLEN]; /* FIXME: arbitrary limit */
      char *s, *t;
      if (fgets(buf, LOAD_CONF_BUFLEN, fin) == NULL)
	 break;
      /* eat rest of line, if longer than buflen */
      s = strchr(buf, '\n');
      if (s)
	 *s = '\0';
      else
	 while (!feof(fin) && getc(fin) != '\n');
      /* trim comment */
      s = strchr(buf, ';');
      if (s)
	 *s = '\0';
      /* find separator */
      s = strchr(buf, '=');
      if (!s)
	 continue;
      *s = '\0';
      s++;
      /* find confitem */
      t = buf;
      while (*t == ' ' || *t == '\t')
	 t++;
      ci = conf_lookup_longname(conf, t);
      if (!ci)
	 continue;
      /* set it */
      parse_conf_strings(ci, s, DIRT_FILE);
   }
   return 0;
}

int
save_conf(const ConfModule *confmod, const char *fn, int dirt)
{
   FILE *fout = fopen(fn, "w");  /* text */
   if (!fout)
      return 1;
   /* TRANS Each line must begin with a semicolon. */
   fprintf(fout, _("; %s\n; this file was created automatically, but you can"
		   " modify it if you want\n"),
	   fn);
   for (; confmod->name; confmod++) {
      const ConfItem *ci;
      int nout = 0;
      for (ci = confmod->items; ci->name; ci++) {
	 if (ci->dirtlvl < dirt)
	    continue;
	 if (ci->flags & CI_NOSAVE)
	    continue;
	 if ((nout++) == 0)
	    fprintf(fout, _("\n; Module `%s': %s\n"),
		    confmod->name, confmod->desc);
	 fprintf(fout, "%s-%s=", confmod->name, ci->name);
	 switch (ci->type) {
	 default:
	    logprintf(LOG_ERROR, 'C', 
		      _("internal error: save_conf():"
			" strange type %d, treating as int\n"),
		      (int) ci->type);
	    /* fallthrough */
	 case CONF_TYPE_INT:
	 case CONF_TYPE_BOOL:
	    fprintf(fout, "%d", ci->intval);
	    break;
	 case CONF_TYPE_ENUM:
	    fput_conf_string(fout, ci->etype[ci->intval].name);
	    break;
	 case CONF_TYPE_STR:
	    fput_conf_string(fout, ci->strval ? ci->strval : "");
	    break;
	 case CONF_TYPE_LIST:
	    {
	       char **ps = ci->listval;
	       while (ps && *ps) {
		  fput_conf_string(fout, *ps);
		  ps++;
		  if (*ps)
		     putc(' ', fout);
	       }
	    }
	    break;
	 } /* switch ci->type */
	 putc('\n', fout);
      }	/* for ci */
   } /* for confmod */
   putc('\n', fout);
   fclose(fout);
   return 0;
}

static void
parse_conf_strings(ConfItem *ci, const char *s, int dirt)
{
   struct strgrow sg;
   strgrow_init(&sg);
   for (;;) {
      while (isspace(*s))
	 s++;
      if (!*s)
	 break;			/* end of line */
      s = dequote_conf_string(&sg, s);
      if (!s)
	 break;			/* error */
      set_conf(ci, sg.str, dirt);
   }
   strgrow_fini(&sg);
}

/* Read one possibly quoted string from S to SG.  Return the address
 * of the next character in S, or NULL if an error occurs.  */
static const char *
dequote_conf_string(struct strgrow *sg, const char *s)
{
   int quote = 0;
   strgrow_clear(sg);
   for (;;) {
      switch (*s) {
      case '\0':
	 if (quote) {
	    /* FIXME: print line number too */
	    logprintf(LOG_ERROR, 'C',
		      _("Missing quote in configuration file\n"));
	    return NULL;
	 } else {
	    strgrow_grow(sg, '\0');
	    return s;
	 }
      case '"':
	 quote = !quote;
	 break;
      case '\\':
	 /* For compatibility with configuration files written by
	  * previous versions that didn't support quoting, treat
	  * backslashes outside quotes as ordinary characters.  */
	 if (!quote) {
	    strgrow_grow(sg, '\\');
	    break;
	 }
	 s++;
	 switch (*s) {
	 case '\0':
	    /* FIXME: print line number too */
	    logprintf(LOG_ERROR, 'C',
		      _("Trailing backslash in configuration file\n"));
	    return NULL;
	 case 'n':
	    strgrow_grow(sg, '\n');
	    break;
	 case 't':
	    strgrow_grow(sg, '\t');
	    break;
	 case '0': case '1': case '2': case '3':
	 case '4': case '5': case '6': case '7':
	    { /* Octal sequence */
	       int sum = 0;
	       int count;
	       for (count=0; count<3 && strchr("01234567", *s); count++, s++)
		  sum = sum*8 + (*s - '0');
	       /* TODO: check limits */
	       strgrow_grow(sg, (char) count);
	       s--; /* now it points to the last digit */
	    }
	    break;
	 default:
	    /* Literal */
	    strgrow_grow(sg, *s);
	    break;
	 } /* switch (*s) after backslash */
	 break;
	 /* that was the backslash case... now the rest */
      default:
	 if (!quote && isspace(*s)) {
	    strgrow_grow(sg, '\0');
	    return s;
	 } else
	    strgrow_grow(sg, *s);
	 break;
      }	/* outer switch (*s) */
      s++;
   } /* for ever */
}

static void
fput_conf_string(FILE *fout, const char *s)
{
   putc('"', fout);
   for (; *s; s++) {
      switch (*s) {
      case '\\':
	 fputs("\\\\", fout);
	 break;
      case '\"':
	 fputs("\\\"", fout);
	 break;
      case '\n':
	 fputs("\\\n", fout);
	 break;
      case '\t':
	 fputs("\\\t", fout);
	 break;
      default:
	 /* Locales affect the result of isprint(), but that doesn't matter,
	  * since the reader can parse both formats in any locale.
	  */
	 if (isprint(*s))
	    putc(*s, fout);
	 else /* FIXME: this assumes chars are 8-bit */
	    fprintf(fout, "\\%03o", *s);
	 break;
      }
   }
   putc('"', fout);
}

// Local Variables:
// c-basic-offset: 3
// End:
