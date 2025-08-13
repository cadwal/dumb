/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbworldb/observer.c: Notifying observers when the level is modified.
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

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "dumbworldb.h"

struct dwdb_observer {
   /* There will probably be just a few observers, and they won't be
    * deleted often, so single linking is enough.  */
   struct dwdb_observer *next;
   struct dwdb_observer_funcs funcs;
   void *clientdata;
};

struct dwdb_observer *
dwdb_new_observer(struct dwdb_level *level,
		  const struct dwdb_observer_funcs *funcs,
		  void *clientdata)
{
   struct dwdb_observer *obs
      = (struct dwdb_observer *) safe_malloc(sizeof(struct dwdb_observer));
   /* Note: this will cause the observers to be called in reverse
    * order.  So let's just say the order is unspecified.  */
   obs->next = level->first_observer;
   obs->funcs = *funcs;
   obs->clientdata = clientdata;
   level->first_observer = obs;
   if (obs->funcs.init)
      obs->funcs.init(level, obs->clientdata);
   return obs;
}

void
dwdb_del_observer(struct dwdb_level *level,
		  struct dwdb_observer *obs)
{
   /* This is O(n_observers).  */
   struct dwdb_observer **addr;
   /* addr points to the pointer pointing to the observer to be
    * possibly deleted.  This way *addr can be changed to point
    * somewhere else.  */
   for (addr = &level->first_observer; *addr; addr = &(*addr)->next) {
      if (*addr == obs) {
	 if (obs->funcs.fini)
	    obs->funcs.fini(level, obs->clientdata);
	 *addr = obs->next;
	 safe_free(obs);
	 return;
      }
   }
   logprintf(LOG_ERROR, 'M', "Attempt to delete nonexistent observer");
}

void
_dwdb_after_new(struct dwdb_level *level,
		enum dwdb_object_type objtype,
		int index)
{
   struct dwdb_observer *obs;
   for (obs = level->first_observer; obs; obs = obs->next) {
      if (obs->funcs.after_new)
	 obs->funcs.after_new(level, objtype, index, obs->clientdata);
   }
}

void
_dwdb_after_ch(struct dwdb_level *level,
	       enum dwdb_object_type objtype,
	       int index)
{
   struct dwdb_observer *obs;
   for (obs = level->first_observer; obs; obs = obs->next) {
      if (obs->funcs.after_ch)
	 obs->funcs.after_ch(level, objtype, index, obs->clientdata);
   }
}

void
_dwdb_before_del(struct dwdb_level *level,
		 enum dwdb_object_type objtype,
		 int index)
{
   struct dwdb_observer *obs;
   for (obs = level->first_observer; obs; obs = obs->next) {
      if (obs->funcs.before_del)
	 obs->funcs.before_del(level, objtype, index, obs->clientdata);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
