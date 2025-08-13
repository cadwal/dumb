/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/parm.c: Reading parameters of simple types.
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

#include "libdumbutil/timer.h"

#include "parm.h"
#include "token.h"

fixed default_speed = 1 << 11;

static int default_time_units = 1;

static int time_units(const char *s, int n);
static int speed_units(const char *s, int n);
static fixed arc_units(const char *s, int n);


void
parm_str(char *buf, size_t n)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("string parameter expected"));
   strncpy(buf, s, n - 1);
   buf[n - 1] = 0;
}

char
parm_ch(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n' || s[1])
      synerr(_("character parameter expected"));
   return *s;
}

int
parm_num(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("integer parameter expected"));
   return atoi(s);
}

double
parm_dbl(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("floating-point parameter expected"));
   return atof(s);
}

int
parm_time(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("time parameter expected"));
   return time_units(s, atoi(s));
}

int
parm_speed(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("speed parameter expected"));
   return speed_units(s, atoi(s));
}

fixed
parm_arc(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      synerr(_("arc parameter expected"));
   return arc_units(s, atoi(s));
}

fixed
parm_arc_opt(fixed def)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      return def;
   return arc_units(s, atoi(s));
}

void
parm_msg(char *buf, size_t n)
{
   char *s;
   parm_str(buf, n);
   for (s = buf; *s; s++)
      if (*s == '_')
	 *s = ' ';
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
   synerr(_("strange timing unit"));
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
   synerr(_("strange speed unit"));
   return n;
}

static fixed
arc_units(const char *s, int n)
{
   while (*s && isdigit(*s))
      s++;
   if (!*s) {
      /* backwards compatibility */
      if (n == 0)
	 return 0;
      return FIXED_PI / n;
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
   synerr(_("strange arc unit"));
   return n;
}

// Local Variables:
// c-basic-offset: 3
// End:
