/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/sdl_video.c: SDL3 video driver (Wayland, X11, Windows, ...).
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

#include <stdio.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "sdl_video.h"
#include "video.h"

ConfItem video_conf[] =
{
   CONFB("fullscreen", NULL, 0, N_("start in fullscreen mode")),
   CONFI("scale", NULL, 0,
	 N_("initial window size as a multiple of the game size (0: fit the screen)"),
	 0),
   CONFB("no-aspect", NULL, 0,
	 N_("show pixels square instead of stretching 320x200 to 4:3")),
   CONFB("integer-scale", NULL, 0,
	 N_("scale only by whole multiples (sharper, may leave borders)")),
   CONFB("no-vsync", NULL, 0, N_("don't wait for the display's refresh")),
   {NULL}
};

#define cnf_fullscreen    (video_conf[0].intval)
#define cnf_scale         (video_conf[1].intval)
#define cnf_no_aspect     (video_conf[2].intval)
#define cnf_integer_scale (video_conf[3].intval)
#define cnf_no_vsync      (video_conf[4].intval)

const char video_dep_name[] = "sdldumb";	/* for --version */

SDL_Window *sdl_window = NULL;

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static int fb_width, fb_height, fb_bpp;
static unsigned char *frame = NULL;	/* DUMB draws here */
static Uint32 *rgb = NULL;		/* frame through the palette (bpp 1) */
static Uint32 palette[256];

void
video_preinit(void)
{
   if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
      logfatal('V', _("Can't initialize SDL video: %s"), SDL_GetError());
}

/* Pick a window size: CNF_SCALE times the game size, or the largest
   whole multiple that fits in most of the screen.  */
static int
initial_scale(int w, int h)
{
   SDL_Rect usable;
   int scale;
   if (cnf_scale > 0)
      return cnf_scale;
   if (!SDL_GetDisplayUsableBounds(SDL_GetPrimaryDisplay(), &usable))
      return 2;
   for (scale = 1; (scale + 1) * w <= usable.w * 9 / 10
	   && (scale + 1) * h <= usable.h * 9 / 10; scale++)
      ;
   return scale;
}

void
init_video(int *width, int *height, int *bpp, int *real_width)
{
   SDL_PixelFormat format;
   SDL_RendererLogicalPresentation presentation;
   SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
   int shown_height, scale;

   fb_width = *width;
   fb_height = *height;
   if (*bpp != 1 && *bpp != 2 && *bpp != 4)
      *bpp = 1;
   fb_bpp = *bpp;
   *real_width = fb_width;

   /* Old games drew 320x200 (16:10) for 4:3 screens, so the pixels
      were taller than wide.  Show them that way unless told not to.  */
   if (!cnf_no_aspect && fb_width * 5 == fb_height * 8)
      shown_height = fb_width * 3 / 4;
   else
      shown_height = fb_height;

   scale = initial_scale(fb_width, shown_height);
   if (cnf_fullscreen)
      flags |= SDL_WINDOW_FULLSCREEN;
   if (!SDL_CreateWindowAndRenderer("DUMB", fb_width * scale,
				    shown_height * scale, flags,
				    &sdl_window, &renderer))
      logfatal('V', _("Can't open a window: %s"), SDL_GetError());
   SDL_SetRenderVSync(renderer, cnf_no_vsync ? 0 : 1);

   presentation = (cnf_integer_scale
		   ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
		   : SDL_LOGICAL_PRESENTATION_LETTERBOX);
   SDL_SetRenderLogicalPresentation(renderer, fb_width, shown_height,
				    presentation);

   switch (fb_bpp) {
   case 2:
      format = SDL_PIXELFORMAT_RGB565;
      break;
   default:			/* bpp 1 is converted to 32 bits */
      format = SDL_PIXELFORMAT_XRGB8888;
      break;
   }
   texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING,
			       fb_width, fb_height);
   if (texture == NULL)
      logfatal('V', _("Can't create a texture: %s"), SDL_GetError());
   SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

   frame = (unsigned char *) safe_calloc(fb_width * fb_height, fb_bpp);
   if (fb_bpp == 1)
      rgb = (Uint32 *) safe_calloc(fb_width * fb_height, sizeof(Uint32));

   logprintf(LOG_INFO, 'V', _("SDL %d.%d.%d, video driver %s, renderer %s"),
	     SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),
	     SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
	     SDL_VERSIONNUM_MICRO(SDL_GetVersion()),
	     SDL_GetCurrentVideoDriver(), SDL_GetRendererName(renderer));
   logprintf(LOG_INFO, 'V', _("Game %dx%d at %d bpp, window %dx%d"),
	     fb_width, fb_height, fb_bpp * 8,
	     fb_width * scale, shown_height * scale);
}

void
reset_video(void)
{
   if (texture)
      SDL_DestroyTexture(texture);
   if (renderer)
      SDL_DestroyRenderer(renderer);
   if (sdl_window)
      SDL_DestroyWindow(sdl_window);
   texture = NULL;
   renderer = NULL;
   sdl_window = NULL;
   if (rgb)
      safe_free(rgb);
   if (frame)
      safe_free(frame);
   rgb = NULL;
   frame = NULL;
   SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void
video_setpal(unsigned char idx,
	     unsigned char red, unsigned char green, unsigned char blue)
{
   palette[idx] = ((Uint32) red << 16) | ((Uint32) green << 8) | blue;
}

void *
video_newframe(void)
{
   return frame;
}

void
video_updateframe(void *v)
{
   const unsigned char *src = (const unsigned char *) v;
   if (fb_bpp == 1) {
      int i, n = fb_width * fb_height;
      for (i = 0; i < n; i++)
	 rgb[i] = palette[src[i]];
      SDL_UpdateTexture(texture, NULL, rgb, fb_width * sizeof(Uint32));
   } else
      SDL_UpdateTexture(texture, NULL, src, fb_width * fb_bpp);
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
   SDL_RenderClear(renderer);
   SDL_RenderTexture(renderer, texture, NULL, NULL);
   SDL_RenderPresent(renderer);
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
   char title[80];
   snprintf(title, sizeof title, "DUMB: %s", desc);
   if (sdl_window)
      SDL_SetWindowTitle(sdl_window, title);
}

void
sdl_toggle_fullscreen(void)
{
   if (sdl_window) {
      int full = (SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_FULLSCREEN) != 0;
      SDL_SetWindowFullscreen(sdl_window, !full);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
