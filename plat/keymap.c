/* keymap.c: key mapping from keycodes and keynames to enum ctlkey.
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

/* This is in plat/ because so far only x11_video.c uses this.  When
 * other input drivers start using these facilities, ctlkey_input.*
 * and keymap.* should probably be moved to lib/ or perhaps to a
 * directory of their own (keymap/).  */

/* Each key in the keyboard is referred to in two ways: keyname (a
 * string) and keycode (long int).  There is a one-to-one match
 * between keynames and keycodes.
 *
 * Keynames are primary.  Keycodes are an optimization.  Keynames are
 * saved to a configuration file and are supposed to be portable
 * between input drivers.  Keycodes exist in runtime only.
 *
 * In the X11 input driver, the keyname for spacebar is the string
 * "space" whereas the corresponding keycode is the constant XK_space
 * (0x020) from <X11/keysymdef.h>.  In the X11 world, they talk about
 * keycodes and keysyms.  DUMB ignores the X11 keycodes as an
 * implementation detail and pretends the X11 keysyms are keycodes.
 * (However, one could optimize by using X11 keycodes as DUMB keycodes
 * and doing the keysym conversions in keymap_keycode_to_keyname().
 * Feel free to do that if you want, but prepare to handle
 * XMappingEvents.)
 *
 * When the player hits the spacebar in the game, the X11 input driver
 * calls keymap_keycode_to_ctlkey(XK_space).  At this phase, the
 * driver hasn't examined the keycode at all.  Then
 * keymap_keycode_to_ctlkey() calls keymap_find_keycode() which scans
 * the keymap to see if 0x020 has been associated with any ctlkey.  It
 * hasn't, so the function returns NULL to keymap_keycode_to_ctlkey()
 * which then calls keymap_keycode_to_keyname(XK_space) to get the
 * corresponding string.  That function is defined in the input
 * driver; in this case, plat/x11_video.c.  It asks Xlib for the
 * string and returns it: "space".  Now keymap_keycode_to_ctlkey()
 * calls keymap_find_keyname(), passing the string to it.  That
 * function scans the keymap again, now looking for "space".  It finds
 * "space" in the keymap and returns a pointer to a keymap structure
 * containing the name "space", the action CTLKEY_ACTIVATE and an
 * empty slot for the keycode.  keymap_keycode_to_ctlkey() puts
 * XK_space=0x020 in that empty slot and returns CTLKEY_ACTIVATE.  The
 * next time the spacebar is pressed, CTLKEY_ACTIVATE will be found
 * without string conversions.
 *
 * I'm pretty satisfied with this implementation.  It could still be
 * sped up with hashes or something but it works fast enough for me as
 * it is.  Also, one could optimize by scanning the keymap once when
 * the program is started and resolving all keynames to keycodes.
 * That would speed up processing of unbound keys.  (Currently, they
 * are not cached at all.)  But then every input driver would have to
 * provide a new function: 
 *   keymap_keycode keymap_keyname_to_keycode(const char *keyname);
 * Currently, only the opposite conversion is required.  And that
 * function we need anyway, to be able to present a keyboard
 * configuration menu where the player can press the key to which he
 * wants to assign a new action and the key's name will be shown to
 * him.
 *
 * Of course, all configuration file handling and such is still
 * missing.  But the lookup engine and dynamic array resizing is
 * ready.  Perhaps lib/conf.c could be used for keyboard configuration
 * too.  */

#include <string.h>
#include "lib/log.h"
#include "lib/safem.h"
#include "plat/keymap.h"

struct keymap_binding {
   char *keyname;		/* the string is in free store */
   keymap_keycode keycode;	/* cache */
   enum ctlkey action;
};

struct keymap_default_binding {
   enum ctlkey action;
   const char *keyname;
};

static struct keymap_binding *keymap_find_keyname(const char *keyname);
static struct keymap_binding *keymap_find_keycode(keymap_keycode keycode);
#if 0
static void keymap_debug_dump(void);
#endif

static void keymap_new_binding(const char *keyname, enum ctlkey action);
static void keymap_del_binding(struct keymap_binding *);
static void keymap_set_capacity(size_t new_capacity);

static struct keymap_default_binding keymap_defaults[] = {
   { CTLKEY_QUIT, "Escape" },
   { CTLKEY_MOVE_FORWARD, "Up" },
   { CTLKEY_MOVE_FORWARD, "KP_Up" },
   { CTLKEY_MOVE_BACKWARD, "Down" },
   { CTLKEY_MOVE_BACKWARD, "KP_Down" },
   { CTLKEY_TURN_LEFT, "Left" },
   { CTLKEY_TURN_LEFT, "KP_Left" },
   { CTLKEY_TURN_RIGHT, "Right" },
   { CTLKEY_TURN_RIGHT, "KP_Right" },
   { CTLKEY_TURN_180, "KP_Insert" },
   { CTLKEY_MOVE_LEFT, "comma" },
   { CTLKEY_MOVE_LEFT, "KP_End" },
   { CTLKEY_MOVE_RIGHT, "period" },
   { CTLKEY_MOVE_RIGHT, "KP_Next" },
   { CTLKEY_MOVE_UP, "a" },
   { CTLKEY_MOVE_DOWN, "z" },
   { CTLKEY_LOOK_UP, "Prior" },
   { CTLKEY_LOOK_DOWN, "Next" },
   { CTLKEY_AIM_UP, "KP_Prior" },
   { CTLKEY_AIM_DOWN, "KP_Home" },
   { CTLKEY_CENTER_VIEW, "KP_Begin" },
   { CTLKEY_RUN, "Shift_L" },
   { CTLKEY_RUN, "Shift_R" },
   { CTLKEY_STRAFE, "Alt_L" },
   { CTLKEY_STRAFE, "Alt_R" },
   { CTLKEY_STRAFE, "Meta_L" },
   { CTLKEY_STRAFE, "Meta_R" },
   { CTLKEY_STRAFE, "Mode_switch" },
   { CTLKEY_STRAFE, "slash" },
   { CTLKEY_ACTIVATE, "space" },
   { CTLKEY_SHOOT, "Control_L" },
   { CTLKEY_SHOOT, "Control_R" },
   { CTLKEY_SHOOT_SPECIAL, "x" },
   { CTLKEY_NEXT_WEAPON, "Tab" },
   { CTLKEY_NEXT_WEAPON, "w" },
   { CTLKEY_PREVIOUS_WEAPON, "q" },
   { CTLKEY_WEAPON_0, "0" },
   { CTLKEY_WEAPON_1, "1" },
   { CTLKEY_WEAPON_2, "2" },
   { CTLKEY_WEAPON_3, "3" },
   { CTLKEY_WEAPON_4, "4" },
   { CTLKEY_WEAPON_5, "5" },
   { CTLKEY_WEAPON_6, "6" },
   { CTLKEY_WEAPON_7, "7" },
   { CTLKEY_WEAPON_8, "8" },
   { CTLKEY_WEAPON_9, "9" },
   { CTLKEY_USE_ITEM, "Return" },
   { CTLKEY_USE_ITEM, "KP_Enter" },
   { CTLKEY_NEXT_ITEM, "KP_Multiply" },
   { CTLKEY_PREVIOUS_ITEM, "KP_Divide" },
   /* CTLKEY_NONE marks the terminator here (but not in the real keymap) */
   { CTLKEY_NONE, NULL }
};

/* These are initialized by keymap_init().  There is no terminator
 * entry in the keymap; you have to check keymap_length. */
static struct keymap_binding *keymap;
static size_t keymap_length, keymap_capacity;

/* ---------------------------------------------------------------------
 * Input driver interface
 */

void
keymap_init(void)
{
   ctlkey_init();
   keymap = NULL;
   keymap_length = keymap_capacity = 0;
}

void
keymap_reset(void)
{
   while (keymap_length > 0)
      keymap_del_binding(&keymap[keymap_length-1]);
   if (keymap)
      safe_free(keymap);
   ctlkey_reset();
}

void
keymap_press_keycode(keymap_keycode keycode, int pressed_flag)
{
   enum ctlkey ctl = keymap_keycode_to_ctlkey(keycode);
   /* if ctl==CTLKEY_NONE, ctlkey_press() won't do anything */
   ctlkey_press(ctl, pressed_flag);
}

enum ctlkey
keymap_keycode_to_ctlkey(keymap_keycode keycode)
{
   struct keymap_binding *binding;
   const char *keyname;
   binding = keymap_find_keycode(keycode);
   if (binding)
      return binding->action;
   keyname = keymap_keycode_to_keyname(keycode);
   if (!keyname) {
      logprintf(LOG_ERROR,'I', "NULL keyname for keycode 0x%08lX",
		(unsigned long) keycode);
      return CTLKEY_NONE;
   }
   binding = keymap_find_keyname(keyname);
   if (binding) {
      binding->keycode = keycode;	/* cache */
      return binding->action;
   }
   logprintf(LOG_DEBUG,'I', "No action for keyname \"%s\"", keyname);
   return CTLKEY_NONE;
}

enum ctlkey
keymap_keyname_to_ctlkey(const char *keyname)
{
   struct keymap_binding *p = keymap_find_keyname(keyname);
   if (p)
      return p->action;
   return CTLKEY_NONE;
}

void
keymap_clear_caches(void)
{
   int i;
   for (i=0; i<keymap_length; i++)
      keymap[i].keycode = KEYMAP_NULL_KEYCODE;
}

/* ---------------------------------------------------------------------
 * Configuration interface
 */

int
keymap_is_empty(void)
{
   return (keymap_length == 0);
}

void
keymap_install_defaults(void)
{
   struct keymap_default_binding *defb;
   for (defb = keymap_defaults; defb->action != CTLKEY_NONE; defb++)
      keymap_bind_key(defb->keyname, defb->action);
}

void
keymap_bind_key(const char *keyname, enum ctlkey action)
{
   struct keymap_binding *binding = keymap_find_keyname(keyname);
   if (action == CTLKEY_NONE) {
      /* Unbind the key if it has been bound.  */
      if (binding) {
	 keymap_del_binding(binding);
      } else {
	 /* Nothing to do.  */
      }
   } else {
      /* Make a binding or change the previous one.  */
      if (binding) {
	 binding->action = action;
      } else {
	 keymap_new_binding(keyname, action);
      }
   }
}

void
keymap_foreach_binding(void (*fn)(const char *keyname,
				  enum ctlkey action,
				  void *extra),
		       void *extra)
{
   int i;
   for (i=0; i<keymap_length; i++)
      (*fn)(keymap[i].keyname, keymap[i].action, extra);
}

/* ---------------------------------------------------------------------
 * Internals
 */

static struct keymap_binding *
keymap_find_keycode(keymap_keycode keycode)
{
   int i;
   for (i=0; i<keymap_length; i++) {
      if (keymap[i].keycode == keycode)
	 return &keymap[i];
   }
   return NULL;
}

static struct keymap_binding *
keymap_find_keyname(const char *keyname)
{
   int i;
   for (i=0; i<keymap_length; i++) {
      if (!strcmp(keymap[i].keyname, keyname))
	 return &keymap[i];
   }
   return NULL;
}

#if 0
static void
keymap_debug_dump(void)
{
   int i;
   logprintf(LOG_DEBUG,'I', "--- begin keymap debug dump");
   for (i=0; i<keymap_length; i++)
      logprintf(LOG_DEBUG,'I', "%-15s %-15s 0x%08lX",
		ctlkey_ugly_name(keymap[i].action),
		keymap[i].keyname,
		(unsigned long) keymap[i].keycode);
   logprintf(LOG_DEBUG,'I', "--- end keymap debug dump");
}
#endif

/* This is a low-level routine used by keymap_bind_key() */
static void
keymap_new_binding(const char *keyname, enum ctlkey action)
{
   struct keymap_binding *binding;
   if (keymap_capacity == keymap_length) {
      /* You could use some other formula to calculate the new
       * capacity.  (keymap_capacity*2) comes to mind, but then you'd
       * need a separate check for keymap_capacity==0.  */
      keymap_set_capacity(keymap_capacity+64);
   }
   /* Now there is space.  */
   binding = &keymap[keymap_length++];
   binding->keyname = safe_strdup(keyname);
   binding->keycode = KEYMAP_NULL_KEYCODE;
   binding->action = action;
}

static void 
keymap_del_binding(struct keymap_binding *binding)
{
   safe_free(binding->keyname);
   /* If the last binding is to be deleted, just decrement the length.
    * Otherwise, move the last binding to the gap first.  */
   if (binding != &keymap[keymap_length-1])
      *binding = keymap[keymap_length-1];
   keymap_length--;
}

static void
keymap_set_capacity(size_t new_capacity)
{
   if (new_capacity < keymap_length) {
      logprintf(LOG_ERROR,'I', "attempt to resize keymap under its length");
      return;
   }
   /* FIXME: Is it permissible to give a null pointer to safe_realloc()? */
   keymap = safe_realloc(keymap, 
			 new_capacity * sizeof (struct keymap_binding));
   keymap_capacity = new_capacity;
}

// Local Variables:
// c-basic-offset: 3
// End:
