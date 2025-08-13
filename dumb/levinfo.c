/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/levinfo.c: LEVINFO lump handling.
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
#include <stdio.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "levdyn.h"
#include "levinfo.h"

static int ninfo = 0;
static const LevInfo *info = NULL;
static LumpNum info_ln = BAD_LUMPNUM;

void
init_levinfo(void)
{
   if (info)
      return;
   info_ln = lookup_lump("LEVINFO", NULL, NULL);
   if (!LUMPNUM_OK(info_ln)) {
      logprintf(LOG_WARNING, 'M', _("warning: no LEVINFO lump for this game"));
      ninfo = -1;
      return;
   }
   ninfo = get_lump_len(info_ln) / sizeof(LevInfo);
   info = (const LevInfo *) load_lump(info_ln);
   logprintf(LOG_INFO, 'M', _("Init %d levels"), ninfo);
}

void
reset_levinfo(void)
{
   if (LUMPNUM_OK(info_ln))
      free_lump(info_ln);
   info_ln = BAD_LUMPNUM;
   info = NULL;
   ninfo = 0;
}

const LevInfo *
find_levinfo(LevData *ld)
{
   int i;
   const LevInfo *li;
   if (ninfo == 0)
      init_levinfo();
   if (ld->levinfo_id >= 0)
      return info + ld->levinfo_id;
   if (ninfo < 0)
      return NULL;
   for (i = 0, li = info; i < ninfo; i++, li++)
      if (!strcasecmp(ld->name, li->name))
	 break;
   if (i >= ninfo)
      return NULL;
   ld->levinfo_id = i;
   return li;
}

static void
load(LevData *ld, const LevInfo *li)
{
   int d = ld->difficulty, m = ld->mplayer, i;
   int lp = ld->localplayer;
   int plhits[MAXPLAYERS], plarm[MAXPLAYERS];
   /* sanity check */
   if (li == NULL || li > info + ninfo)
      logfatal('M', _("bogus levinfo pointer in levinfo_load"));
   /* save old level's player hit points */
   for (i = 0; i < MAXPLAYERS; i++) {
      if (ld->player[i] >= 0) {
	 plhits[i] = ldthingd(ld)[ld->player[i]].hits;
	 plarm[i] = ldthingd(ld)[ld->player[i]].armour;
      } else {
	 plhits[i] = -1;
	 plarm[i] = -1;
      }
   }
   /* load new level */
   reset_level(ld);
   load_level(ld, li->name, d, m);
   ld->levinfo_id = li - info;
   /* restore player number */
   ld->localplayer=lp;
   /* restore player hps */
   for (i = 0; i < MAXPLAYERS; i++) {
      if (ld->player[i] >= 0 && plhits[i] >= 0) {
	 ldthingd(ld)[ld->player[i]].hits = plhits[i];
	 ldthingd(ld)[ld->player[i]].armour = plarm[i];
      }
   }
}

void levinfo_startgame(LevData *ld, int episode, int difficulty, int mplayer);

void
levinfo_next(LevData *ld, int secret)
{
   const LevInfo *li = find_levinfo(ld);
   int id;
   if (!li)
      logfatal('M', _("no levinfo for %s, so can't find next level"),
	       ld->name);
   if (secret)
      id = li->secret;
   else
      id = li->next;
   if (id >= 0)
      load(ld, info + id);
   else
      load(ld, ++li);
}

const char *
get_skyname(LevData *ld)
{
   const LevInfo *li = find_levinfo(ld);
   if (!li)
      return "SKY1";
   return li->sky;
}

// Local Variables:
// c-basic-offset: 3
// End:
