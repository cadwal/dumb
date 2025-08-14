/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/ggi_renoir_video.c: GGI video & input driver.
 * Copyright (C) 1999  Kalle Niemitalo	<tosi@stekt.oulu.fi>
 * Copyright (C) 1998  Andrew Apted  <andrew@ggi-project.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

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
   /* idea: --input-sensitivity 100 (percent) */
   /* idea: --input-target input-stdin */
   CONFITEM_END
};

ConfItem video_conf[] =
{
   /* no video configs yet... */
   /* idea: --video-target display-X */
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


/*****	GGI Graphics  *****/


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

   *width      = ggi_screen.mode.visible.x;
   *height     = ggi_screen.mode.visible.y;
   *real_width = ggi_screen.mode.visible.x;

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

   gcol.r = red	  << 8;
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


/*****	GGI Input  *****/


#define GGI_EVENTS  (emKey | emPointer | emValuator)


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

   if ((ev->key.label == GIIK_VOID) ||
       (ev->key.label == GIIK_NIL)) return;

   /* handle shift & alt */

   switch (ev->key.label) {

     case GIIK_ShiftL:
     case GIIK_ShiftR:
       ggi_screen.running = state;
       break;

     case GIIK_AltL: case GIIK_MetaL:
     case GIIK_AltR: case GIIK_MetaR:
       ggi_screen.strafing = state;
       break;
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

   if (ev->pbutton.button == GII_PBUTTON_PRIMARY) {
      ctlkey_press(CTLKEY_SHOOT, state);
   }
   if (ev->pbutton.button == GII_PBUTTON_SECONDARY) {
      ctlkey_press(CTLKEY_STRAFE, state);
      ggi_screen.strafing = state;
   }
   if (ev->pbutton.button == GII_PBUTTON_TERTIARY) {
      ctlkey_press(CTLKEY_NEXT_WEAPON, state);
   }
}

void
get_input(PlayerInput *in)
{
   ggi_event ev;

   int d_sideways = 0;
   int d_forward  = 0;
   int d_rotate	  = 0;

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

   in->forward	+= d_forward;
   in->rotate	+= d_rotate;
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

   switch (GII_KTYP(keycode)) {

      case GII_KT_LATIN1:

	 switch (keycode) {

	    case GIIUC_Tab:		return "Tab";
	    case GIIUC_Linefeed:	return "Linefeed";
	    case GIIUC_Return:		return "Return";
	    case GIIUC_Escape:		return "Escape";
	    case GIIUC_Space:		return "Space";
	    case GIIUC_Delete:		return "Delete";

	    default:
	       return keyname_of_char((char) tolower(GII_KVAL(keycode)));

	 }
	 break;			/* unreached */

      case GII_KT_SPEC:

	 switch (keycode) {

	    case GIIK_Break:		return "Break";
	    case GIIK_ScrollForw:	return "Scroll Forward";
	    case GIIK_ScrollBack:	return "Scroll Back";

	    /* I doubt DUMB ever gets the chance to see these keys, but here
	       they are anyway.	 */
	    case GIIK_Boot:		return "Boot";
	    case GIIK_Compose:		return "Compose";
	    case GIIK_SAK:		return "SAK";

	    case GIIK_Undo:		return "Undo";
	    case GIIK_Redo:		return "Redo";
	    case GIIK_Menu:		return "Menu";
	    case GIIK_Cancel:		return "Cancel";
	    case GIIK_PrintScreen:	return "Print Screen";
	    case GIIK_Execute:		return "Execute";
	    case GIIK_Find:		return "Find";
	    case GIIK_Begin:		return "Begin";
	    case GIIK_Clear:		return "Clear";
	    case GIIK_Insert:		return "Insert";
	    case GIIK_Select:		return "Select";
	    case GIIK_Macro:		return "Macro";
	    case GIIK_Help:		return "Help";
	    case GIIK_Do:		return "Do";
	    case GIIK_Pause:		return "Pause";
	    case GIIK_SysRq:		return "SysRq";
	    case GIIK_ModeSwitch:	return "Mode_switch";

	    case GIIK_Up:		return "Up";
	    case GIIK_Down:		return "Down";
	    case GIIK_Left:		return "Left";
	    case GIIK_Right:		return "Right";
	    case GIIK_Prior:		return "Prior";
	    case GIIK_Next:		return "Next";
	    case GIIK_Home:		return "Home";
	    case GIIK_End:		return "End";

	 }
	 break;			/* sometimes reached */

      case GII_KT_FN:

	 sprintf(namebuf, "F%d", (int) GII_KVAL(keycode));
	 return namebuf;

	 break;			/* obviously unreached */

      case GII_KT_PAD:

	 switch (keycode) {

	    /* The default case handles most pad keys like "Pad 5".
	       List only the exceptions here.  */

#if 0
	    case GIIK_PSeparator:	return "Pad Separator";
	    case GIIK_PDecimal:		return "Pad Decimal";
#endif
	    case GIIK_PSpace:		return "Pad Space";
	    case GIIK_PEnter:		return "Pad Enter";
	    case GIIK_PTab:		return "Pad Tab";

	    case GIIK_PPlusMinus:	return "Pad +-";
	    case GIIK_PBegin:		return "Pad Begin";

	    case GIIK_PF1:		return "Pad F1";
	    case GIIK_PF2:		return "Pad F2";
	    case GIIK_PF3:		return "Pad F3";
	    case GIIK_PF4:		return "Pad F4";
	    case GIIK_PF5:		return "Pad F5";
	    case GIIK_PF6:		return "Pad F6";
	    case GIIK_PF7:		return "Pad F7";
	    case GIIK_PF8:		return "Pad F8";
	    case GIIK_PF9:		return "Pad F9";

	    default:
	       /* This handles all the "Pad x" keys where x is a
		  printable ASCII character.  */
	       if (GII_KVAL(keycode) > 0x20
		   && GII_KVAL(keycode) < 0x7F) {
		  sprintf(namebuf, "Pad %c", (char) GII_KVAL(keycode));
		  return namebuf;
	       }

	 }
	 break;			/* sometimes reached */

      case GII_KT_MOD:

	 switch (keycode) {

	    case GIIK_ShiftL:		return "Shift_L";
	    case GIIK_ShiftR:		return "Shift_R";
	    case GIIK_CtrlL:		return "Control_L";
	    case GIIK_CtrlR:		return "Control_R";
	    case GIIK_AltL:		return "Alt_L";
	    case GIIK_AltR:		return "Alt_R";
	    case GIIK_MetaL:		return "Meta_L";
	    case GIIK_MetaR:		return "Meta_R";
	    case GIIK_SuperL:		return "Super_L";
	    case GIIK_SuperR:		return "Super_R";
	    case GIIK_HyperL:		return "Hyper_L";
	    case GIIK_HyperR:		return "Hyper_R";
	    case GIIK_ShiftLock:	return "ShiftLock";
	    case GIIK_CtrlLock:		return "CtrlLock";
	    case GIIK_AltLock:		return "AltLock";
	    case GIIK_MetaLock:		return "MetaLock";
	    case GIIK_SuperLock:	return "SuperLock";
	    case GIIK_HyperLock:	return "HyperLock";
	    case GIIK_AltGrLock:	return "AltGrLock";
	    case GIIK_CapsLock:		return "CapsLock";
	    case GIIK_NumLock:		return "NumLock";
	    case GIIK_ScrollLock:	return "ScrollLock";

	 }
	 break;			/* sometimes reached */

   } /* switch GII_KTYP */

   /* We couldn't name it above, so use the default format.  */
   sprintf(namebuf, "ggi_%04lx", (unsigned long) keycode);
   return namebuf;
}

void
keymap_free_keyname(const char *keyname)
{
   /* no need to free anything */
}

// Local Variables:
// c-basic-offset: 3
// c-file-offsets: ((case-label . +))
// End:
