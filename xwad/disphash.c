/* DUMB: A Doom-like 3D game engine.
 *
 * xwad/disphash.c: Dispatching window events.
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
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "disphash.h"

typedef struct _DHEnt {
   struct _DHEnt *chain;
   Window w;
   DispatchFunc func;
   AppInst *inst;
   void *info;
} DHEnt;

#define HASHSIZE 29

#define HASH(wn) ((wn)%HASHSIZE)

static DHEnt tbl[HASHSIZE];

void
init_dh(void)
{
   int i;
   for (i = 0; i < HASHSIZE; i++) {
      tbl[i].w = None;
      tbl[i].chain = NULL;
   }
}

void
add_dh(Window w, DispatchFunc func, AppInst *inst, void *info)
{
   DHEnt *dh = tbl + HASH(w);
   while (dh->w != None && dh->w != w && dh->chain != NULL)
      dh = dh->chain;
   if (dh->w != None && dh->w != w) {
      dh->chain = (DHEnt *) calloc(1, sizeof(DHEnt));
      dh = dh->chain;
   }
   dh->w = w;
   dh->func = func;
   dh->inst = inst;
   dh->info = info;
}

void
remove_dh(Window w)
{
   DHEnt *dh = tbl + HASH(w);
   while (dh->w != w && dh->chain != NULL)
      dh = dh->chain;
   if (dh->w == w)
      dh->w = None;
}

int
dispatch(XEvent *ev)
{
   Window w = ev->xany.window;
   DHEnt *dh = tbl + HASH(w);
   while (dh->w != w && dh->chain != NULL)
      dh = dh->chain;
   if (dh->w == w)
      dh->func(ev, dh->inst, dh->info);
   else
      return -1;
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
