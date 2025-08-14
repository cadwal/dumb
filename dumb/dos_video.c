/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/dos_video.c: MS-DOS video driver.
 * Copyright (C) 1998 by Ulf Axelsson <ulf@ore.ims.se>
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

#include <stdarg.h>
#include <stdio.h>
#include <allegro.h>
#include <sys/nearptr.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "video.h"

ConfItem video_conf[] =
{
   CONFB("no-page-flip", NULL, 0, N_("dont try to use page flipping")),
   CONFB("use-vesa2-linear", NULL, 0,
	 N_("use VESA 2.0 linear modes (UniVBE preferred)")),
   {NULL}
};

#define cnf_no_page_flip (video_conf[0].intval)
#define cnf_use_vesa2 (video_conf[1].intval)

const char video_dep_name[] = "dosdumb"; /* for --version */

static void *fb = NULL;
static BITMAP *bfb = NULL;
static int col_bpp;
static int curpage = -1;
static unsigned long screen_base_addr;

/* Remember: The powers of 2 are: 2 4 8 16 32 64 ...
 * not: 2 4 6 16 24 32 ....
 * :-)
 *
 * int
 * local_is_linear_bitmap(BITMAP *bmp)
 * {
 *    int i;
 *
 *    for (i = 1 ; i < SCREEN_H ; i++) {
 *	 if (bmp->line[i] != (bmp->line[i-1] + (SCREEN_W * col_bpp)))
 *	    break ;
 *    }
 *    if (i == SCREEN_H)
 *	 return TRUE ;
 *    else
 *	 return FALSE ;
 * }
 */

void
video_preinit(void)
{
   allegro_init();
}

void
init_video(int *width, int *height, int *bpp, int *real_width)
{
   col_bpp = *bpp;
   set_color_depth(col_bpp * 8);

   /* Check for a two-page linear virtual screen, otherwise we'll have to use
    * normal double buffering.
    */
   if (!cnf_no_page_flip ||
       set_gfx_mode((cnf_use_vesa2 ? GFX_VESA2L : GFX_AUTODETECT),
		    *width, *height, *width, (*height) * 2)
       )
      if (set_gfx_mode((cnf_use_vesa2 ? GFX_VESA2L : GFX_AUTODETECT),
		       *width, *height, 0, 0)) {
	 if (cnf_use_vesa2) {
	    if (set_gfx_mode(GFX_AUTODETECT, *width, *height, 0, 0))
	       logprintf(LOG_FATAL, 'V', _("Can't initialize video: %s"),
			 allegro_error);
	 } else
	    logprintf(LOG_FATAL, 'V', _("Can't initialize video: %s"),
		      allegro_error);
      }
   if ((!cnf_no_page_flip) && (VIRTUAL_H >= (2 * SCREEN_H - 1)) &&
       (SCREEN_W == VIRTUAL_W)) {
      /* If we didn't get a linear bitmap there is no use for
       * the virtual screen, it probably causes complications when
       * blitting.
       *
       * Actually the __djgpp_nearptr_enable isn't recommended since it
       * turns off memory protection but speed is everything :-)
       */
      if (is_linear_bitmap(screen)) {
	 if (__djgpp_nearptr_enable()) {
	    __dpmi_get_segment_base_address(screen->seg, &screen_base_addr);

	    bfb = screen;
	    screen_base_addr -= __djgpp_base_address;

	    curpage = 0;
	    logprintf(LOG_INFO, 'V', _("Using page flipping"));
	 }
      } else if (set_gfx_mode(GFX_AUTODETECT, *width, *height, 0, 0))
	 logprintf(LOG_FATAL, 'V', _("Can't initialize video: %s"),
		   allegro_error);
   }
   *real_width = *width = SCREEN_W;
   *height = SCREEN_H;

   if (bfb == NULL) {
      bfb = create_bitmap(SCREEN_W, SCREEN_H);

      if (is_linear_bitmap(bfb)) {
	 fb = bfb->line[0];
	 logprintf(LOG_INFO, 'V', _("Using double buffering"));
      }
   }
   /* If we end up doing this, something is odd */
   if ((curpage < 0) && (fb == NULL)) {
      fb = safe_calloc(SCREEN_W * SCREEN_H, *bpp);
      logprintf(LOG_INFO, 'V', _("Using two-stage double buffering"));
   }
   logprintf(LOG_INFO, 'V', _("Video mode: %s (%s)"),
	     gfx_driver->name, gfx_driver->desc);

}

void
reset_video(void)
{
   if (bfb) {
      if (bfb == screen)
	 __djgpp_nearptr_disable();

      if ((fb != NULL) && (fb != bfb->line[0]))
	 safe_free(fb);
      destroy_bitmap(bfb);
   }
   set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}

#define SHRPAL 2

void
video_setpal(unsigned char idx,
	     unsigned char red,
	     unsigned char green,
	     unsigned char blue)
{
   RGB cpal =
   {red >> SHRPAL, green >> SHRPAL, blue >> SHRPAL};

   vsync();
   set_color(idx, &cpal);
}

void *
video_newframe(void)
{
   if (curpage < 0)
      return fb;
   else
      return (screen_base_addr + bfb->line[SCREEN_H * curpage]);
}

void
video_updateframe(void *v)
{
   int i;

   if (curpage < 0) {
      if (bfb->line[0] != fb)
	 for (i = 0; i < SCREEN_H; i++)
	    memcpy(bfb->line[i], v + (i * SCREEN_W * col_bpp),
		   SCREEN_W * col_bpp);

      vsync();
      blit(bfb, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   } else {
      scroll_screen(0, curpage * SCREEN_H);
      curpage = (curpage == 0 ? 1 : 0);
   }
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
}

// Local Variables:
// c-basic-offset: 3
// End:
