/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/drawmap.c: Drawing the map and everything in it.
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

#include <string.h>
#include <math.h>

#include <X11/Xlib.h>

#include "xwad.h"
#include "colour.h"
#include "xwadmap.h"

void
message(XWadInstance *inst, const char *text)
{
   int x = inst->map_width / 2, y = 0;
   XCharStruct xcs;
   int dirn, asc, dsc;
   int len = strlen(text);
   XTextExtents(msgfont, text, len, &dirn, &asc, &dsc, &xcs);
   /* figure out where to start drawing */
   x -= xcs.width / 2;
   y += asc + dsc + 2;
   /* now do it */
   XDrawImageString(dpy, inst->map, msggc, x, y, text, len);
}

void
qmessage(XWadInstance *inst, const char *text)
{
   inst->qmsg = text;
   XClearArea(dpy, inst->map, 0, 0, 0, 0, True);
}

typedef void (*VertexDrawFunc) (int x, int y, Window w, int v,
				XWadInstance *inst);

#define CROSSRAD 6
void
vd_cross(int x, int y, Window w, int v, XWadInstance *inst)
{
   XDrawLine(dpy, w, mapgc,
	     x - CROSSRAD, y - CROSSRAD, x + CROSSRAD, y + CROSSRAD);
   XDrawLine(dpy, w, mapgc,
	     x - CROSSRAD, y + CROSSRAD, x + CROSSRAD, y - CROSSRAD);
}

#define VERBOX 8
#define VERBIGBOX 16
void
vd_box(int x, int y, Window w, int v, XWadInstance *inst)
{
   XDrawRectangle(dpy, w, mapgc,
		  x - VERBOX / 2, y - VERBOX / 2, VERBOX, VERBOX);
}

void
draw_vertices(XWadInstance *inst, Window w, VertexDrawFunc func)
{
   int i;
   const VertexData *p = inst->ver;
   for (i = 0; i < inst->nvers; i++, p++) {
      const int x = VER_SCREENX(p), y = VER_SCREENY(p);
      if (x < 0 || x >= inst->map_width || y < 0 || y >= inst->map_height)
	 continue;
      func(x, y, w, i, inst);
      /* special treatment for selected vertices */
      if (inst->mode != VerMode);
      else if (i == inst->curselect) {
	 XSetForeground(dpy, mapgc2, CTLC(MapCurSelectFg));
	 XDrawRectangle(dpy, w, mapgc2,
			x - VERBIGBOX / 2, y - VERBIGBOX / 2,
			VERBIGBOX, VERBIGBOX);
      } else if (inst->enttbl[i] & ENT_SELECTED) {
	 XSetForeground(dpy, mapgc2, CTLC(MapSelectFg));
	 XDrawRectangle(dpy, w, mapgc2,
			x - VERBIGBOX / 2, y - VERBIGBOX / 2,
			VERBIGBOX, VERBIGBOX);
      }
   }
}

#define TH_CIRCX 12
#define TH_CIRCY 12
#define TH_BOXX 16
#define TH_BOXY 16

void
draw_things(XWadInstance *inst, Window w)
{
   int i;
   const ThingData *p = inst->thing;
   for (i = 0; i < inst->nthings; i++, p++) {
      const int x = VER_SCREENX(p), y = VER_SCREENY(p);
      if (x < 0 || x >= inst->map_width || y < 0 || y >= inst->map_height)
	 continue;
      XDrawArc(dpy, w, mapgc, x - TH_CIRCX / 2, y - TH_CIRCY / 2,
	       TH_CIRCX, TH_CIRCY, 0, 360 * 64);
      XFillArc(dpy, w, mapgc, x - TH_CIRCX / 2, y - TH_CIRCY / 2,
	       TH_CIRCX, TH_CIRCY, (p->angle - 15) * 64, 30 * 64);
      /* special treatment for selected things */
      if (inst->mode != ThingMode);
      else if (i == inst->curselect) {
	 XSetForeground(dpy, mapgc2, CTLC(MapCurSelectFg));
	 XDrawRectangle(dpy, w, mapgc2,
			x - TH_BOXX / 2, y - TH_BOXY / 2,
			TH_BOXX, TH_BOXY);
      } else if (inst->enttbl[i] & ENT_SELECTED) {
	 XSetForeground(dpy, mapgc2, CTLC(MapSelectFg));
	 XDrawRectangle(dpy, w, mapgc2,
			x - TH_BOXX / 2, y - TH_BOXY / 2,
			TH_BOXX, TH_BOXY);
      }
   }
}

#define FRONTS(i) (inst->line[i].side[0])
#define BACKS(i) (inst->line[i].side[1])

/* the casts to int clear an ambiguity when compiling as C++ */
#define FRONT(i) (FRONTS(i)<0?(int)-1:(int)(inst->side[FRONTS(i)].sector))
#define BACK(i) (BACKS(i)<0?(int)-1:(int)(inst->side[BACKS(i)].sector))

/* now returns a bitmask */
#define LINESEL_FRONT_CURRENT  0x0001
#define LINESEL_FRONT_SELECTED 0x0002
#define LINESEL_BACK_CURRENT   0x0004
#define LINESEL_BACK_SELECTED  0x0008
static int
line_is_sel(XWadInstance *inst, int i)
{
   int f;
   int bits = 0;
   switch (inst->mode) {
   case (LineMode):
      if (i == inst->curselect)
	 bits |= LINESEL_FRONT_CURRENT | LINESEL_BACK_CURRENT;
      if (inst->enttbl[i] & ENT_SELECTED)
	 bits |= LINESEL_FRONT_SELECTED | LINESEL_BACK_SELECTED;
      break;
   case (SectMode):
      f = FRONT(i);
      if (f >= 0) {
	 if (f == inst->curselect)
	    bits |= LINESEL_FRONT_CURRENT;
	 if (inst->enttbl[f] & ENT_SELECTED)
	    bits |= LINESEL_FRONT_SELECTED;
      }
      f = BACK(i);
      if (f >= 0) {
	 if (f == inst->curselect)
	    bits |= LINESEL_BACK_CURRENT;
	 if (inst->enttbl[f] & ENT_SELECTED)
	    bits |= LINESEL_BACK_SELECTED;
      }
      break;
   default:
      break;
   }
   return bits;
}

static int
line_is_tagged(XWadInstance *inst, int i)
{
   int f, r = 0;
   if (inst->curselect < 0)
      return 0;
   switch (inst->mode) {
   case (SectMode):
      if (inst->line[i].tag)
	 r = (inst->line[i].tag == inst->sect[inst->curselect].tag);
      break;
   case (LineMode):
      if (inst->line[inst->curselect].tag == 0)
	 break;
      f = FRONT(i);
      if (f >= 0)
	 r |= (inst->sect[f].tag == inst->line[inst->curselect].tag);
      f = BACK(i);
      if (f >= 0)
	 r |= (inst->sect[f].tag == inst->line[inst->curselect].tag);
      break;
   default:
      break;
   }
   return r;
}

#define ARROWA (PI/6.0)
#define RIGHTA (PI/2.0)

static void
draw_line(int x1, int y1, int x2, int y2,
	  Window w, int i, XWadInstance *inst, int detail)
{
   XDrawLine(dpy, w, mapgc, x1, y1, x2, y2);

   if (detail) {
      double a = atan2(y2 - y1, x2 - x1);
      int sel = line_is_sel(inst, i);

      /* draw arrowhead */
      if (detail > 1
	  && x2 >= 0
	  && y2 >= 0
	  && x2 < inst->map_width
	  && y2 < inst->map_height) {
	 XDrawLine(dpy, w, mapgc, x2, y2,
		   x2 - (int) (8 * cos(a + ARROWA)),
		   y2 - (int) (8 * sin(a + ARROWA)));
	 XDrawLine(dpy, w, mapgc, x2, y2,
		   x2 - (int) (8 * cos(a - ARROWA)),
		   y2 - (int) (8 * sin(a - ARROWA)));
      }
      /* show selection */
      if (sel) {
	 const int ox1 = (int) (3.99 * cos(a + ARROWA));
	 const int oy1 = (int) (3.99 * sin(a + ARROWA));
	 const int ox2 = (int) (3.99 * cos(a - ARROWA));
	 const int oy2 = (int) (3.99 * sin(a - ARROWA));
	 if (sel & LINESEL_FRONT_CURRENT) {
	    XSetForeground(dpy, mapgc2, CTLC(MapCurSelectFg));
	    XDrawLine(dpy, w, mapgc2, x1 + ox2, y1 + oy2, x2 - ox1, y2 - oy1);
	 } else if (sel & LINESEL_FRONT_SELECTED) {
	    XSetForeground(dpy, mapgc2, CTLC(MapSelectFg));
	    XDrawLine(dpy, w, mapgc2, x1 + ox2, y1 + oy2, x2 - ox1, y2 - oy1);
	 }
	 if (sel & LINESEL_BACK_CURRENT) {
	    XSetForeground(dpy, mapgc2, CTLC(MapCurSelectFg));
	    XDrawLine(dpy, w, mapgc2, x1 + ox1, y1 + oy1, x2 - ox2, y2 - oy2);
	 } else if (sel & LINESEL_BACK_SELECTED) {
	    XSetForeground(dpy, mapgc2, CTLC(MapSelectFg));
	    XDrawLine(dpy, w, mapgc2, x1 + ox1, y1 + oy1, x2 - ox2, y2 - oy2);
	 }
      }
   }
}

void
draw_lines(XWadInstance *inst, Window w, int detail)
{
   int i;
   const LineData *p = inst->line;
   for (i = 0; i < inst->nlines; i++, p++) {
      const VertexData *v1 = inst->ver + p->ver1;
      const VertexData *v2 = inst->ver + p->ver2;
      const int x1 = VER_SCREENX(v1), y1 = VER_SCREENY(v1);
      const int x2 = VER_SCREENX(v2), y2 = VER_SCREENY(v2);
      const int tagged = line_is_tagged(inst, i);
      if (tagged)
	 XSetForeground(dpy, mapgc, CTLC(MapTaggedFg));
      draw_line(x1, y1, x2, y2, w, i, inst, detail);
      if (tagged)
	 XSetForeground(dpy, mapgc, CTLC(MapFg));
   }
}

void
draw_grid(XWadInstance *inst, Window w)
{
   int i, j, k;
   XGCValues gcv;
   k = j = 1;
   if (inst->scale < 0)
      j <<= -inst->scale;
   else
      k <<= inst->scale;
   gcv.line_style = LineOnOffDash;
   XChangeGC(dpy, mapgc2, GCLineStyle, &gcv);
   XSetForeground(dpy, mapgc2, CTLC(MapGridFg));
   for (i = 0; i < inst->map_width; i += j)
      if (abs(MAPX(i) % inst->gridsize) < k)
	 XDrawLine(dpy, w, mapgc2, i, 0, i, inst->map_height);
   for (i = 0; i < inst->map_height; i += j)
      if (abs(MAPY(i) % inst->gridsize) < k)
	 XDrawLine(dpy, w, mapgc2, 0, i, inst->map_width, i);
   gcv.line_style = LineSolid;
   XChangeGC(dpy, mapgc2, GCLineStyle, &gcv);
}

void
draw_map(XWadInstance *inst, Window w, int clear)
{
   /* clear window, if we're not responding to an Expose */
   if (clear)
      XClearWindow(dpy, w);
   XSetForeground(dpy, mapgc, CTLC(MapFg));
   /* draw grid, if in grid mode */
   if (inst->showgrid)
      draw_grid(inst, w);
   /* dragging map around: quickly draw just the basics */
   if (inst->dragmode == DragMap) {
      if (inst->mode == VerMode)
	 draw_vertices(inst, w, vd_cross);
      else {
	 draw_lines(inst, w, 0);
      }
   }
   /* normal redraw */
   else {
      switch (inst->mode) {
      case (VerMode):
	 draw_vertices(inst, w, vd_cross);
	 break;
      case (LineMode):
	 draw_lines(inst, w, 255);
	 break;
      case (SectMode):
	 draw_lines(inst, w, 1);
	 break;
      case (ThingMode):
	 draw_lines(inst, w, 0);
	 draw_things(inst, w);
	 break;
      }
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
