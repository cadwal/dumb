/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/xwad.h: The level editor.
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

#ifndef XWAD_H
#define XWAD_H

#define PI M_PI

/* #define USE_LIBDUMBLEVEL */

#include "libdumbutil/fixed.h"
#include "libdumbwad/wadstruct.h"
#include "libdumbwad/wadio.h"
#ifdef USE_LIBDUMBLEVEL
#include "libdumblevel/dumblevel.h"
#endif
#include "controls.h"
#include "choose.h"

extern Display *dpy;
extern Window root;
extern int screen;
/* mapgc is to draw entities, mapgc2 is to annonate / show selection */
extern GC mapgc, mapgc2, msggc;
extern Cursor drag_map_cursor, drag_obj_cursor;
extern XFontStruct *msgfont, *detailfont;

typedef enum {
   VerMode,
   LineMode,
   SectMode,
   ThingMode
} XWadMode;

typedef enum {
   NoDrag,
   DragMap,
   DragObj
} DragMode;

#define NumModes (ThingMode+1)

extern const ControlSet gen_cset[1];
extern const ControlSet map_cset[1];
extern const ControlSet mode_csets[NumModes];

#define MAXENTS 65536

typedef char EntFlags;
#define ENT_SELECTED 1

typedef struct AppInstance {
   /* these may be null for a new level */
   char mapname[10];
   char loadname[10];
#ifdef USE_LIBDUMBLEVEL
   struct dumblevel level;
#else				/* !USE_LIBDUMBLEVEL */
   LumpNum thing_ln, ver_ln, line_ln, side_ln, sect_ln;
   /* level data */
   int nthings, nvers, nlines, nsides, nsects;
   ThingData *thing;
   VertexData *ver;
   LineData *line;
   SideData *side;
   SectorData *sect;
#endif				/* !USE_LIBDUMBLEVEL */
   /* texture chooser stuff */
   ChooseInst tch;
   char *tch_buf;
   int tch_type;
   CSetInstance tchctls;
   Window tch_frame, tch_preview;
   int tch_do_preview;
   /* editor state stuff */
   XWadMode mode;
   Window mapframe, map;
   CSetInstance genctls, modectls, mapctls;
   unsigned int min_width, min_height;
   unsigned int map_width, map_height;
   int xoffset, yoffset;
   int scale;
   int curselect;
   EntFlags *enttbl;
   DragMode dragmode;
   int dragstartx, dragstarty;
   const char *qmsg;
   int gridsize;
   int want_quit:1;
   int bigmap:1;
   int showgrid:1;
} XWadInstance;

void init_instance(XWadInstance *inst);
void load_instance(XWadInstance *inst, const char *mapname);
void free_instance(XWadInstance *inst);

void init_tchoose(XWadInstance *inst);
void free_tchoose(XWadInstance *inst);
void tchoose_wall(XWadInstance *inst, char *buf);
void tchoose_flat(XWadInstance *inst, char *buf);
void tchoose_sprite(XWadInstance *inst, char *buf);
void tchoose_patch(XWadInstance *inst, char *buf);

void save_level(XWadInstance *inst);
void garbage_collect(XWadInstance *inst);

void enter_mode(XWadInstance *inst, XWadMode mode);
void new_selection(int which, XWadInstance *inst, int extended_select);

void update_wmtitle(XWadInstance *inst);
void update_intgeo(XWadInstance *inst);

void draw_map(XWadInstance *inst, Window w, int clear);

void map_button(XButtonEvent *ev, XWadInstance *inst);
void map_motion(XMotionEvent *ev, XWadInstance *inst);

/* message sends message asynchronously, qmessage waits for a redraw */
void message(XWadInstance *inst, const char *msg);
void qmessage(XWadInstance *inst, const char *msg);

int maxsel(const XWadInstance *inst);

void connect_sel_vers(XWadInstance *inst, int ccw);
void join_lines_at_vertex(XWadInstance *inst, int joinver);
void make_sector_from_sel_lines(XWadInstance *inst);
void split_sel_lines(XWadInstance *inst);
void cross_sel_lines(XWadInstance *inst);

int make_corridor_between(XWadInstance *inst, int line1, int line2);
void make_stairs(XWadInstance *inst, int sect, int inc);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
