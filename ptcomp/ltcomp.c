/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/ltcomp.c: LineType/SectorType compiler.
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumb/levdatanums.h"
#include "libdumb/linetypestruct.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "ltcomp.h"
#include "protocomp.h"
#include "soundcomp.h"

typedef struct {
   LineType l;
   char name[NAMELEN];
} LineTypeRec;

static int nlts = 0, nsts = 0, maxlts = 0, maxsts = 0;
static LineTypeRec *lts, *sts;

static LT_TermType parm_termtype(void);
static int atolumptype(const char *s);
static int atoeventtype(const char *s);
static int ltacomp(const char *s, LT_Action *lta, int *stage);


void
init_ltcomp(void)
{
   maxlts = maxsts = ALLOC_BLK;
   lts = (LineTypeRec *) safe_calloc(maxlts, sizeof(LineTypeRec));
   sts = (LineTypeRec *) safe_calloc(maxsts, sizeof(LineTypeRec));
}

void
ltcomp(int issect)
{
   const char *s;
   LineTypeRec *p;
   LT_Action *lta = NULL;
   int id, nactions = 0;
   char myname[NAMELEN];
   int stage = 0;
   /* two parameters, name and id */
   s = parm_name(_("Name expected after Line/SectorType"));
   strncpy(myname, s, NAMELEN - 1);
   myname[NAMELEN - 1] = 0;
   id = parm_uint();
   /* allocate a sectortype */
   if (issect) {
      /* check allocation */
      if (id >= maxsts) {
	 maxsts = id + ALLOC_BLK;
	 sts = (LineTypeRec *) safe_realloc(sts, maxsts * sizeof(LineTypeRec));
	 memset(sts + nsts, 0, (maxsts - nsts) * sizeof(LineTypeRec));
      }
      /* deal with id */
      if (id >= nsts)
	 nsts = id + 1;
      p = sts + id;
   }
   /* allocate a linetype */
   else {
      /* check allocation */
      if (id >= maxlts) {
	 maxlts = id + ALLOC_BLK;
	 lts = (LineTypeRec *) safe_realloc(lts, maxlts * sizeof(LineTypeRec));
	 memset(lts + nlts, 0, (maxlts - nlts) * sizeof(LineTypeRec));
      }
      /* deal with id */
      if (id >= nlts)
	 nlts = id + 1;
      p = lts + id;
   }
   /* check uniqueness */
   if (p->name[0]) {
      err(_("ID %d (%s) not unique (clashes with %s)"),
	  id, myname, p->name);
   }
   strcpy(p->name, myname);
   /* start filling in data */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "Action")) {
	 lta = p->l.action + nactions++;
	 if (nactions > MAX_LT_ACTIONS)
	    err(_("Too many actions (max %d)"), (int) MAX_LT_ACTIONS);
	 s = parm_name(_("Lumptype expected after Action"));
	 lta->lumptype = atolumptype(s);
	 s = parm_name(_("Eventtype expected after Action"));
	 lta->eventtype = atoeventtype(s);
	 lta->sound = lta->stopsound = -1;
	 lta->speed[0] = default_speed;
      } else if (!strcasecmp(s, "KeyType"))
	 p->l.keytype = parm_uint();
      else if (!strcasecmp(s, "Damage"))
	 p->l.damage = parm_int();
      else if (!strcasecmp(s, "SpotType"))
	 p->l.spottype = parm_proto();
      else if (!strcasecmp(s, "Repeatable"))
	 p->l.flags |= LT_REPEATABLE;
      else if (!strcasecmp(s, "AllowPlayer"))
	 p->l.flags |= LT_ALLOW_PLAYER;
      else if (!strcasecmp(s, "AllowMonster"))
	 p->l.flags |= LT_ALLOW_NONPLAYER;
      else if (!strcasecmp(s, "OnThumped"))
	 p->l.flags |= LT_ON_THUMPED;
      else if (!strcasecmp(s, "OnCrossed"))
	 p->l.flags |= LT_ON_CROSSED;
      else if (!strcasecmp(s, "OnActivated"))
	 p->l.flags |= LT_ON_ACTIVATED;
      else if (!strcasecmp(s, "OnDamaged"))
	 p->l.flags |= LT_ON_DAMAGED;
      else if (!strcasecmp(s, "FrontOnly"))
	 p->l.flags |= LT_FRONT_ONLY;
      else if (!strcasecmp(s, "Scroll")) {
	 p->l.scrolldx = parm_speed();
	 p->l.scrolldy = parm_speed();
      } else if (lta == NULL)
	 break;
      else if (ltacomp(s, lta, &stage));
      else
	 break;
   }
   unget_token();
}

void
wrlts(WADWR *wout)
{
   int i;
   printf(_("%5d linetypes\n"), nlts);
   wadwr_lump(wout, "LINETYPE");
   for (i = 0; i < nlts; i++)
      wadwr_write(wout, &lts[i].l, sizeof(LineType));
}

void
wrsts(WADWR *wout)
{
   int i;
   printf(_("%5d sectortypes\n"), nsts);
   wadwr_lump(wout, "SECTTYPE");
   for (i = 0; i < nsts; i++)
      wadwr_write(wout, &sts[i].l, sizeof(LineType));
}


static LT_TermType
parm_termtype(void)
{
   const char *s = next_token();
   if (s == NULL || *s == '\n')
      ;
   else if (!strcasecmp(s, "Floor"))
      return Floor;
   else if (!strcasecmp(s, "LowestFloor"))
      return LowestFloor;
   else if (!strcasecmp(s, "LowestAdjacentFloor"))
      return LowestAdjacentFloor;
   else if (!strcasecmp(s, "HighestFloor"))
      return HighestFloor;
   else if (!strcasecmp(s, "HighestAdjacentFloor"))
      return HighestAdjacentFloor;
   else if (!strcasecmp(s, "NextHighestFloor"))
      return NextHighestFloor;
   else if (!strcasecmp(s, "NextLowestFloor"))
      return NextLowestFloor;
   else if (!strcasecmp(s, "Ceiling"))
      return Ceiling;
   else if (!strcasecmp(s, "LowestCeiling"))
      return LowestCeiling;
   else if (!strcasecmp(s, "LowestAdjacentCeiling"))
      return LowestAdjacentCeiling;
   else if (!strcasecmp(s, "HighestCeiling"))
      return HighestCeiling;
   else if (!strcasecmp(s, "HighestAdjacentCeiling"))
      return HighestAdjacentCeiling;
   else if (!strcasecmp(s, "NextHighestCeiling"))
      return NextHighestCeiling;
   else if (!strcasecmp(s, "NextLowestCeiling"))
      return NextLowestCeiling;
   else if (!strcasecmp(s, "FloorPlusSLT"))
      return FloorPlusSLT;
   else if (!strcasecmp(s, "FloorMinusSLT"))
      return FloorMinusSLT;
   else if (!strcasecmp(s, "OrigFloor"))
      return OrigFloor;
   else if (!strcasecmp(s, "OrigCeiling"))
      return OrigCeiling;
   else if (!strcasecmp(s, "One"))
      return TTOne;
   else if (!strcasecmp(s, "Zero"))
      return TTZero;
   else if (!strcasecmp(s, "DarkestSector"))
      return DarkestSector;
   else if (!strcasecmp(s, "DarkestAdjacentSector"))
      return DarkestAdjacentSector;
   else if (!strcasecmp(s, "LightestSector"))
      return LightestSector;
   else if (!strcasecmp(s, "LightestAdjacentSector"))
      return LightestAdjacentSector;
   /* dynamic lightlevels are actually darklevels */
   else if (!strcasecmp(s, "VeryDark"))
      return TTOne;
   else if (!strcasecmp(s, "VeryLight"))
      return TTZero;
   /* abbreviations */
   else if (!strcasecmp(s, "LIF"))
      return LowestFloor;
   else if (!strcasecmp(s, "LEF"))
      return LowestAdjacentFloor;
   else if (!strcasecmp(s, "HIF"))
      return HighestFloor;
   else if (!strcasecmp(s, "HEF"))
      return HighestAdjacentFloor;
   else if (!strcasecmp(s, "LIC"))
      return LowestCeiling;
   else if (!strcasecmp(s, "LEC"))
      return LowestAdjacentCeiling;
   else if (!strcasecmp(s, "HIC"))
      return HighestCeiling;
   else if (!strcasecmp(s, "HEC"))
      return HighestAdjacentCeiling;
   else if (!strcasecmp(s, "nhEF"))
      return NextHighestFloor;
   else if (!strcasecmp(s, "nlEF"))
      return NextLowestFloor;
   else if (!strcasecmp(s, "nhEC"))
      return NextHighestCeiling;
   else if (!strcasecmp(s, "nlEC"))
      return NextLowestCeiling;
   /* none of the above */
   else
      synerr(_("Termtype parameter expected"));
   return 0;
}

static int
atolumptype(const char *s)
{
   if (!strcasecmp(s, "Sector"))
      return ML_SECTOR;
   else if (!strcasecmp(s, "Side"))
      return ML_SIDE;
   else if (!strcasecmp(s, "Thing"))
      return ML_THING;
   else
      synerr(_("Strange lumptype"));
   return 0;
}

static int
atoeventtype(const char *s)
{
   if (!strcasecmp(s, "Ceiling"))
      return ME_CEILING;
   else if (!strcasecmp(s, "CeilingTexture"))
      return ME_CEILING_TEX;
   else if (!strcasecmp(s, "CeilingType"))
      return ME_CEILING_TYPE;
   else if (!strcasecmp(s, "Floor"))
      return ME_FLOOR;
   else if (!strcasecmp(s, "FloorTexture"))
      return ME_FLOOR_TEX;
   else if (!strcasecmp(s, "FloorType"))
      return ME_FLOOR_TYPE;
   else if (!strcasecmp(s, "Darkness"))
      return ME_LIGHT;
   else if (!strcasecmp(s, "Light"))
      return ME_LIGHT;
   else if (!strcasecmp(s, "SwitchOn"))
      return ME_SWITCHON;
   else if (!strcasecmp(s, "SwitchOff"))
      return ME_SWITCHOFF;
   else if (!strcasecmp(s, "Teleport"))
      return ME_TELEPORT;
   else if (!strcasecmp(s, "SecretLevel"))
      return ME_SECRETLEVEL;
   else if (!strcasecmp(s, "NewLevel"))
      return ME_NEWLEVEL;
   else
      synerr(_("Strange eventtype"));
   return 0;
}

static int
ltacomp(const char *s, LT_Action *lta, int *stage)
{
   if (!strcasecmp(s, "WaitFor")) {
      s = parm_name(_("Eventtype expected after WaitFor"));
      lta->waittype = atoeventtype(s);
   } else if (!strcasecmp(s, "Manual"))
      lta->flags |= LTA_MANUAL;
   else if (!strcasecmp(s, "ManualF"))
      lta->flags |= LTA_MANUAL_FRONT;
   else if (!strcasecmp(s, "DonutOuter"))
      lta->flags |= LTA_DONUT_OUTER;
   else if (!strcasecmp(s, "DonutInner"))
      lta->flags |= LTA_DONUT_INNER;
   else if (!strcasecmp(s, "Stair"))
      lta->flags |= LTA_STAIR;
   else if (!strcasecmp(s, "UnqueueAll"))
      lta->flags |= LTA_UNQUEUE_ALL;
   else if (!strcasecmp(s, "FastCrush"))
      lta->flags |= LTA_FASTCRUSH;
   else if (!strcasecmp(s, "SlowCrush"))
      lta->flags |= LTA_SLOWCRUSH;
   else if (!strcasecmp(s, "NoCrush"))
      lta->flags |= LTA_NOCRUSH;
   else if (!strcasecmp(s, "TriggerModel"))
      /* lta->flags |= LTA_TRIG_MODEL */;
   else if (!strcasecmp(s, "NumericModel"))
      lta->flags |= LTA_NUM_MODEL;
   /* else if (!strcasecmp(s, "Lockout")) lta->flags |= LTA_LOCKOUT; */
   else if (!strcasecmp(s, "Delay"))
      lta->delay = parm_time();
   else if (!strcasecmp(s, "Plus"))
      lta->term_offset[*stage] = parm_int();
   else if (!strcasecmp(s, "Minus"))
      lta->term_offset[*stage] = -parm_int();
   else if (!strcasecmp(s, "To")) {
      *stage = 0;
      lta->term_type[*stage] = parm_termtype();
   } else if (!strcasecmp(s, "BackTo")) {
      *stage = 1;
      lta->term_type[*stage] = parm_termtype();
      lta->speed[*stage] = -lta->speed[0];
   } else if (!strcasecmp(s, "Speed")) {
      int i = parm_speed();
      if (lta->speed[*stage] < 0)
	 lta->speed[*stage] = -i;
      else
	 lta->speed[*stage] = i;
   } else if (!strcasecmp(s, "Up")) {
      if (lta->speed[*stage] < 0)
	 lta->speed[*stage] = -lta->speed[*stage];
   } else if (!strcasecmp(s, "Down")) {
      if (lta->speed[*stage] > 0)
	 lta->speed[*stage] = -lta->speed[*stage];
   } else if (!strcasecmp(s, "SpawnType"))
      lta->spawntype = parm_proto();
   else if (!strcasecmp(s, "Sound"))
      lta->sound = parm_sound();
   else if (!strcasecmp(s, "ContSound"))
      lta->contsound = parm_sound();
   else if (!strcasecmp(s, "StartSound"))
      lta->sound = parm_sound();
   else if (!strcasecmp(s, "StopSound"))
      lta->stopsound = parm_sound();
   else
      return 0;
   return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
