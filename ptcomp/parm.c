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

#include <ctype.h>
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

static size_t parse_string(char *dest, const char *src);
static int time_units(const char *s, int n);
static int speed_units(const char *s, int n);
static fixed arc_units(const char *s, int n);


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
      } else
	 synerr(_("String parameter expected.  Try --fake-strings"));
      /* NOTREACHED */
   default:
      synerr(_("Character parameter expected"));
   }
}

int
parm_num(void)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return atoi(token);
   else
      synerr(_("Integer parameter expected"));
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
   if (class_of_token(token) == TOKENCL_NAME)
      return time_units(token, atoi(token));
   else
      synerr(_("Time parameter expected"));
}

int
parm_speed(void)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return speed_units(token, atoi(token));
   else
      synerr(_("Speed parameter expected"));
}

fixed
parm_arc(void)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return arc_units(token, atoi(token));
   else
      synerr(_("Arc parameter expected"));
}

fixed
parm_arc_opt(fixed def)
{
   const char *token = next_token();
   if (class_of_token(token) == TOKENCL_NAME)
      return arc_units(token, atoi(token));
   else {
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
change_default_speed(void)
{
   default_speed = parm_speed();
}


static int
time_units(const char *s, int n)
{
   while (*s && isdigit(*s))
      s++;
   if (!*s)
      return n * default_time_units;
   if (!strcasecmp(s, "sec"))
      return n * 1000 / MSEC_PER_TICK;
   if (!strcasecmp(s, "msec"))
      return n / MSEC_PER_TICK;
   if (!strcasecmp(s, "hsec"))
      return n * 10 / MSEC_PER_TICK;
   if (!strcasecmp(s, "ticks"))
      return n;
   synerr(_("Strange timing unit"));
   return n;
}

static int
speed_units(const char *s, int n)
{
   while (*s && (isdigit(*s) || *s == '+' || *s == '-'))
      s++;
   if (!*s)
      return n;
   /* we must be talking pixels */
   n <<= 12;
   if (!strcasecmp(s, "/sec"))
      return n * MSEC_PER_TICK / 1000;
   if (!strcasecmp(s, "/msec"))
      return n * MSEC_PER_TICK;
   if (!strcasecmp(s, "/hsec"))
      return n * MSEC_PER_TICK / 10;
   if (!strcasecmp(s, "/tick"))
      return n;
   synerr(_("Strange speed unit"));
   return n;
}

static fixed
arc_units(const char *s, int n)
{
   const char *whole = s;
   while (*s && isdigit(*s))
      s++;
   if (!*s) {
      /* backwards compatibility */
      if (n == 0) {
	 warn(_("Obsolete arc syntax `%s'; use `0deg'"), whole);
	 return 0;
      } else {
	 warn(_("Obsolete arc syntax `%s'; use `pi/%d'"), whole, n);
	 return FIXED_PI / n;
      }
   }
   if (!strcasecmp(s, "deg"))
      return (n * FIXED_PI) / 180;
   if (!strcasecmp(s, "pi")) {
      if (n == 0)
	 n = 1;
      return FIXED_PI * n;
   }
   if (!strncasecmp(s, "pi/", 3)) {
      int m;
      if (n == 0)
	 n = 1;
      m = atoi(s + 3);
      if (m)
	 return (FIXED_PI * n) / m;
   }
   synerr(_("Strange arc unit"));
   return n;
}

// Local Variables:
// c-basic-offset: 3
// End:
