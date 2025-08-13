/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/confhelp.c: Online help for configuration items.
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
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "confhelp.h"
#include "confdef.h"
#include "confeng.h"

static void swhelp(ConfItem *ci, const char *mod);
static void modhelp(const ConfModule *conf);
static void pr_etype(const ConfEnum *ce);

int
conf_help(const ConfModule *conf, const char *prog, const char *mod)
{
   conf_usage(conf, prog, NULL);
   if (mod[0] == '-') {
      /* help with a switch */
      ConfItem *ci;
      if (mod[1] == '?')
	 return 1;
      else if (mod[1] == '-')
	 ci = conf_lookup_longname(conf, mod + 2);
      else
	 ci = conf_lookup_shortname(conf, mod[1]);
      if (ci)
	 swhelp(ci, mod);
      else
	 printf(_("`%s' is not a switch I recognise\n"), mod);
   } else {
      /* help with a module */
      if (!strcasecmp(mod, "all"))
	 mod = NULL;
      while (conf->name) {
	 if (mod == NULL || !strcasecmp(conf->name, mod)) {
	    modhelp(conf);
	    putchar('\n');
	 }
	 conf++;
      }
   }
   return 1;
}

int
conf_usage(const ConfModule *conf, const char *prog, const char *err)
{
   if (err)
      printf(_("I don't understand your command line."
	       "  The token which is causing trouble\n"
	       "is `%s'.\n\n"), err);
   printf(_("usage: %s [switch [parameter]] [switch [parameter]]...\n\n"
	    "For a list of all switches available, invoke %s -? all\n\n"
	    "Or, for switches relating to a particular module, %s -? <module>\n"
	    "where module is one of: all"),
	  prog, prog, prog);
   while (conf->name) {
      printf(", %s", conf->name);
      conf++;
   }
   printf(_("\n\nFor information on a particular switch: %s -? <switch>\n\n"),
	  prog);
   return 1;
}

static void
modhelp(const ConfModule *conf)
{
   const ConfItem *it = conf->items;
   printf(_("Module `%s': %s\n"), conf->name, conf->desc);
   if (!it->name)
      printf(_("<no configurable items>\n"));
   else
      while (it->name) {
	 printf("--%s-%s", conf->name, it->name);
	 switch (it->type) {
	 case CONF_TYPE_INT:
	    printf(_(" <integer>"));
	    break;
	 case CONF_TYPE_BOOL:	/* no-op */
	    break;
	 case CONF_TYPE_ENUM:
	    printf(_(" <enum>"));
	    break;
	 case CONF_TYPE_STR:
	    printf(_(" <string>"));
	    break;
	 case CONF_TYPE_LIST:
	    printf(_(" <list>"));
	    break;
	 default:
	    printf(_(" <internal error: strange type %d>"), (int) it->type);
	    break;
	 }
	 if (it->help)
	    printf(": %s", _(it->help));
	 if (it->shortname)
	    printf(_(" (short form -%c)"), it->shortname);
	 putchar('\n');
	 it++;
      }
}

static void
swhelp(ConfItem *ci, const char *mod)
{
   printf(_("Help for %s:\n  `%s'\n"), mod, _(ci->help));
   switch (ci->type) {
   case CONF_TYPE_INT:
      printf(_("  Type=%s\n"), _("Integer"));
      printf(_("  Default=%d\n"), ci->intval);
      break;
   case CONF_TYPE_BOOL:
      printf(_("  Type=%s\n"), _("Boolean"));
      printf(_("  Default=%d\n"), ci->intval);
      break;
   case CONF_TYPE_ENUM:
      printf(_("  Type=%s\n"), _("Enumerated"));
      printf(_("  Default=%s\n"), ci->etype[ci->intval].name);
      printf(_("  Values="));
      pr_etype(ci->etype);
      putchar('\n');
      break;
   case CONF_TYPE_STR:
      printf(_("  Type=%s\n"), _("String"));
      printf(_("  Default=%s\n"), ci->strval);
      break;
   case CONF_TYPE_LIST:
      printf(_("  Type=%s\n"), _("List"));
      break;
   default:
      printf(_("  Type=%d (internal error)\n"), (int) ci->type);
      break;
   }
   if (ci->shortname)
      printf(_("  Short Form=-%c\n"), ci->shortname);
   if (ci->maxlen)
      printf(_("  Maxlen=%d\n"), (int) (ci->maxlen));
   if (ci->flags & CI_NOSAVE)
      printf(_("  Cannot be saved in a config file.\n"));
}

static void
pr_etype(const ConfEnum *ce)
{
   while (ce->name) {
      printf("%s", ce->name);
      ce++;
      if (ce->name)
	 putchar(',');
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
