/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/sdl_input.c: SDL3 input driver: keyboard through the keymap,
 * plus mouse.
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "ctlkey.h"
#include "input.h"
#include "keymap.h"
#include "keyname.h"
#include "sdl_video.h"

ConfItem input_conf[] =
{
   CONFB("no-mouse", NULL, 0, N_("disable mouse support")),
   CONFI("mouse-sens", NULL, 0, N_("mouse sensitivity (4)"), 4),
   CONFB("mouse-move", NULL, 0,
	 N_("moving the mouse up and down walks instead of looking")),
   {NULL}
};

#define cnf_no_mouse   (input_conf[0].intval)
#define cnf_mouse_sens (input_conf[1].intval)
#define cnf_mouse_move (input_conf[2].intval)

/* Keys that are down, so they can be released if the window loses
   the keyboard focus; otherwise they would stay pressed.  */
#define MAX_DOWN_KEYS 16
static keymap_keycode down_keys[MAX_DOWN_KEYS];
static int n_down_keys;

/* Keys pressed and released between two frames would never be seen,
   because the game reads key states once a frame.  Their release is
   held back to the next frame instead.  */
static keymap_keycode new_keys[MAX_DOWN_KEYS], late_releases[MAX_DOWN_KEYS];
static int n_new_keys, n_late_releases;

static int quit_requested;

static void
press_key(keymap_keycode key, int pressed)
{
   int i;
   for (i = 0; i < n_down_keys && down_keys[i] != key; i++)
      ;
   if (pressed && i == n_down_keys && n_down_keys < MAX_DOWN_KEYS)
      down_keys[n_down_keys++] = key;
   else if (!pressed && i < n_down_keys)
      down_keys[i] = down_keys[--n_down_keys];
   keymap_press_keycode(key, pressed);
}

static void
release_all_keys(void)
{
   while (n_down_keys > 0)
      press_key(down_keys[n_down_keys - 1], 0);
}

void
init_input(void)
{
   if (!cnf_no_mouse && sdl_window
       && !SDL_SetWindowRelativeMouseMode(sdl_window, true)) {
      logprintf(LOG_ERROR, 'I', _("Can't capture the mouse: %s"),
		SDL_GetError());
      cnf_no_mouse = 1;
   }
}

void
reset_input(void)
{
   release_all_keys();
   if (sdl_window)
      SDL_SetWindowRelativeMouseMode(sdl_window, false);
}

void
get_input(PlayerInput *in)
{
   SDL_Event ev;
   float mouse_dx = 0, mouse_dy = 0;
   SDL_MouseButtonFlags buttons;

   /* Releases held back from the last frame (see late_releases).  */
   while (n_late_releases > 0)
      press_key(late_releases[--n_late_releases], 0);
   n_new_keys = 0;

   while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
	 if (ev.key.repeat)
	    break;
	 if (!ev.key.down) {
	    int i;
	    for (i = 0; i < n_new_keys
			&& new_keys[i] != (keymap_keycode) ev.key.key; i++)
	       ;
	    if (i < n_new_keys && n_late_releases < MAX_DOWN_KEYS) {
	       late_releases[n_late_releases++] = ev.key.key;
	       break;
	    }
	 } else if (n_new_keys < MAX_DOWN_KEYS)
	    new_keys[n_new_keys++] = ev.key.key;
	 if (ev.key.down
	     && (ev.key.key == SDLK_F11
		 || (ev.key.key == SDLK_RETURN && (ev.key.mod & SDL_KMOD_ALT)))) {
	    sdl_toggle_fullscreen();
	    break;
	 }
	 press_key((keymap_keycode) ev.key.key, ev.key.down);
	 break;
      case SDL_EVENT_MOUSE_MOTION:
	 mouse_dx += ev.motion.xrel;
	 mouse_dy += ev.motion.yrel;
	 break;
      case SDL_EVENT_WINDOW_FOCUS_LOST:
	 release_all_keys();
	 break;
      case SDL_EVENT_QUIT:
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
	 quit_requested = 1;
	 break;
      default:
	 break;
      }
   }

   ctlkey_calc_tick();
   ctlkey_get_player_input(in);

   if (quit_requested)
      in->quit = 1;
   if (cnf_no_mouse)
      return;

   /* The DOS driver turned by mouse_dx * UNIT_SPEED / 60 mickeys;
      pixels are larger, and the sensitivity defaults to 4.  */
   in->rotate -= (LE_int32) (mouse_dx * cnf_mouse_sens * UNIT_SPEED / 32);
   if (cnf_mouse_move)
      in->forward -= (LE_int32) (mouse_dy * cnf_mouse_sens * UNIT_SPEED / 32);
   else
      in->lookup -= (LE_int32) (mouse_dy * cnf_mouse_sens * UNIT_SPEED / 32);

   buttons = SDL_GetMouseState(NULL, NULL);
   if (buttons & SDL_BUTTON_LMASK)
      in->shoot = 1;
   if (buttons & SDL_BUTTON_RMASK)
      in->action = 1;
   if (buttons & SDL_BUTTON_MMASK)
      in->forward += UNIT_SPEED;
}

/* Names compatible with the X11 driver's, so keymaps can be shared:
   printable keys are their character, the rest X keysym names.  */
const char *
keymap_keycode_to_keyname(keymap_keycode keycode)
{
   switch ((SDL_Keycode) keycode) {
   case SDLK_UP:           return "Up";
   case SDLK_DOWN:         return "Down";
   case SDLK_LEFT:         return "Left";
   case SDLK_RIGHT:        return "Right";
   case SDLK_PAGEUP:       return "Prior";
   case SDLK_PAGEDOWN:     return "Next";
   case SDLK_HOME:         return "Home";
   case SDLK_END:          return "End";
   case SDLK_INSERT:       return "Insert";
   case SDLK_LSHIFT:       return "Shift_L";
   case SDLK_RSHIFT:       return "Shift_R";
   case SDLK_LCTRL:        return "Control_L";
   case SDLK_RCTRL:        return "Control_R";
   case SDLK_LALT:         return "Alt_L";
   case SDLK_RALT:         return "Alt_R";
   case SDLK_LGUI:         return "Super_L";
   case SDLK_RGUI:         return "Super_R";
   case SDLK_MODE:         return "Mode_switch";
   case SDLK_CAPSLOCK:     return "Caps_Lock";
   case SDLK_NUMLOCKCLEAR: return "Num_Lock";
   case SDLK_SCROLLLOCK:   return "Scroll_Lock";
   case SDLK_PRINTSCREEN:  return "Print";
   case SDLK_PAUSE:        return "Pause";
   case SDLK_MENU:         return "Menu";
   case SDLK_F1:           return "F1";
   case SDLK_F2:           return "F2";
   case SDLK_F3:           return "F3";
   case SDLK_F4:           return "F4";
   case SDLK_F5:           return "F5";
   case SDLK_F6:           return "F6";
   case SDLK_F7:           return "F7";
   case SDLK_F8:           return "F8";
   case SDLK_F9:           return "F9";
   case SDLK_F10:          return "F10";
   case SDLK_F12:          return "F12";
   case SDLK_KP_ENTER:     return "Pad Enter";
   case SDLK_KP_EQUALS:    return "Pad =";
   case SDLK_KP_MULTIPLY:  return "Pad *";
   case SDLK_KP_PLUS:      return "Pad +";
   case SDLK_KP_MINUS:     return "Pad -";
   case SDLK_KP_PERIOD:    return "Pad Decimal";
   case SDLK_KP_DIVIDE:    return "Pad /";
   case SDLK_KP_0:         return "Pad 0";
   case SDLK_KP_1:         return "Pad 1";
   case SDLK_KP_2:         return "Pad 2";
   case SDLK_KP_3:         return "Pad 3";
   case SDLK_KP_4:         return "Pad 4";
   case SDLK_KP_5:         return "Pad 5";
   case SDLK_KP_6:         return "Pad 6";
   case SDLK_KP_7:         return "Pad 7";
   case SDLK_KP_8:         return "Pad 8";
   case SDLK_KP_9:         return "Pad 9";
   default:
      /* SDL keycodes of printable keys are their characters.  */
      if (keycode >= 0 && keycode < 128)
	 return keyname_of_char((char) keycode);
      return SDL_GetKeyName((SDL_Keycode) keycode);
   }
}

void
keymap_free_keyname(const char *keyname ATTR_UNUSED)
{
   /* no need to free anything */
}

// Local Variables:
// c-basic-offset: 3
// End:
