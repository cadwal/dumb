/* keymapconf.c: interface between keymap.c and conf.c.
 *
 * Copyright (C) 1998 Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
 * 02139, USA.
 */

#include <assert.h>
#include "plat/keymapconf.h"

/* Used for passing parameters through keymap_foreach_binding() */
struct keymapconf_list_params {
   ConfItem *ci;
   enum ctlkey ctl;
};

static void check_binding(const char *keyname, enum ctlkey action,
			  void *extra);

void
keymapconf_interpret_all(const ConfItem items[CTLKEY_ARRAY_SIZE])
{
   int i;
   for (i = 0; i < CTLKEY_ARRAY_SIZE; i++)
      keymapconf_interpret_list(&items[i], i);
}

void
keymapconf_make_all(ConfItem items[CTLKEY_ARRAY_SIZE])
{
   int i;
   for (i = 0; i < CTLKEY_ARRAY_SIZE; i++)
      keymapconf_make_list(&items[i], i);
}

void
keymapconf_interpret_list(const ConfItem *ci, enum ctlkey ctl)
{
   char **p;
   if (ci->listval) {
      for (p = ci->listval; *p; p++)
	 keymap_bind_key(*p, ctl);
   } /* else no change */
}

void
keymapconf_make_list(ConfItem *ci, enum ctlkey ctl)
{
   struct keymapconf_list_params params;
   params.ci = ci;
   params.ctl = ctl;
   conf_clear_list(ci);
   keymap_foreach_binding(check_binding, &params);
}

static void
check_binding(const char *keyname, enum ctlkey action, void *extra)
{
   struct keymapconf_list_params *params = extra;
   if (action == params->ctl) {
      /* set_conf() won't change keyname anyway because 
       * params->ci->type==CONF_TYPE_LIST  */
      set_conf(params->ci, (char *) keyname, DIRT_MODF);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
