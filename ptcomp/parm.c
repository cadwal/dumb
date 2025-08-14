/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/parm.c: Reading parameters of simple types.
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

#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumbutil/timer.h"

#include "globals.h"
#include "parm.h"
#include "token.h"

fixed default_speed = 1 << 11;

static int default_time_units = 1;
static int default_speed_units = 1;

static long parse_long(const char *token, long min, long max,
		       const char **tailptr,
		       const char *wrongtypemsg);
static unsigned long parse_ulong(const char *token,
				 unsigned long min, unsigned long max,
				 const char **tailptr,
				 const char *wrongtypemsg);
static size_t parse_string(char *dest, const char *src);
static int time_units(int n, const char *unit);
static int speed_units(int n, const char *unit);
static fixed arc_units(int n, const char *unit);


void
parm_str(char *buf, size_t n)
{
   const char *token = next_token();
   switch (class_of_token(token)) {
   case TOKENCL_STRING:
      {
	 size_t length = parse_string(NULL, token+1);
	 if (length >= n)
	    err(_("String is too long (max %d characters)"), n-1);
	 parse_string(buf, token+1);
	 memset(buf+length, 0, n-length); /* n-length > 0 */
      }
      break;
   case TOKENCL_NAME:      
      if (fake_strings_flag) {
	 size_t length = strlen(token);
	 if (length >= n)
	    err(_("String is too long (max %d characters)"), n-1);
	 strncpy(buf, token, n);	/* will add '\0', since length<n */
      } else
	 synerr(_("String parameter expected.  Try --fake-strings"));
      break;
   default:
      synerr(_("String parameter expected"));
   }
}

char *
parm_strdup(void)
{
   const char *token = next_token();
   switch (class_of_token(token)) {
   case TOKENCL_STRING:
      {
	 size_t length = parse_string(NULL, token+1);
	 char *str = (char *) safe_malloc(length + 1);
	 parse_string(str, token+1);
	 str[length] = '\0';
	 return str;
      }
   case TOKENCL_NAME:
      if (fake_strings_flag)
	 return safe_strdup(token);
      else
	 synerr(_("String parameter expected.  Try --fake-strings"));
      /* NOTREACHED */
   default:
      synerr(_("String parameter expected"));
   }
}

/* Interpret SRC as a string containing C-style backslash escapes and
   store the characters in DEST, without trailing '\0'.  Stop parsing
   on '\0' or unescaped '\"'.  Return the number of characters stored.
   If DEST is NULL, just count the characters.  */
static size_t
parse_string(char *dest, const char *src)
{
   size_t count = 0;
   while (*src != '\0' && *src != '\"') {
      if (*src == '\\') {
	 char c;
	 switch (*++src) {
	 case '\\':
	 case '\"':
	 case '\'':
	    c = *src++;
	    break;
	 /* add other escape sequences here if needed */
	 case '\0':
	 default:
	    err(_("Invalid escape sequence `\\%c'"), *src);
	 }
	 if (dest)
	    *dest++ = c;
	 count++;
      } else {			/* normal characters */
	 if (dest)
	    *dest++ = *src;
	 src++, count++;
      }
   }
   return count;
}

const char *
parm_name(const char *errmsg)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return token;
   else
      synerr(errmsg);
}

int
parm_keyword_opt(const char *keyword)
{
   const char *token = next_token();
   if (!strcasecmp(token, keyword))
      return 1;
   else {
      unget_token();
      return 0;
   }
}

char
parm_ch(void)
{
   const char *token = next_token();
   switch (class_of_token(token)) {
   case TOKENCL_STRING:
      {
	 char ch[1];
	 if (parse_string(NULL, token+1) != 1)
	    synerr(_("Must be exactly one character"));
	 parse_string(ch, token+1);
	 return ch[0];
      }
   case TOKENCL_NAME:
      if (fake_strings_flag) {
	 if (strlen(token) == 1)
	    return token[0];
	 else
	    synerr(_("Must be exactly one character"));
      } else {
	 if (strlen(token) == 1)
	    warn(_("Option `--fake-strings' could help"));
	 synerr(_("String parameter expected"));
      }
      /* NOTREACHED */
   default:
      synerr(_("Character parameter expected"));
   }
}

int
parm_int(void)
{
   return (int) parse_long(next_token(), INT_MIN, INT_MAX, NULL,
			   _("Integer expected"));
}

int
parm_int_opt(int def)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return (int) parse_long(token, INT_MIN, INT_MAX, NULL,
			      _("Integer or newline expected"));
   else {
      unget_token();
      return def;
   }
}

unsigned int
parm_uint(void)
{
   return (unsigned int) parse_ulong(next_token(), 0, UINT_MAX, NULL,
				     _("Integer expected"));
}

unsigned long
parm_ulong(void)
{
   return parse_ulong(next_token(), 0, ULONG_MAX, NULL,
		      _("Integer expected"));
}

/* Parse and return the long int at the beginning of TOKEN.  Fail if
   TOKEN doesn't begin with an integer or the value is out of range
   MIN...MAX, inclusive.  If TAILPTR isn't null, save in *TAILPTR the
   address of the character after the integer; else the caller doesn't
   expect there to be anything after the integer, so fail if there is.  */
static long
parse_long(const char *token, long min, long max,
	   const char **tailptr, const char *wrongtypemsg)
{
   char *tail;
   long value;
   if (class_of_token(token) != TOKENCL_NAME)
      synerr(wrongtypemsg);
   errno = 0;
   value = strtoul(token, &tail, 0);
   /* be careful not to clobber errno here */
   if (tail == token)		/* couldn't parse it at all */
      synerr(wrongtypemsg);
   if (tailptr != NULL)
      *tailptr = tail;
   else if (*tail != '\0')	/* garbage after integer */
      synerr(wrongtypemsg);
   /* now check errno */
   if (errno == ERANGE || value < min || value > max)
      err(_("Integer %.*s is not in range (%ld to %ld)"),
	  tail-token, token, min, max);
   return value;
}

/* Like parse_long() but unsigned.  */
static unsigned long
parse_ulong(const char *token, unsigned long min, unsigned long max,
	    const char **tailptr, const char *wrongtypemsg)
{
   char *tail;
   unsigned long value;
   if (class_of_token(token) != TOKENCL_NAME)
      synerr(wrongtypemsg);
   errno = 0;
   value = strtoul(token, &tail, 0);
   /* be careful not to clobber errno here */
   if (tail == token)		/* couldn't parse it at all */
      synerr(wrongtypemsg);
   if (tailptr != NULL)
      *tailptr = tail;
   else if (*tail != '\0')	/* garbage after integer */
      synerr(wrongtypemsg);
   /* now check errno */
   if (errno == ERANGE || value < min || value > max)
      err(_("Integer %.*s is not in range (%lu to %lu)"),
	  tail-token, token, min, max);
   return value;
}

double
parm_dbl(void)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return atof(token);
   else
      synerr(_("Floating-point parameter expected"));
}

int
parm_time(void)
{
   const char *token = next_token();
   const char *tail;
   int i = parse_long(token, INT_MIN, INT_MAX, &tail,
		      _("Time parameter expected"));
   return time_units(i, tail);
}

int
parm_speed(void)
{
   const char *token = next_token();
   const char *tail;
   int i = parse_long(token, INT_MIN, INT_MAX, &tail,
		      _("Speed parameter expected"));
   return speed_units(i, tail);
}

fixed
parm_arc(void)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME
       && isalpha(token[0])) {
      /* "pi" or "pi/123" can appear without a preceding number.  */
      return arc_units(1, token);
   } else {
      const char *tail;
      int i = parse_long(token, INT_MIN, INT_MAX, &tail,
			 _("Arc parameter expected"));
      return arc_units(i, tail);
   }
}

fixed
parm_arc_opt(fixed def)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME) {
      if (isalpha(token[0])) {
	 /* "pi" or "pi/42" can appear without a preceding number.  */
	 return arc_units(1, token);
      } else {
	 const char *tail;
	 int i = parse_long(token, INT_MIN, INT_MAX, &tail,
			    _("Arc parameter or newline expected"));
	 return arc_units(i, tail);
      }
   } else {
      unget_token();
      return def;
   }
}

void
change_time_units(void)
{
   default_time_units = parm_time();
}

void
change_speed_units(void)
{
   default_speed_units = parm_speed();
}

void
change_default_speed(void)
{
   default_speed = parm_speed();
}


static int
time_units(int n, const char *unit)
{
   if (!*unit)
      return n * default_time_units;
   if (!strcasecmp(unit, "sec"))
      return n * 1000 / MSEC_PER_TICK;
   if (!strcasecmp(unit, "msec"))
      return n / MSEC_PER_TICK;
   if (!strcasecmp(unit, "hsec"))
      return n * 10 / MSEC_PER_TICK;
   if (!strcasecmp(unit, "ticks"))
      return n;
   synerr(_("Strange timing unit"));
   return n;
}

static int
speed_units(int n, const char *unit)
{
   if (!*unit)
      return n * default_speed_units;
   /* we must be talking pixels */
   n <<= 12;
   if (!strcasecmp(unit, "/sec"))
      return n * MSEC_PER_TICK / 1000;
   if (!strcasecmp(unit, "/msec"))
      return n * MSEC_PER_TICK;
   if (!strcasecmp(unit, "/hsec"))
      return n * MSEC_PER_TICK / 10;
   if (!strcasecmp(unit, "/tick"))
      return n;
   synerr(_("Strange speed unit"));
   return n;
}

static fixed
arc_units(int n, const char *unit)
{
   if (!*unit) {
      /* backwards compatibility */
      if (n == 0) {
	 warn(_("Obsolete arc syntax; use `0deg'"));
	 return 0;
      } else {
	 warn(_("Obsolete arc syntax; use `pi/%d'"), n);
	 return FIXED_PI / n;
      }
   }
   if (!strcasecmp(unit, "deg"))
      return (n * FIXED_PI) / 180;
   if (!strcasecmp(unit, "pi"))
      return n * FIXED_PI;
   if (!strncasecmp(unit, "pi/", 3)) {
      /* MIN=1 takes care of "pi/0" */
      int divisor = parse_long(unit+3, 1, INT_MAX, NULL,
			       _("Divisor expected after `pi/'"));
      return (n * FIXED_PI) / divisor;
   }
   synerr(_("Strange arc unit"));
   return n;
}

// Local Variables:
// c-basic-offset: 3
// End:
