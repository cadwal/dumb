/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/colour.h: Colors and their black-and-white substitutes.
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

#ifndef XWAD_COLOUR_H
#define XWAD_COLOUR_H

typedef enum {
   Background,
   Border,
   UnpressedCtl,
   PressedCtl,
   HighlightCtl,
   LowlightCtl,
   UnpressedText,
   PressedText,
   UnpressedDanger,
   PressedDanger,
   HighlightDanger,
   LowlightDanger,
   MapBg,
   MapFg,
   MapShowFg,
   ScaleFg,
   LitLight,
   UnlitLight,
   MapCurSelectFg,
   MapSelectFg,
   MapAnnotateFg,
   MapMessageFg,
   MapGridFg,
   ChooseBg,
   ChooseFg,
   ChooseCurBg,
   ChooseCurFg,
   MapTaggedFg,
   NumCtlColours
} CtlColour;

#define XCINIT(c,b) {b,(c>>8)&0xff00,c&0xff00,(c<<8)&0xff00,DoRed|DoGreen|DoBlue,0}

extern int controls_3d;
extern XColor ctl_colours[NumCtlColours];
extern XColor pal_colours[256];
extern Colormap ctl_cmap, pal_cmap;

#define CTLC(c) (ctl_colours[c].pixel)
#define PALC(c) (pal_colours[c].pixel)

void init_colour(void);
void reset_colour(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
