/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/netio.c: Transport-independent network station & packet operations.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "netplay.h"
#include "net.h"

#define MAX_STATIONS 16
int nstations = 0;
RemoteStation *stations = NULL;

void
net_init(void)
{
   int i;
   stations = (RemoteStation *) safe_calloc(MAX_STATIONS,
					    sizeof(RemoteStation));
   for (i = 0; netdriver[i].name; i++)
      netdriver[i].init();
}
void
net_reset(void)
{
   int i;
   net_bufflush();
   for (i = 0; i < nstations; i++)
      stations[i].driver->reset_station(stations + i);
   for (i = 0; netdriver[i].name; i++)
      netdriver[i].reset();
}

int
net_initstation(int station, const char *name, int flags)
{
   RemoteStation *rs;
   int i;
   if (station < 0) {
      if (nstations >= MAX_STATIONS) {
	 logprintf(LOG_ERROR, 'N', _("too many stations"));
	 return -1;
      }
      station = nstations;
   }
   rs = stations + station;
   strncpy(rs->name, name, RS_NAME_LEN);
   rs->name[RS_NAME_LEN - 1] = 0;
   rs->flags = flags;
   rs->ackstate = 0;
   rs->player = -1;
   for (i = 0; netdriver[i].name; i++) {
      if (!netdriver[i].init_station(rs)) {
	 rs->driver = netdriver + i;
	 if (station >= nstations)
	    nstations = station + 1;
	 return station;
      }
   }
   return -1;
}

char *
net_getmyhost(void)
{
   /* get hostname from default driver */
   if (netdriver[0].getmyhost)
      return netdriver[0].getmyhost();
   else {
      logprintf(LOG_ERROR, 'N',
		_("network driver doesn't know how to gethostname()?"));
      return safe_strdup("anonymous");
   }
}


const unsigned char *
net_recpkt(size_t * pktlen, int *station)
{
   int i;
   *pktlen = 0;
   *station = -1;
   for (i = 0; netdriver[i].name; i++) {
      const unsigned char *pkt = netdriver[i].recpkt(pktlen, station);
      if (pkt)
	 return pkt;
   }
   return NULL;
}

int
net_waitpkt(RemoteStation *rs, int msec)
{
   return rs->driver->waitpkt(msec);
}

void
net_sendto(RemoteStation *rs, const void *pkt, size_t len)
{
   rs->driver->sendpkt(rs, pkt, len);
}

void
net_sendmaster_nobuf(const void *pkt, size_t len)
{
   int i;
   for (i = 0; i < nstations; i++)
      if (stations[i].flags & RS_MASTER)
	 net_sendto(stations + i, pkt, len);
}
void
net_slavecast_nobuf(const void *pkt, size_t len)
{
   int i;
   NetDriver *nd;
   /* first, send to all broadcast-capable drivers */
   for (nd = netdriver; nd->name; nd++)
      if (nd->slavecast)
	 nd->slavecast(pkt, len);
   /* then, send individually to slaves of non-bcast drivers */
   for (i = 0; i < nstations; i++)
      if ((stations[i].flags & RS_SLAVE)
	  && (stations[i].flags & RS_LIVE)
	  && (stations[i].driver->slavecast == NULL))
	 net_sendto(stations + i, pkt, len);
}

/* buffered send functions */
static unsigned char scbuf[NETBUF_LEN];
static int sclen = 0;
static NetSendFunc scsend = NULL;

void
net_bufsend(NetSendFunc func, const void *pkt, size_t len)
{
   if (len % 4)
      logprintf(LOG_ERROR, 'N',
		_("packet length not a multiple of 4 (type=%c len=%lu)"),
		*(const char *) pkt, (unsigned long) len);
   /* send the packet */
   if (scsend != func) {
      net_bufflush();
      scsend = func;
   }
   if (len + sclen > NETBUF_LEN)
      net_bufflush();
   memcpy(scbuf + sclen, pkt, len);
   sclen += len;
}

void
net_bufflush(void)
{
   if (scsend && sclen > 0)
      scsend(scbuf, sclen);
   sclen = 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
