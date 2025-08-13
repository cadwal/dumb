/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/net.h: Declarations for variables each network driver must provide.
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

#ifndef PLAT_NET_H
#define PLAT_NET_H

#include "libdumbutil/confdef.h"

struct RemoteStation_struct;

typedef struct {
   const char *name;
   void (*init) (void);
   void (*reset) (void);
   int (*init_station) (struct RemoteStation_struct * rs);
   void (*reset_station) (struct RemoteStation_struct * rs);
   const unsigned char *(*recpkt) (size_t * size, int *station);
   int (*waitpkt) (int msec);
   void (*sendpkt) (struct RemoteStation_struct * rs,
		    const void *pkt, size_t size);
   void (*slavecast) (const void *pkt, size_t size);
   void (*broadcast) (const void *pkt, size_t size);
   void (*getmyhost) (char *buf, size_t len);
} NetDriver;

extern NetDriver netdriver[];

extern ConfItem net_conf[];

#endif

// Local Variables:
// c-basic-offset: 3
// End:
