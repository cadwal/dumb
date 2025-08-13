/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/keymapconf.c: Interface between keymap.c and confeng.c.
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

#include <config.h>

#include <assert.h>
#include <stddef.h>		/* NULL */

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/confeng.h"
#include "libdumbutil/log.h"
#include "keymap.h"
#include "keymapconf.h"

ConfItem keymapconf[] =
{
   CONFL("quit", NULL, 0, N_("Keys to quit DUMB")),
   CONFL("move-forward", NULL, 0, N_("Keys to move forward")),
   CONFL("move-backward", NULL, 0, N_("Keys to move backward")),
   CONFL("turn-left", NULL, 0, N_("Keys to turn left")),
   CONFL("turn-right", NULL, 0, N_("Keys to turn right")),
   CONFL("turn-180", NULL, 0, N_("(not implemented)")),
   CONFL("move-left", NULL, 0, N_("Keys to sidestep left")),
   CONFL("move-right", NULL, 0, N_("Keys to sidestep right")),
   CONFL("move-up", NULL, 0, N_("Keys to jump / swim/fly up")),
   CONFL("move-down", NULL, 0, N_("Keys to duck / swim/fly down")),
   CONFL("look-up", NULL, 0, N_("Keys to look up")),
   CONFL("look-down", NULL, 0, N_("Keys to look down")),
   CONFL("aim-up", NULL, 0, N_("Keys to aim up")),
   CONFL("aim-down", NULL, 0, N_("Keys to aim down")),
   CONFL("center-view", NULL, 0, N_("Keys to look straight ahead")),
   CONFL("run", NULL, 0, N_("Keys to hold to move faster")),
   CONFL("strafe", NULL, 0, N_("Keys to hold to sidestep instead of turning")),
   CONFL("activate", NULL, 0, N_("Keys to activate switches and doors")),
   CONFL("shoot", NULL, 0, N_("Keys to shoot with the selected weapon")),
   CONFL("shoot-special", NULL, 0, N_("(not implemented)")),
   CONFL("next-weapon", NULL, 0, N_("Keys to select the next weapon")),
   CONFL("previous-weapon", NULL, 0, N_("Keys to select the previous weapon")),
   CONFL("weapon-0", NULL, 0, N_("Keys to select weapon 0")),
   CONFL("weapon-1", NULL, 0, N_("Keys to select weapon 1")),
   CONFL("weapon-2", NULL, 0, N_("Keys to select weapon 2")),
   CONFL("weapon-3", NULL, 0, N_("Keys to select weapon 3")),
   CONFL("weapon-4", NULL, 0, N_("Keys to select weapon 4")),
   CONFL("weapon-5", NULL, 0, N_("Keys to select weapon 5")),
   CONFL("weapon-6", NULL, 0, N_("Keys to select weapon 6")),
   CONFL("weapon-7", NULL, 0, N_("Keys to select weapon 7")),
   CONFL("weapon-8", NULL, 0, N_("Keys to select weapon 8")),
   CONFL("weapon-9", NULL, 0, N_("Keys to select weapon 9")),
   CONFL("use-item", NULL, 0, N_("Keys to use the selected item")),
   CONFL("next-item", NULL, 0, N_("Keys to select the next item")),
   CONFL("previous-item", NULL, 0, N_("Keys to select the previous item")),
   CONFITEM_END
};

/* Used for passing parameters through keymap_foreach_binding() */
struct keymapconf_list_params {
   ConfItem *ci;
   enum ctlkey ctl;
};

static void check_binding(const char *keyname, enum ctlkey action,
			  void *extra);
static void keymapconf_interpret_list(const ConfItem *, enum ctlkey);
static void keymapconf_make_list(ConfItem *, enum ctlkey);

void
keymapconf_after_load(void)
{
   int i;
   for (i = 0; i < CTLKEY_ARRAY_SIZE; i++)
      keymapconf_interpret_list(&keymapconf[i], i);
   if (keymap_is_empty()) {
      logprintf(LOG_INFO, 'I', _("Keymap is empty, installing defaults."));
      keymap_install_defaults();
   }
}

void
keymapconf_before_save(void)
{
   int i;
   for (i = 0; i < CTLKEY_ARRAY_SIZE; i++)
      keymapconf_make_list(&keymapconf[i], i);
}

void
keymapconf_interpret_list(const ConfItem *ci, enum ctlkey ctl)
{
   char **p;
   if (ci->listval) {
      for (p = ci->listval; *p; p++)
	 keymap_bind_key(*p, ctl);
   }				/* else no change */
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
