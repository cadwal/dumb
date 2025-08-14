/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/thinghit.c: Actions from linetypes.
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

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "levdyn.h"
#include "things.h"
#include "updmap.h"
#include "game.h"
#include "gettable.h"
#include "linetype.h"

/*#define THINGHIT_DEBUG */

static int
find_taggedsect(const LevData *ld, int tag, int i)
{
   while (i < ldnsectors(ld)) {
      if (ldsector(ld)[i].tag == tag)
	 return i;
      i++;
   }
   return -1;
}

static int
find_next_step(const LevData *ld, int sect)
{
   int i;
   int nusect = -1;
   /* find first 2S line with side 0 in this sector */
   for (i = 0; i < ldnlines(ld); i++)
      if (ldline(ld)[i].side[0] >= 0 && ldline(ld)[i].side[1] >= 0 &&
	  ldside(ld)[ldline(ld)[i].side[0]].sector == sect)
	 break;
   if (i < ldnlines(ld))
      nusect = ldside(ld)[ldline(ld)[i].side[1]].sector;
   if (nusect < 0)
      return nusect;
   /* check floor matches */
   if (ldsectord(ld)[nusect].ftex != ldsectord(ld)[sect].ftex)
      return -1;
   /* done */
   return nusect;
}

static int
find_num_model(const LevData *ld, int sect)
{
   int i, j;
   /* find first 2S line with a side in this sector */
   for (i = 0; i < ldnlines(ld); i++)
      for (j = 0; j < 2; j++)
	 if (ldline(ld)[i].side[j] >= 0 && ldline(ld)[i].side[1] >= 0 &&
	     ldside(ld)[ldline(ld)[i].side[j]].sector == sect)
	    return ldside(ld)[ldline(ld)[i].side[1 - j]].sector;
   /* failed */
   return -1;
}

static int
find_donut_sector(const LevData *ld, int core)
{
   /* TODO: write this function */
   return core;
}

static int
find_spot(const LevData *ld, const LineType *lt, int tag)
{
   int i;
   int sector = find_taggedsect(ld, tag, 0);	/* TODO: pick a random tagged sect */
   const ThingDyn *td = ldthingd(ld);
   logprintf(LOG_DEBUG, 'M', "find_spot: tag=%d sector=%d spottype=%d",
	     tag, sector, (int) (lt->spottype));
   if (sector < 0)
      return -1;
   for (i = 0; i < ldnthings(ld); i++, td++)
      if (td->proto && td->proto->id == lt->spottype && td->sector == sector) {
	 logprintf(LOG_DEBUG, 'M', _("find_spot: found %d"), i);
	 return i;
      }
   logprintf(LOG_DEBUG, 'M', _("find_spot: no spot found"));
   return -1;
}

/* th is where to make the fog, who is who is being teleported */
static void
spawn_fog(LevData *ld, int th, int who, int prid)
{
   const ThingDyn *td = ldthingd(ld) + th;
   const ThingDyn *tdwho = ldthingd(ld) + who;
   fixed x = td->x, y = td->y, z = td->z;
   x += (9 * fixcos(td->angle)) / 8;
   y += (9 * fixsin(td->angle)) / 8;
   z += (3 * tdwho->proto->height) / 4;
   new_thing(ld, prid, x, y, z);
}

static void
perform_666(LevData *ld, int sect)
{
   MapEvent *me;
   logprintf(LOG_DEBUG, 'M', _("boss tag effect on sector %d"), sect);
   unqueue_event(ld, ML_SECTOR, ME_FLOOR, sect, NULL);
   me = insert_event(ld, ML_SECTOR, ME_FLOOR, sect, NULL);
   me->delta[0] = -1 << 10;
   me->term[0] = get_term_type(ld, LowestAdjacentFloor, sect);
}

/* call this when the level boss is dead */
void
do_tag666(LevData *ld)
{
   int i;
   for (i = 0; i < ldnsectors(ld); i++)
      if (ldsector(ld)[i].tag == 666)
	 perform_666(ld, i);
}

/* the heart of the LT_Action mechanism: queues appropriate events for
   a given LT_Action */
void
perform_lta(LevData *ld, const LT_Action *lta, int on, int targ)
{
   MapEvent *me;
   int ttfrom = on;
   int stairiter = 1;
   int i;

   /* donut inner gets its term from donut, not tagged */
   if (lta->flags & LTA_DONUT_INNER)
      ttfrom = find_donut_sector(ld, on);
   if (ttfrom < 0)
      ttfrom = on;

   /* stairs loop (don't worry, we bail out if not LTA_STAIR) */
   while (on >= 0) {
      /* deal with spawntype */
      if (lta->spawntype > 0) {
	 spawn_fog(ld, on, on, lta->spawntype);
	 if (targ >= 0)
	    spawn_fog(ld, targ, on, lta->spawntype);
      }
      /* prevent queuing of events that are already happening */
      /* this also bails out of Escher stairs */
      if ((lta->delay == 0) && !(lta->flags & LTA_UNQUEUE_ALL) &&
	  find_active_event(ld, lta->lumptype, lta->eventtype, on))
	 break;

      /* unqueue future events of this type */
      if (lta->flags & LTA_UNQUEUE_ALL)
	 unqueue_event(ld, lta->lumptype, lta->eventtype, on, NULL);
      else
	 unqueue_event(ld, lta->lumptype, lta->eventtype, on, lta);
      /* OK, create that event */
      me = insert_event(ld, lta->lumptype, lta->eventtype, on, lta);

      /* deal with delay and sound */
      me->start += lta->delay;
      me->wait = lta->waittype;
      me->sound = lta->sound;
      me->stopsound = lta->stopsound;
      me->contsound = lta->contsound;

      /* deal with model */
      if (lta->flags & LTA_NUM_MODEL)
	 me->model = find_num_model(ld, on);
      else
	 me->model = targ;

      /* deal with crush effects */
      if (lta->flags & LTA_NOCRUSH)
	 me->crush_effect = MEC_REVERSE;
      else if (lta->flags & LTA_SLOWCRUSH)
	 me->crush_effect = MEC_SLOWHURT;
      else if (lta->flags & LTA_FASTCRUSH)
	 me->crush_effect = MEC_FASTHURT;

#ifdef THINGHIT_DEBUG
      logprintf(LOG_DEBUG, 'M',
		"perform_lta: evtype=%d termtype[1]=%d delta[1]=%d",
		lta->eventtype, lta->term_type[1], lta->speed[1]);
#endif

      for (i = 0; i < 2; i++) {
	 /* deal with speed */
	 me->delta[i] = lta->speed[i];
	 /* deal with term (sectors only) */
	 if (lta->lumptype == ML_SECTOR) {
	    me->term[i] = get_term_type(ld, (int) (lta->term_type[i]), ttfrom);
	    me->term[i] += stairiter * (fixed) (lta->term_offset[i]) << 12;
	 }
      }

      /* deal with targ */
      if (targ >= 0 && lta->lumptype == ML_THING) {
	 ThingDyn *td = ldthingd(ld) + targ;
	 me->x = td->x;
	 me->y = td->y;
	 me->z = td->z;
	 me->angle = td->angle;
      }
      /* find next stair, if applicable */
      if (lta->flags & LTA_STAIR)
	 on = find_next_step(ld, on);
      else
	 on = -1;
      stairiter++;
   }
}

void
thing_hit_wall(LevData *ld, int thing, int wall, int isback, WallHitType t)
{
   int front, back, i, pl;
   ThingDyn *td = ldthingd(ld) + thing;
   LineDyn *wd = ldlined(ld) + wall;
   const LineData *w = ldline(ld) + wall;
   const LineType *lt = lookup_linetype(w->type);

#ifdef THINGHIT_DEBUG
   logprintf(LOG_DEBUG, 'M',
	   "thing_hit_wall(th=%d,wall=%d,side=%d,wht=%d,type=%d,lt=%ld)",
	     thing, wall, isback, (int) t, (int) (w->type), (long) lt);
#endif

   if (lt == NULL)
      return;

   /* check reactivation */
   if (wd->block_activation)
      return;

   /* check direction */
   if ((lt->flags & LT_FRONT_ONLY) && isback)
      return;

   /* check to see whether this wall is being activated in appropriate way */
   switch (t) {
   case (Crossed):
      if (lt->flags & LT_ON_CROSSED)
	 break;
      else
	 return;
   case (Thumped):
      if (lt->flags & LT_ON_THUMPED)
	 break;
      else
	 return;
   case (Activated):
      if (lt->flags & LT_ON_ACTIVATED)
	 break;
      else
	 return;
   case (Damaged):
      if (lt->flags & LT_ON_DAMAGED)
	 break;
      else
	 return;
   default:
      logprintf(LOG_ERROR, 'M',
		_("strange WallHitType (%d) in thing_hit_wall()"),
		t);
      return;
   }

   /* check if player / monster is allowed */
   /* if t==Damaged, td might be a missile or bullet, so skip this test */
   if (t != Damaged) {
      if ((td->proto->flags & PT_PLAYER) && !(lt->flags & LT_ALLOW_PLAYER)) {
#ifdef THINGHIT_DEBUG
	 logprintf(LOG_DEBUG, 'M',
		   _("player is not allowed to activate type %d"),
		   (int) (ldline(ld)[wall].type));
#endif
	 return;
      }
      if (!(td->proto->flags & PT_PLAYER) && !(lt->flags & LT_ALLOW_NONPLAYER)) {
#ifdef THINGHIT_DEBUG
	 logprintf(LOG_DEBUG, 'M',
		   _("non-player is not allowed to activate type %d"),
		   (int) (ldline(ld)[wall].type));
#endif
	 return;
      }
   }
   /* check keytype */
   pl = thing2player(ld, thing);
   if (pl >= 0 && lt->keytype && !gettable_chk_key(ld, pl, lt->keytype)) {
      if (!wd->had_keymsg)
	 game_utf8_message(pl, U_("You need a key!"));
      wd->had_keymsg = 1;
      return;
   }
   /* looks like we can go ahead */
#ifdef THINGHIT_DEBUG
   logprintf(LOG_DEBUG, 'M',
	     _("line %d is having something done to it by thing %d"),
	     wall, thing);
#endif

   /* block future activation if effect is not repeatable */
   if (!(lt->flags & LT_REPEATABLE))
      wd->block_activation = 1;

   /* find front + back sides of wall for manual ops */
   front = ldline(ld)[wall].side[0];
   back = ldline(ld)[wall].side[1];

   /* perform actions */
   for (i = 0; i < MAX_LT_ACTIONS; i++) {
      const LT_Action *lta = lt->action + i;
      int model = -1;
      if (NO_ACTION(lta))
	 break;
      switch (lta->lumptype) {

      case (ML_SECTOR):
	 /* find trigger model, in case it's needed */
	 if (front >= 0)
	    model = ldside(ld)[front].sector;
	 /* deal with "manual" actions */
	 if (lta->flags & LTA_MANUAL) {
	    if (back < 0 || ldside(ld)[back].sector < 0)
	       logprintf(LOG_ERROR, 'M',
			 _("tried manual action on wall with no back?"));
	    else
	       perform_lta(ld, lta, ldside(ld)[back].sector, -1);
	 } else if (lta->flags & LTA_MANUAL_FRONT) {
	    if (front < 0 || ldside(ld)[front].sector < 0)
	       logprintf(LOG_ERROR, 'M',
		      _("tried manual_f action on wall with no front?"));
	    else
	       perform_lta(ld, lta, ldside(ld)[front].sector, -1);
	 } else {
	    /* act on all tagged sectors */
	    int j, tag = ldline(ld)[wall].tag;
	    for (j = 0; j < ldnsectors(ld); j++) {
	       if (ldsector(ld)[j].tag == tag) {
		  /* DONUT_OUTER: perform on donut instead of tagged */
		  if (lta->flags & LTA_DONUT_OUTER) {
		     int k = find_donut_sector(ld, j);
		     if (k >= 0)
			perform_lta(ld, lta, k, model);
		  } else
		     perform_lta(ld, lta, j, model);
	       }
	    }
	 }
	 break;

      case (ML_SIDE):
	 if (lta->flags & LTA_MANUAL)
	    perform_lta(ld, lta, back, -1);
	 if (lta->flags & LTA_MANUAL_FRONT)
	    perform_lta(ld, lta, front, -1);
	 break;

      case (ML_THING):
	 if (lt->spottype)
	    perform_lta(ld, lta, thing, find_spot(ld, lt, w->tag));
	 else
	    perform_lta(ld, lta, thing, -1);
	 break;

      case (ML_LINE):
	 perform_lta(ld, lta, wall, -1);
	 break;

	 /* linetype refered to an action on some strange lumptype */
      default:
	 logprintf(LOG_ERROR, 'M',
		   _("strange lumptype (%d) while performing actions"),
		   (int) (lta->lumptype));
	 break;
      }
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
