/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/sdl_video.h: What the SDL3 input driver needs from the SDL3
 * video driver.
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

#ifndef SDL_VIDEO_H
#define SDL_VIDEO_H

#include <SDL3/SDL.h>

/* The game window; NULL before init_video().  */
extern SDL_Window *sdl_window;

/* Switch between a window and fullscreen.  */
void sdl_toggle_fullscreen(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
