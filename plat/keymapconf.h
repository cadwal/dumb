/* keymapconf.h: interface between keymap.c and conf.c.
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

#ifndef KEYMAPCONF_H
#define KEYMAPCONF_H

/* This is a HACK.  */

#include "plat/keymap.h"
#include "lib/conf.h"

void keymapconf_interpret_all(const ConfItem[CTLKEY_ARRAY_SIZE]);
void keymapconf_make_all(ConfItem[CTLKEY_ARRAY_SIZE]);

void keymapconf_interpret_list(const ConfItem *, enum ctlkey);
void keymapconf_make_list(ConfItem *, enum ctlkey);

#define KEYMAPCONF_ITEM(name,doc) CONFL("keymap-"name,NULL,0,doc)

#define KEYMAPCONF_DEFS							    \
   KEYMAPCONF_ITEM("quit","Keys to quit DUMB"),				    \
   KEYMAPCONF_ITEM("move-forward","Keys to move forward"),		    \
   KEYMAPCONF_ITEM("move-backward","Keys to move backward"),		    \
   KEYMAPCONF_ITEM("turn-left","Keys to turn left"),			    \
   KEYMAPCONF_ITEM("turn-right","Keys to turn right"),			    \
   KEYMAPCONF_ITEM("turn-180","(not implemented)"),			    \
   KEYMAPCONF_ITEM("move-left","Keys to sidestep left"),		    \
   KEYMAPCONF_ITEM("move-right","Keys to sidestep right"),		    \
   KEYMAPCONF_ITEM("move-up","Keys to jump / swim/fly up"),		    \
   KEYMAPCONF_ITEM("move-down","Keys to duck / swim/fly down"),		    \
   KEYMAPCONF_ITEM("look-up","Keys to look up"),			    \
   KEYMAPCONF_ITEM("look-down","Keys to look down"),			    \
   KEYMAPCONF_ITEM("aim-up","Keys to aim up"),				    \
   KEYMAPCONF_ITEM("aim-down","Keys to aim down"),			    \
   KEYMAPCONF_ITEM("center-view","Keys to look straight ahead"),	    \
   KEYMAPCONF_ITEM("run","Keys to hold to move faster"),		    \
   KEYMAPCONF_ITEM("strafe","Keys to hold to sidestep instead of turning"), \
   KEYMAPCONF_ITEM("activate","Keys to activate switches and doors"),	    \
   KEYMAPCONF_ITEM("shoot","Keys to shoot with the selected weapon"),	    \
   KEYMAPCONF_ITEM("shoot-special","(not implemented)"),		    \
   KEYMAPCONF_ITEM("next-weapon","Keys to select the next weapon"),	    \
   KEYMAPCONF_ITEM("previous-weapon","Keys to select the previous weapon"), \
   KEYMAPCONF_ITEM("weapon-0","Keys to select weapon 0"),		    \
   KEYMAPCONF_ITEM("weapon-1","Keys to select weapon 1"),		    \
   KEYMAPCONF_ITEM("weapon-2","Keys to select weapon 2"),		    \
   KEYMAPCONF_ITEM("weapon-3","Keys to select weapon 3"),		    \
   KEYMAPCONF_ITEM("weapon-4","Keys to select weapon 4"),		    \
   KEYMAPCONF_ITEM("weapon-5","Keys to select weapon 5"),		    \
   KEYMAPCONF_ITEM("weapon-6","Keys to select weapon 6"),		    \
   KEYMAPCONF_ITEM("weapon-7","Keys to select weapon 7"),		    \
   KEYMAPCONF_ITEM("weapon-8","Keys to select weapon 8"),		    \
   KEYMAPCONF_ITEM("weapon-9","Keys to select weapon 9"),		    \
   KEYMAPCONF_ITEM("use-item","Keys to use the selected item"),		    \
   KEYMAPCONF_ITEM("next-item","Keys to select the next item"),		    \
   KEYMAPCONF_ITEM("previous-item","Keys to select the previous item")

#endif /* KEYMAPCONF_H */

// Local Variables:
// c-basic-offset: 3
// End:
