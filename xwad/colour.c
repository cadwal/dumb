/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/colour.c: Colors and their black-and-white substitutes.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
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

#include <stddef.h>

#include <X11/Xlib.h>

#include "xwad.h"
#include "colour.h"

int controls_3d = 1;
XColor ctl_colours[NumCtlColours] =
{
   XCINIT(0x50a050, 1),		/*Background */
   XCINIT(0x000000, 0),		/*Border */
   XCINIT(0xc0c0c0, 1),		/*UnpressedCtl */
   XCINIT(0x909090, 0),		/*PressedCtl */
   XCINIT(0xd0d0d0, 1),		/*HighlightCtl */
   XCINIT(0x808080, 1),		/*LowlightCtl */
   XCINIT(0x000000, 0),		/*UnpressedText */
   XCINIT(0x000000, 1),		/*PressedText */
   XCINIT(0xef9090, 1),		/*UnpressedDanger */
   XCINIT(0xbf6060, 0),		/*PressedDanger */
   XCINIT(0xffa0a0, 1),		/*HighlightDanger */
   XCINIT(0xaf5050, 1),		/*LowlightDanger */
   XCINIT(0x000000, 0),		/*MapBg */
   XCINIT(0xffffff, 1),		/*MapFg */
   XCINIT(0x805020, 1),		/*MapShowFg */
   XCINIT(0xa00000, 1),		/*ScaleFg */
   XCINIT(0xff4000, 1),		/*LitLight */
   XCINIT(0x800000, 1),		/*UnlitLight */
   XCINIT(0xffff30, 1),		/*MapCurSelectFg */
   XCINIT(0x40ff40, 1),		/*MapSelectFg */
   XCINIT(0xa05030, 1),		/*MapAnnotateFg */
   XCINIT(0xdfff30, 1),		/*MapMessageFg */
   XCINIT(0x805020, 1),		/*MapGridFg */
   XCINIT(0xd0d0d0, 1),		/*ChooseBg */
   XCINIT(0x000000, 0),		/*ChooseFg */
   XCINIT(0x0040e0, 0),		/*ChooseCurBg */
   XCINIT(0xffff40, 1),		/*ChooseCurFg */
   XCINIT(0xff30ff, 1)		/*MapTaggedFg */
};
XColor pal_colours[256];
Colormap ctl_cmap, pal_cmap;

void
init_colour(void)
{
   int i;
   ctl_cmap = pal_cmap = DefaultColormap(dpy, screen);
   for (i = 0; i < NumCtlColours; i++)
      XAllocColor(dpy, ctl_cmap, ctl_colours + i);
}

void
reset_colour(void)
{
}

// Local Variables:
// c-basic-offset: 3
// End:
