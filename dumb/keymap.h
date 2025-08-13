/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/keymap.h: Key mapping from keycodes and keynames to enum ctlkey.
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

#ifndef KEYMAP_H
#define KEYMAP_H

#include "ctlkey.h"

/* The type used for keycodes.  Should have so many bits that keycodes
 * from all input libraries (like KeySyms from X11) can be converted
 * to it and back.  */
typedef long int keymap_keycode;

/* A keycode that can't be received from the keyboard.  Used by
 * keymap.c as a placeholder when the real keycode isn't known yet.
 */
#define KEYMAP_NULL_KEYCODE ((keymap_keycode) 0)

/* ---------------------------------------------------------------------
 * Input driver interface
 */

/* Call these in init_input() and reset_input(), respectively.  They
 * then call the corresponding ctlkey functions.  */
void keymap_init(void);
void keymap_reset(void);

/* A helper function that calls keymap_keycode_to_ctlkey() and
 * ctlkey_press().  Call this each time a key is pressed or released.
 * pressed_flag must be either 0=released or 1=pressed.  */
void keymap_press_keycode(keymap_keycode keycode, int pressed_flag);

/* Conversion from keycodes and keynames to ctlkeys.  It is faster to
 * convert keycodes because that usually avoids string comparisons.
 * But if you have a strange input driver that doesn't use keycodes at
 * all, you can convert keynames too.  */
enum ctlkey keymap_keycode_to_ctlkey(keymap_keycode keycode);
enum ctlkey keymap_keyname_to_ctlkey(const char *keyname);

/* If the interpretation of keycodes should change for some reason so
 * that keymap_keycode_to_keyname will return a different name than it
 * used to, call this.  */
void keymap_clear_caches(void);

/* --- Functions to be defined by each input driver --- */

/* Define this function in your input driver.  It is called when a key
 * is pressed and keymap.c doesn't yet know what its keycode means.
 * The pointer returned must be valid until it's freed with
 * keymap_free_keyname().
 * KEYCODE is a keycode previously given as argument to
 * keymap_keycode_to_ctlkey().
 * This function must not return NULL; if the input driver gave keymap.c
 * a keycode, it should have _some_ name for that.  On the other hand,
 * returning NULL makes keymap.c just print an error message, so don't
 * panic.  */
const char *keymap_keycode_to_keyname(keymap_keycode keycode);

/* Define this function in your input driver.  It is called when ctlkey.c
 * has finished using the pointer returned by keymap_keycode_to_keyname().
 * It is guaranteed that ctlkey.c will call this before calling
 * keymap_keycode_to_keyname() again.
 * The parameter may lose "const" in the future if that makes things simpler.
 */
void keymap_free_keyname(const char *name);

/* I see two possible approaches to the pointer problem:
 * a) keymap_keycode_to_keyname() returns a pointer to static storage.
 *    keymap_free_keyname() is a no-op.
 * b) keymap_keycode_to_keyname() copies the name to a dynamic variable
 *    and returns a pointer to that.
 *    keymap_free_keyname() calls free() to dispose of the variable.
 * Since I don't know which is best, I'm trying to support both.  This may
 * prove to be a bad compromise :)
 */

/* -------------------------------------------------------------------
 * Configuration interface
 */

/* Return 1 if the keymap is all empty and 0 if not.  */
int keymap_is_empty(void);

/* Install default bindings in the keymap.  */
void keymap_install_defaults(void);

/* Bind KEYNAME to ACTION.  Erase any previous binding for that key.
 * If ACTION is CTLKEY_NONE, unbind the key.  The KEYNAME string is
 * copied with strdup(), so it need not exist after the call.  */
void keymap_bind_key(const char *keyname, enum ctlkey action);

/* Call FN for each binding in the keymap.  It must not change the
 * keymap.  The KEYNAME pointer passed to FN is only guaranteed to be
 * valid for the time of the call.  ACTION is of course valid forever.
 * The EXTRA pointer is passed to FN intact.  You could make it point
 * to some structure.
 *
 * Currently, it is safe to longjmp() out of the loop, but that might
 * change in the future.  */
void keymap_foreach_binding(void (*fn) (const char *keyname,
					enum ctlkey action,
					void *extra),
			    void *extra);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
