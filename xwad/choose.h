/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/choose.h: Scrolling lists.
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

#ifndef XWAD_CHOOSE_H
#define XWAD_CHOOSE_H

struct AppInstance;

#define AppInst struct AppInstance

#define CHOOSEBUFSIZE 256

typedef struct {
   int nitems;
   int curitem;
   int scroll;
   int sbdragstart;
   unsigned int width, height;
   Window w, wscroll;
   void **tbl;
   const char *(*text) (int item, AppInst * inst);
   int (*find) (const char *prefix, AppInst * inst);
   void (*choose) (int item, AppInst * inst);
   void (*chcur) (int item, AppInst * inst);
} ChooseInst;

void choose_cur(ChooseInst *ci, int i, AppInst * inst);
void choose_keycmd(ChooseInst *ci, int code, AppInst * inst);
void choose_setcur(ChooseInst *ci, int i, AppInst * inst);

void init_choose(ChooseInst *ci, Window parent, AppInst * inst);

void free_choose(ChooseInst *ci);

void init_choosers(XFontStruct * f);
void reset_choosers(void);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
