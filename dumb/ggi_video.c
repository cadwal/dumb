/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/ggi_video.c: GGI video & input driver.
 * Copyright (C) 1998 by Andrew Apted <andrew.apted@ggi-project.org>
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

#include <ggi/maintainers.h>
#define MAINTAINER      "Andrew Apted <ajapted@ggi-project.org>"
#define REVISION        "Revision: 1.6"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"

#include "input.h"
#include "video.h"
#include "keymap.h"
#include "keyname.h"

#include <ggi/ggi.h>

#define RUN_SPEED  (UNIT_SPEED << 1)

#define MOUSE_X_SENSITIVITY  40;
#define MOUSE_Y_SENSITIVITY  16;


ConfItem input_conf[] =
{
   /* no input configs yet... */
   /* one idea: --input-sensitivity 24 */
   CONFITEM_END
};

ConfItem video_conf[] =
{
   /* no video configs yet... */
   /* one idea: --video-target display-X */
   CONFITEM_END
};

const char video_dep_name[] = "ggidumb"; /* for --version */

struct ggi_screen_info {

   ggi_visual_t vis;

   ggi_mode mode;

   void *pagev;
   int pagelen;

   int strafing;   /* Ideally these would be queryable from ctlkey */
   int running;

} ggi_screen;


/*****  GGI Graphics  *****/


void
video_preinit(void)
{
   if (ggiInit() != 0) {
      logfatal('V', _("GGI: ggiInit failed."));
   }

   ggi_screen.vis = ggiOpen(NULL);

   if (ggi_screen.vis == NULL) {
      logfatal('V', _("GGI: ggiOpen failed."));
   }

   ggi_screen.pagev = NULL;
}

void
init_video(int *width, int *height, int *bpp, int *real_width)
{
   int err;

   if (ggi_screen.pagev != NULL) {
      safe_free(ggi_screen.pagev);
      ggi_screen.pagev = NULL;
   }

   /* set everything to GGI_AUTO */
   ggiParseMode("", &ggi_screen.mode);

   ggi_screen.mode.visible.x = *width;
   ggi_screen.mode.visible.y = *height;

   switch (*bpp) {
     case 1:
       ggi_screen.mode.graphtype = GT_8BIT;
       break;
   
     case 2:
       ggi_screen.mode.graphtype = GT_16BIT;
       break;
   
     case 4:
       ggi_screen.mode.graphtype = GT_32BIT;
       break;
   
     default:
        logfatal('V', _("Bad BPP (%d) in init_video"), *bpp);
   }

   ggiCheckMode(ggi_screen.vis, &ggi_screen.mode);

   *width      = ggi_screen.mode.visible.x;
   *height     = ggi_screen.mode.visible.y;
   *real_width = ggi_screen.mode.visible.x;

   switch(ggi_screen.mode.graphtype & GT_DEPTH_MASK) {
      case 8:  *bpp = 1; break;
      case 16: *bpp = 2; break;
      case 32: *bpp = 4; break;

      default:
         logfatal('V', _("Depth %d not supported."),
	          ggi_screen.mode.graphtype & GT_DEPTH_MASK);
   }

   if (ggiSetMode(ggi_screen.vis, &ggi_screen.mode) < 0) {
      logfatal('V', _("Mode not supported by LibGGI."));
   }

   ggiSetFlags(ggi_screen.vis, GGIFLAG_ASYNC);

   ggi_screen.pagelen = (*width) * (*height) * (*bpp);
   ggi_screen.pagev = safe_malloc(ggi_screen.pagelen);

   logprintf(LOG_INFO, 'V', _("init_video in copying mode pagelen=%d"),
	     ggi_screen.pagelen);
}

void
reset_video(void)
{
   if (ggi_screen.pagev != NULL) {
      safe_free(ggi_screen.pagev);
      ggi_screen.pagev = NULL;
   }

   if (ggi_screen.vis != NULL) {
      ggiClose(ggi_screen.vis);
   }

   ggiExit();
}

void
video_setpal(unsigned char index, unsigned char red,
	     unsigned char green, unsigned char blue)
{
   ggi_color gcol;

   gcol.r = red   << 8;
   gcol.g = green << 8;
   gcol.b = blue  << 8;

   ggiSetPalette(ggi_screen.vis, index, 1, &gcol);
}

void *
video_newframe(void)
{
   return ggi_screen.pagev;
}

void
video_updateframe(void *v)
{
   ggiPutBox(ggi_screen.vis, 0, 0, ggi_screen.mode.visible.x, 
             ggi_screen.mode.visible.y, v);
   ggiFlush(ggi_screen.vis);
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
   /* nothing to do */
}


/*****  GGI Input  *****/


#define GGI_EVENTS  (emKey | emPointer)


static void
handle_ggi_key_event(ggi_event *ev)
{
   int state;

   if (ev->any.type == evKeyPress) {
      state = 1;
   } else if (ev->any.type == evKeyRelease) {
      state = 0;
   } else {
      return;
   }

   if ((ev->key.label == GGI_KEY_VOID) ||
       (ev->key.label == GGI_KEY_NIL)) return;
   
   /* handling shift & alt */

   switch (ev->key.label) {

#if defined(GGI_KEY_NORMAL_SHIFT)   /* NOTE: old API only */
     case GGI_KEY_NORMAL_SHIFT:
     case GGI_KEY_NORMAL_SHIFTL:
     case GGI_KEY_NORMAL_SHIFTR:
       ggi_screen.running = state;
       break;
     
     case GGI_KEY_NORMAL_ALT:
     case GGI_KEY_NORMAL_ALTGR:
       ggi_screen.strafing = state;
       break;

#elif defined(GGI_KEY_SHIFT)
     case GGI_KEY_SHIFT:
     case GGI_KEY_SHIFTL:
     case GGI_KEY_SHIFTR:
       ggi_screen.running = state;
       break;
     
     case GGI_KEY_ALTL:
     case GGI_KEY_ALTR:
       ggi_screen.strafing = state;
       break;
#endif
   }
    
   keymap_press_keycode(ev->key.label, state);
}

static void
handle_ggi_ptr_event(ggi_event *ev, int *forward, int *rotate, 
                     int *sideways)
{
   int state;
   int speed = (ggi_screen.running) ? RUN_SPEED : UNIT_SPEED;

   if (ev->any.type == evPtrRelative) {

      int dx = -ev->pmove.x;
      int dy = -ev->pmove.y;

      dx = (dx * speed) / MOUSE_X_SENSITIVITY;
      dy = (dy * speed) / MOUSE_Y_SENSITIVITY;

      if (ggi_screen.strafing) {
	 *sideways += dx;
      } else {
	 *rotate += dx;
      }

      *forward += dy;
      return;
   }

   if (ev->any.type == evPtrButtonPress) {
      state = 1;
   } else if (ev->any.type == evPtrButtonRelease) {
      state = 0;
   } else {
      return;
   }

   if (ev->pbutton.button & 1) {	/* primary button */
      ctlkey_press(CTLKEY_SHOOT, state);
   }
   if (ev->pbutton.button & 2) {	/* secondary button */
      ctlkey_press(CTLKEY_MOVE_FORWARD, state);
   }
   if (ev->pbutton.button & 4) {	/* tertiary button */
      ctlkey_press(CTLKEY_STRAFE, state);
      ggi_screen.strafing = state;
   }
}

void
get_input(PlayerInput *in)
{
   ggi_event ev;

   int d_forward  = 0;
   int d_rotate   = 0;
   int d_sideways = 0;

   for (;;) {
      struct timeval tv = {0, 0};

      if (ggiEventPoll(ggi_screen.vis, GGI_EVENTS, &tv) == 0) {
	 break;
      }
      ggiEventRead(ggi_screen.vis, &ev, GGI_EVENTS);

      handle_ggi_key_event(&ev);
      handle_ggi_ptr_event(&ev, &d_forward, &d_rotate, &d_sideways);
   }

   ctlkey_calc_tick();
   ctlkey_get_player_input(in);

   in->forward  += d_forward;
   in->rotate   += d_rotate;
   in->sideways += d_sideways;
}

void
init_input(void)
{
   ggi_screen.running  = 0;
   ggi_screen.strafing = 0;
}

void
reset_input(void)
{
   /* nothing to do */
}

const char *
keymap_keycode_to_keyname(keymap_keycode keycode)
{
   static char namebuf[20];

   /* GGI has key "labels" which are a good (and portable)
    * representation of the particular key pressed.
    */

   if (GGI_KTYP(keycode) == GGI_KT_LATIN1)
      return keyname_of_char((char) GGI_KVAL(keycode));

   switch (keycode) {
      case GGI_KEY_ENTER:  return "Return";
      
      case GGI_KEY_UP:     return "Up";
      case GGI_KEY_DOWN:   return "Down";
      case GGI_KEY_LEFT:   return "Left";
      case GGI_KEY_RIGHT:  return "Right";

      case GGI_KEY_INSERT: return "Insert";
      case GGI_KEY_REMOVE: return "Delete";
      case GGI_KEY_FIND:   return "Home";
      case GGI_KEY_END:    return "End";
      case GGI_KEY_PGUP:   return "PageUp";
      case GGI_KEY_PGDN:   return "PageDown";

      case GGI_KEY_F1:  return "F1";
      case GGI_KEY_F2:  return "F2";
      case GGI_KEY_F3:  return "F3";
      case GGI_KEY_F4:  return "F4";
      case GGI_KEY_F5:  return "F5";
      case GGI_KEY_F6:  return "F6";
      case GGI_KEY_F7:  return "F7";
      case GGI_KEY_F8:  return "F8";
      case GGI_KEY_F9:  return "F9";
      case GGI_KEY_F10: return "F10";
      case GGI_KEY_F11: return "F11";
      case GGI_KEY_F12: return "F12";

      case GGI_KEY_P0:  return "Pad 0";
      case GGI_KEY_P1:  return "Pad 1";
      case GGI_KEY_P2:  return "Pad 2";
      case GGI_KEY_P3:  return "Pad 3";
      case GGI_KEY_P4:  return "Pad 4";
      case GGI_KEY_P5:  return "Pad 5";
      case GGI_KEY_P6:  return "Pad 6";
      case GGI_KEY_P7:  return "Pad 7";
      case GGI_KEY_P8:  return "Pad 8";
      case GGI_KEY_P9:  return "Pad 9";

      case GGI_KEY_PPLUS:  return "Pad +";
      case GGI_KEY_PMINUS: return "Pad -";
      case GGI_KEY_PSTAR:  return "Pad *";
      case GGI_KEY_PSLASH: return "Pad /";
      case GGI_KEY_PENTER: return "Pad Enter";

#if defined(GGI_KEY_NORMAL_SHIFT)   /* NOTE: old API only */
      case GGI_KEY_NORMAL_SHIFT:
      case GGI_KEY_NORMAL_SHIFTL:
      case GGI_KEY_NORMAL_SHIFTR: return "Shift";

      case GGI_KEY_NORMAL_CTRL:
      case GGI_KEY_NORMAL_CTRLL:
      case GGI_KEY_NORMAL_CTRLR: return "Control";

      case GGI_KEY_NORMAL_ALT:
      case GGI_KEY_NORMAL_ALTGR: return "Alt";

#elif defined(GGI_KEY_SHIFT)
      case GGI_KEY_SHIFT:
      case GGI_KEY_SHIFTL:
      case GGI_KEY_SHIFTR: return "Shift";

      case GGI_KEY_CTRL:
      case GGI_KEY_CTRLL:
      case GGI_KEY_CTRLR: return "Control";

      case GGI_KEY_ALTL:
      case GGI_KEY_ALTR: return "Alt";
#endif
   }
   
   /* unknown */

   sprintf(namebuf, "ggi_%04x", keycode);
   return namebuf;
}

void
keymap_free_keyname(const char *keyname)
{
   /* no need to free anything */
}

// Local Variables:
// c-basic-offset: 3
// End:
