/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/thingm.c: Moving things.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1994 by Chris Laurel
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
#include <stdlib.h>
#include <limits.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/fixed.h"
#include "libdumbutil/timer.h"
#include "libdumb/dsound.h"
#include "things.h"
#include "game.h"

/* #define THINGM_DEBUG */

#define MAX_DELEV (FIXED_ONE/8)
void
player_apply_lookup(const LevData *ld, int th, int lookup)
{
   ThingDyn *td = ldthingd(ld) + th;
   td->delev += INT_TO_FIXED(lookup) / 12;
   td->delev -= td->elev / 12;
}

static int
appropriate_target(const LevData *ld, int targ, int th, fixed arc)
{
   const ThingDyn *td = ldthingd(ld) + th;
   const ThingDyn *ttd = ldthingd(ld) + targ;
   fixed t;
   if (targ < 0 || targ == th)
      return 0;
   if (ttd->proto == NULL)
      return 0;
   if ((td->proto->flags & PT_BOGUS) && td->owner == targ)
      return 0;
   if (!(ttd->proto->flags & PT_TARGET))
      return 0;
   if (ttd->hits <= 0)
      return 0;
   t = fix_vec2angle(ttd->x - td->x, ttd->y - td->y) - td->angle;
   NORMALIZE_ANGLE(t);
   return t < arc / 2 || t > (FIXED_2PI - arc / 2);
}

#define AUTOTARG_MAXDIST (512<<12)

void
thing_autotarget(const LevData *ld, int th, fixed arc)
{
   ThingDyn *td = ldthingd(ld) + th;
   /* find a target */
   fixed minlen = AUTOTARG_MAXDIST;
   int i, is;
   td->target = -1;
   i = is = ((unsigned) rand()) % ldnthings(ld);
   do {
      if (appropriate_target(ld, i, th, arc)) {
	 fixed l = fix_pyth3d(ldthingd(ld)[i].x - td->x,
			      ldthingd(ld)[i].y - td->y,
			      ldthingd(ld)[i].z - td->z);
	 if (l < minlen) {
	    minlen = l;
	    td->target = i;
	 }
      }
      i++;
      if (i >= ldnthings(ld))
	 i = 0;
   } while (i != is);
#ifdef THINGM_DEBUG
   logprintf(LOG_DEBUG, 'O', "thing_autotarget(%d) targ=%d",
	     th, td->target);
#endif
}

#define HEATSEEK_ARC (FIXED_PI/3)	/* 60 degrees */

static void
do_heatseek(const LevData *ld, int th)
{
   ThingDyn *td = ldthingd(ld) + th;
   /* find a target */
   if (!appropriate_target(ld, td->target, th, HEATSEEK_ARC))
      thing_autotarget(ld, th, HEATSEEK_ARC);
   /* if we found one, try to steer towards it */
   if (td->target >= 0) {
      const ThingDyn *ttd = ldthingd(ld) + td->target;
      fixed speed = fix_pythagoras(td->dx, td->dy);
      fixed t = fix_vec2angle(ttd->x - td->x, ttd->y - td->y) - td->angle;
      fixed dz = ttd->z + ttd->proto->height / 2 - td->z;
#ifdef THINGM_DEBUG
      logprintf(LOG_DEBUG, 'O', _("Obj %d is heatseeking %d"),
		th, td->target);
#endif
      NORMALIZE_ANGLE(t);
      if (t < HEATSEEK_ARC / 2)
	 td->angle += t / 3;
      if (t > (FIXED_2PI - HEATSEEK_ARC / 2))
	 td->angle += (t - FIXED_2PI) / 3;
      td->dx = td->dy = 0;
      thingd_apply_forward(td, speed);
      if (dz > speed / 3)
	 td->dz = speed / 3;
      else if (dz < -speed / 3)
	 td->dz = -speed / 3;
      else
	 td->dz = dz / 3;
      td->elev = fix_vec2angle(speed, td->dz);
      /*logprintf(LOG_DEBUG, 'O', _("heatseek: dz=%f speed=%f elev=%f"),
         FIXED_TO_FLOAT(td->dz), FIXED_TO_FLOAT(speed),
         FIXED_TO_FLOAT(td->elev)); */
   }
}

int
thing_can_see(const LevData *ld, int th, int targ)
{
   if (reject_thing_thing(ld, th, targ))
      return 0;
   else {
      fixed sarc = ldthingd(ld)[th].proto->see_arc;
      fixed a = fix_vec2angle(ldthingd(ld)[targ].x - ldthingd(ld)[th].x,
			      ldthingd(ld)[targ].y - ldthingd(ld)[th].y);
      a -= ldthingd(ld)[th].angle;
      NORMALIZE_ANGLE(a);
      if (a < sarc / 2 || a > FIXED_2PI - sarc / 2) {
	 if (thing_can_shoot_at(ld, th, targ))
	    return 1;
	 else
	    return 0;
      } else
	 return 0;
   }
}


extern inline double
pyth_sq(fixed x, fixed y)
{
   double dx = FIXED_TO_FLOAT(x), dy = FIXED_TO_FLOAT(y);
   return dx * dx + dy * dy;
}
#define SQ(x) ((x)*(x))

/* for now, everyone hates players! */
#define emnity(p1,p2) ((p2)->flags&PT_PLAYER)

void
thing_find_enemy(const LevData *ld, int th)
{
   ThingDyn *td = ldthingd(ld) + th;
   int targ, ts;
   const ThingDyn *ttd;
   /* random start ensures that monsters won't all fixate on one player */
   /* Should we check every object to see if it could be a target? */
   if (td->proto->flags & PT_USE_EMNITY) {
      targ = ts = ((unsigned) rand()) % ldnthings(ld);
      do {
	 ttd = ldthingd(ld) + targ;
	 if (ttd->proto &&
	     ttd->hits > 0 &&
	     emnity(td->proto, ttd->proto) &&
	     thing_can_see(ld, th, targ)) {
	    td->target = targ;
	    break;
	 }
	 targ++;
	 if (targ >= ldnthings(ld))
	    targ = 0;
      } while (targ != ts);
   }
   /* just check for live players */
   else {
      targ = ts = ((unsigned) rand()) % MAXPLAYERS;
      do {
	 ttd = ldthingd(ld) + ld->player[targ];
	 if (ld->player[targ] >= 0 &&
	     ttd->proto &&
	     ttd->hits > 0 &&
	     thing_can_see(ld, th, ld->player[targ])) {
	    td->target = ld->player[targ];
	    break;
	 }
	 targ++;
	 if (targ == MAXPLAYERS)
	    targ = 0;
      } while (targ != ts);
   }
}

/* how close a monster will try to aim */
#define CLOSE_ENOUGH_ARC (FIXED_PI/16)
/* arc around front of enemy that monsters consider dangerous */
#define DODGE_ARC (FIXED_PI/6)
/* angle that monster will try to follow when dodging */
#define DODGE_ANGLE (FIXED_PI/8)

/* slow speeds for fine turning */
#define FINE_TURNSPEED (FIXED_ONE/4)
#define VERY_FINE_TURNSPEED (FIXED_ONE/16)

static void
do_strategy(const LevData *ld, int th)
{
   ThingDyn *td = ldthingd(ld) + th;

   /* trash dead targets */
   if (td->target >= 0 && ldthingd(ld)[td->target].proto == NULL)
      td->target = -1;

   /* need a new one? */
   if (td->target < 0) {
      thing_find_enemy(ld, th);
#ifdef THINGM_DEBUG
      if (td->target >= 0)
	 logprintf(LOG_DEBUG, 'O', _("thing %d is out to get %d!"),
		   th, td->target);
#endif
   }
   /* chase after and attack our target */
   if (td->target >= 0) {
      ThingDyn *ttd = ldthingd(ld) + td->target;
      fixed angle_to_target = fix_vec2angle(ttd->x - td->x, ttd->y - td->y);
      fixed t = angle_to_target - td->angle; 
      fixed t2 = FIXED_PI + angle_to_target - ttd->angle;
      double d2 = pyth_sq(ttd->x - td->x, ttd->y - td->y);
      fixed attack_dist = td->proto->radius + ttd->proto->radius;
      double attd2,bkod2;
      fixed dz = (ttd->z + ttd->proto->height/2) - 
	 (td->z + td->proto->height/2);
      fixed movespeed = fixmul(td->proto->realmass, td->proto->speed);
      fixed turnspeed = 3 * movespeed / 4;
      NORMALIZE_ANGLE(t);
      NORMALIZE_ANGLE(t2);
      /* figure out a good distance to attack from */
      if (td->proto->signals[TS_FIGHT] < 0) {
	 attack_dist += INT_TO_FIXED(4);
	 attd2 = SQ(FIXED_TO_FLOAT(attack_dist));
	 bkod2 = attd2;
      }
      else {
	 attack_dist += FIXED_ONE;
	 attd2 = SQ(FIXED_TO_FLOAT(attack_dist));
	 bkod2 = /* 3 * attd2 / 4 */ attd2/2;
      };
      /* if they're in our sights, fight or shoot */
      if (t < CLOSE_ENOUGH_ARC / 2 || t > FIXED_2PI - CLOSE_ENOUGH_ARC / 2) {
	 if (reject_thing_thing(ld, th, td->target));
	 else if (td->proto->signals[TS_FIGHT] >= 0 && d2 <= attd2)
	    thing_send_sig(ld, th, TS_FIGHT);
	 else if (!thing_can_shoot_at(ld, th, td->target));
#if 0 /* make all monsters nasty, they need it if they're going to dodge */
	 else if (td->proto->flags & PT_FAST_SHOOTER)
	    thing_send_sig(ld, th, TS_SHOOT);
	 /* if we're just standing there, attack more often */
	 else if ((d2 <= attd2) && (!rand() & 3))
	    thing_send_sig(ld, th, TS_SHOOT);
	 /* sometimes just shoot anyway */
	 else if (!(rand() & 3))
	    thing_send_sig(ld, th, TS_SHOOT);
#endif
	 else
	    thing_send_sig(ld, th, TS_SHOOT);
      }
      /* if we're in their sights, dodge, unless,
         1. we're a vicious nasty monster, or,
	 2. there might be an impassible wall in the way, or,
	 3. we're in melee
      */
      /*if ((ld->map_ticks-td->last_hit_wall)<1000/MSEC_PER_TICK) 
	 logprintf(LOG_DEBUG,'O',"dodge for %d inhibited by impassible wall",
	 th);*/
      if (!(td->proto->flags & PT_FAST_SHOOTER) 
	  && !((ld->map_ticks-td->last_hit_wall)<1000/MSEC_PER_TICK)
	  && !(td->proto->signals[TS_FIGHT] >= 0 && d2 <= attd2)
	  && (t2 < DODGE_ARC / 2 || t2 > FIXED_2PI - DODGE_ARC / 2)) 
      {
	 /*logprintf(LOG_DEBUG,'O',"monster %d is dodging",th);*/
	 if (t < FIXED_PI) {
	    if (t < DODGE_ANGLE)
	       thingd_apply_torque(td, -turnspeed);
	    else if(t > FIXED_PI/2)
	       thingd_apply_torque(td, turnspeed);
	 }
	 else /* >= FIXED_PI */ {
	    if(t > (FIXED_2PI-DODGE_ANGLE)) 
	       thingd_apply_torque(td, turnspeed);
	    else if(t < FIXED_PI+FIXED_PI/2)
	       thingd_apply_torque(td, -turnspeed);
	 }
	 thingd_apply_forward(td, movespeed);
      }
      /* otherwise, go towards them */
      else {
	 /* turn in right direction */
	 fixed myturn,abst;
	 if (t < FIXED_PI) {
	    myturn=1;
	    abst=t;
	 }
	 else {
	    myturn=-1;
	    abst=FIXED_2PI-t;
	 }
	 if(abst<CLOSE_ENOUGH_ARC/2) myturn*=VERY_FINE_TURNSPEED;
	 else if(abst<CLOSE_ENOUGH_ARC) myturn*=FINE_TURNSPEED;
	 else myturn*=turnspeed;
	 if(myturn) 
	    thingd_apply_torque(td, myturn);
	 /* back off if we're too close */
	 if (d2 < bkod2)
	    thingd_apply_forward(td, -movespeed / 4);
	 /* move towards foe */
	 else if (d2 > attd2 * 2)
	    thingd_apply_forward(td, movespeed);
	 /* move slower when you're close */
	 else if (d2 > attd2)
	    thingd_apply_forward(td, movespeed / 2);
	 /* flyers can move up and down too */
	 if (td->proto->flags & PT_CAN_FLY) {
	    if (dz > td->proto->height)
	       thingd_apply_up(td, movespeed / 2);
	    else if (dz < -td->proto->height)
	    thingd_apply_up(td, -movespeed / 2);
	    else if (dz > 0)
	       thingd_apply_up(td, movespeed / 4);
	    else if (dz < -td->proto->height/2)
	       thingd_apply_up(td, -movespeed / 4);
	 }
      }
      /* give up if they seem dead */
      if (!(ttd->proto->flags & PT_BEASTIE))
	 td->target = -1;
      if (ttd->hits <= 0)
	 td->target = -1;
   }
}

static void
spawn_or_hurl(const LevData *ld, int th, int prid)
{
   ThingDyn *td = ldthingd(ld) + th;
   int nth=new_thing(ld, prid, td->x, td->y, td->z);
   ldthingd(ld)[nth].owner=th;
}

/* wake anyone who can see me */
void
thing_wake_others(const LevData *ld, int th, int tickspassed)
{
   int i;
   for (i = 0; i < ldnthings(ld); i++) {
      ThingDyn *td = ldthingd(ld) + i;
      const ThingPhase *tph = td->phase_tbl + td->phase;
      td->wakeness += tickspassed;
      if ((td->wakeness >= WAKE_TICKS) && i != th && td->proto
	  && (tph->flags & TPH_IDLE) && thing_can_see(ld, i, th))
	 thing_send_sig(ld, i, TS_DETECT);
      td->wakeness %= WAKE_TICKS;
   }
}

/* take damage, potentially ouching, dying, or exploding */
void
thing_take_damage(const LevData *ld, int th, int dmg)
{
   ThingDyn *td = ldthingd(ld) + th;
   int hitdmg, armdmg;

   /* bail out if dmg<=0 */
   if (dmg <= 0)
      return;

   /* if hits<0 either we're already dead,
    * or we were invulnerable to begin with (proto->hits==-1)
    */
   if (td->hits < 0)
      return;

   /* check if we explode before checking hits */
   if ((td->proto->flags & PT_EXPLOSIVE)) {
      /* stop explosions from being knocked around */
      td->dx = td->dy = td->dz = 0;	
      /*logprintf(LOG_DEBUG,'O',"explosive %d set off by dmg (%d)",th,dmg);*/
      thing_send_sig(ld, th, TS_EXPLODE);
      return;
   }

   /* temporary invulnerability */
   if (td->tmpinv > 0)
      return;

#ifdef THINGM_DEBUG
   logprintf(LOG_DEBUG, 'O', _("thing %d took %d damage (%d/%d hits, %d/%d armour)"),
	     th, dmg,
	     td->hits, (int) (td->proto->hits),
	     td->armour, (int) (td->proto->armour));
#endif

   /* this should divide damage nicely between armour and health */
   armdmg = (dmg + (rand() < (RAND_MAX / 2))) / 2;
   hitdmg = dmg - armdmg;
   td->armour -= armdmg;
   td->hits -= hitdmg;
   if (td->armour < 0) {
      td->hits += td->armour;
      td->armour = 0;
   }

   /* we survived */
   if (td->hits > 0) {
      thing_send_sig(ld, th, TS_OUCH);
      return;
   }
   /* we didn't: how sad, never mind */
   if (td->hits < -td->proto->hits && thing_send_sig(ld, th, TS_EXPLODE) != -1)
      return;
   thing_send_sig(ld, th, TS_DIE);
}

/* apply torque so that td turns towards angle
   intensity should be between 1 and 0
   this fn is not currently used: I don't know whether or not it works
 */
/*
void
thingd_apply_torque_towards(ThingDyn *td, fixed angle, fixed intensity)
{
   angle-=td->angle;
   NORMALIZE_ANGLE(angle);
   if(angle<FIXED_PI) td->dangle+=fixmul(angle,intensity);
   else td->dangle+=fixmul(FIXED_2PI-angle,intensity);
} */

static int
unowned_owner(const LevData *ld, int th)
{
   int o;
   while ((o = ldthingd(ld)[th].owner) >= 0)
      th = o;
   return th;
}

/* do (explosion-like) damage to nearby monsters */
#define EXPLODE_RADIUS (FIXED_ONE*8)
static void
do_damage(const LevData *ld, int th, fixed arc)
{
   int targ, myowner = unowned_owner(ld, th);
   const ThingDyn *td = ldthingd(ld) + th;
   ThingDyn *ttd = ldthingd(ld);
   arc /= 2;
   for (targ = 0; targ < ldnthings(ld); targ++, ttd++) {
      fixed angle, dist, dmul;
      int dmg;
      if (ttd->proto == NULL || ttd->proto->hits == -1 || targ == th)
	 continue;
      /* check immunetosuch */
      if((ttd->proto->flags&PT_IMMUNETOSUCH) &&
	 td->proto->id==ttd->proto->spawn1)
	 continue;
      /* check nohurto */
      if ((td->proto->flags & PT_NOHURTO) 
	  && (targ == td->owner || targ== myowner))
	 continue;
      angle = fix_vec2angle(ttd->x - td->x, ttd->y - td->y);
      NORMALIZE_ANGLE(angle);
      /* this thing completely behind effect, anyway? */
      if (angle > arc && angle < (FIXED_2PI - arc))
	 continue;
      /* no, check distance (lots of guesstimation going on here) */
      dist = fix_pyth3d(ttd->x - td->x, ttd->y - td->y, ttd->z - td->z);
      dist -= (ttd->proto->radius + ttd->proto->height) / 2;
      /* damage calculation */
      if (dist > EXPLODE_RADIUS)
	 continue;
      dmul = fixdiv(EXPLODE_RADIUS - dist, EXPLODE_RADIUS);
      dmul = fixmul(dmul, dmul);
      dmg = FIXED_TO_INT(fixmul(INT_TO_FIXED(td->proto->damage), dmul));
      if (dmg <= 0)
	 continue;
      /* OK, we've hurt them: what happens? */
      /* 1) they get pissed off with us (unless we are they) */
      if (myowner != targ)
	 ttd->target = myowner;
      /* 2) knockback */
      thingd_apply_polar(ttd, INT_TO_FIXED(dmg) / 16, angle);
      if (td->z <= ttd->z)
	 thingd_apply_up(ttd, INT_TO_FIXED(dmg) / 16);
      else if (td->z < ttd->z + ttd->proto->height)
	 thingd_apply_up(ttd, INT_TO_FIXED(dmg) / 32);
      /* 3) they take damage */
      thing_take_damage(ld, targ, dmg);
   }
}

#define MELEE_DISTANCE (24<<12)

/* do damage just to target */
static void
do_melee_damage(const LevData *ld, int th, fixed knockback)
{
   ThingDyn *td = ldthingd(ld) + th;
   ThingDyn *ttd = ldthingd(ld) + td->target;
   fixed dist;
   int myowner;
   if (td->target < 0 || ttd->proto == NULL)
      return;
   /* check immunetosuch */
   if((ttd->proto->flags&PT_IMMUNETOSUCH) && td->proto->id==ttd->proto->spawn1)
      return;
   /* check distance */
   dist = fix_pyth3d(td->x - ttd->x, td->y - ttd->y, td->z - ttd->z);
   if ((dist > td->proto->radius + ttd->proto->radius + MELEE_DISTANCE) &&
       !(td->proto->flags & PT_BULLET)) {
#ifdef THINGM_DEBUG
      logprintf(LOG_DEBUG, 'O', _("melee: %d's attack on %d failed"),
		th, td->target);
#endif
      return;
   }
   /* piss them off */
   myowner = unowned_owner(ld, th);
   if (myowner != td->target)
      ttd->target = myowner;
   /* knock them back */
   thingd_apply_polar(ttd, td->proto->damage * knockback, td->angle);
   /* if this does lots of damage, lift them up too */
   if (knockback && td->proto->damage > 10)
      thingd_apply_up(ttd, td->proto->damage * knockback / 4);
   /* actually do the damage */
   thing_take_damage(ld, td->target, td->proto->damage);
   /* perhaps turn my owner (as with DOOM chainsaw) */
   if (td->proto->flags & PT_TURNWHENHITTING) {
      /* FIXME: Turn this bogothing and all others as well... */
      ldthingd(ld)[myowner].angle
	  = fix_vec2angle(ttd->x - ldthingd(ld)[myowner].x,
			  ttd->y - ldthingd(ld)[myowner].y);
   }
}

void
thing_autoaim(const LevData *ld, int th, fixed arc, fixed *angle,
	      fixed *elev)
{
   ThingDyn *td = ldthingd(ld) + th;
   /* *angle=*elev=FIXED_ZERO; */
   /* figure out angle to target */
   if (td->target >= 0) {
      const ThingDyn *ttd = ldthingd(ld) + td->target;
      fixed t = fix_vec2angle(ttd->x - td->x, ttd->y - td->y) - td->angle;
      fixed d = fix_pythagoras(ttd->x - td->x, ttd->y - td->y);
      fixed e = fix_vec2angle(d, (ttd->z + SHOOT_HEIGHT(ttd)) -
			      (td->z + SHOOT_HEIGHT(td)));
      NORMALIZE_ANGLE(t);
      NORMALIZE_ANGLE(e);
      if (t < arc / 2) {
	 *angle = t;
	 *elev = e;
      } else if (t > (FIXED_2PI - arc / 2)) {
	 *angle = t;
	 *elev = e;
      } else if (t < FIXED_PI) {
	 *angle = arc / 2;
	 *elev = e;
      } else {
	 *angle = FIXED_2PI - arc / 2;
	 *elev = e;
      }
   }
}

/*
   TPH_SHOOT encompasses many activities: it is supposed to do
   whatever shooting type activity is appropriate to the thing, or if none
   to just spawn a spawn1 object
 */
static void
do_shoot(LevData *ld, int th)
{
   ThingDyn *td = ldthingd(ld) + th;
   fixed angle = FIXED_ZERO, elev = td->elev;
   /* figure out angle to target */
   thing_autoaim(ld, th, ldthingd(ld)[th].proto->aim_arc, &angle, &elev);
   /* do it */
   /* FIXME: should check PT_TURNWHENHITTING here?
    * no such weapon yet, though */
   if (td->proto->flags & PT_SHOOTER)
      thing_shoot(ld, th, td->proto->spawn1, angle, elev,
		  td->proto->shootarc_h, td->proto->shootarc_v,
		  td->proto->shootnum,
		  td->proto->flags & PT_PARA_SHOOT);
   else
      thing_hurl(ld, th, td->proto->spawn1, angle, elev,
		 td->proto->shootarc_h, td->proto->shootnum,
		 td->proto->flags & PT_PARA_SHOOT);
}

void
thing_become(LevData *ld, int th, int id)
{
   ThingDyn *td = ldthingd(ld) + th;
#ifdef THINGM_DEBUG
   logprintf(LOG_DEBUG, 'O', _("Thing %d becomes type %d; old type was %d"),
	     th, id, td->proto ? (int) (td->proto->id) : -1);
#endif
   td->proto = find_protothing(id);
   if (td->proto == NULL)
      return;
   td->phase_tbl = find_first_thingphase(td->proto->phase_id);
   td->hits = td->proto->hits;
   td->phase = td->proto->signals[TS_INIT];
}

static int
rnd(int n)
{
   return (int) (((float) n) * rand() / (RAND_MAX + 1.0));
}

void
thing_enter_phase(LevData *ld, int th, int ph)
{
   ThingDyn *td = ldthingd(ld) + th;
   const ThingPhase *nu = td->phase_tbl + ph;
   const ThingPhase *old = td->phase_tbl + td->phase;

   /* check we've been passed a valid thing */
   if (td->phase_tbl == NULL || td->proto == NULL)
      return;

   /* check for RSKIP */
   while ((nu->flags & TPH_RSKIP) && !(rand() & 1) ) {
      ph++;
      nu = td->phase_tbl + ph;
   };

   /* do post-phase effects */
   if (old->flags & TPH_DESTROY) {
      /* remove one from my owner's spawncount */
      if(td->owner>=0) {
	 if(--(ldthingd(ld)[td->owner].spawncount)<0)
	    ldthingd(ld)[td->owner].spawncount=0;
      };
      td->proto = NULL;
      return;
   }
   if (old->flags & TPH_BECOME) {
      thing_become(ld, th, td->proto->become1);
      if (td->proto == NULL)
	 return;
      ph = td->phase;
      nu = td->phase_tbl + ph;
   }
   if (old->flags & TPH_BECOME2) {
      thing_become(ld, th, td->proto->become2);
      if (td->proto == NULL)
	 return;
      ph = td->phase;
      nu = td->phase_tbl + ph;
   }
   
   /* update phase */
   td->phase = ph;
   td->phase_wait = nu->wait + rnd(nu->rwait);
   
   /* play sound */
   if (nu->sound >= 0)
      play_dsound(nu->sound + td->proto->sound,
		  td->x, td->y, td->proto->radius);

   /* do pre-phase effects */
   if (nu->flags & TPH_CHARGE)
      thingd_apply_forward(td, 
			  3 * fixmul(td->proto->realmass, td->proto->speed));
   if (nu->flags & TPH_EXPLODE)
      do_damage(ld, th, FIXED_2PI);
   if (nu->flags & TPH_MELEE)
      do_melee_damage(ld, th,
	     td->proto->flags & PT_BEASTIE ? FIXED_ZERO : FIXED_ONE / 2);
   /*if(nu->flags&TPH_BDMG) do_melee_damage(ld,th,FIXED_ONE/2); */
   if (nu->flags & TPH_HEATSEEK)
      do_heatseek(ld, th);
   if (nu->flags & TPH_STRATEGY)
      do_strategy(ld, th);
   /*if(nu->flags&TPH_NOISY) thing_wake_others(ld,th); */
   if (nu->flags & TPH_SHOOT)
      do_shoot(ld, th);
   if (nu->flags & TPH_SPAWN2 || ((nu->flags & TPH_RSPAWN2) && (rand() & 1)))
      spawn_or_hurl(ld, th, td->proto->spawn2);
}

int
thing_sig_ok(const LevData *ld, int th, ThingSignal ts)
{
   ThingDyn *td = ldthingd(ld) + th;
   /* if
      1) we respond to this type of signal, and
      2) we aren't already pending a higher priority signal, and
      3) signals aren't locked out for this phase,
    */
   if (td->proto->signals[ts] >= 0 &&
       td->pending_signal < ts &&
       !(td->phase_tbl[td->phase].flags & TPH_NOSIGS))
      return 1;
   else
      return 0;
}

int
thing_send_sig(const LevData *ld, int th, ThingSignal ts)
{
   ThingDyn *td = ldthingd(ld) + th;
   if (thing_sig_ok(ld, th, ts))
      td->pending_signal = ts;
   else
      return -1;
   return 0;
}

void
thingd_apply_polar(ThingDyn *td, fixed f, fixed a)
{
   if (td->proto->flags & PT_INF_MASS)
      return;
   td->dx += fixmul(fixcos(a), fixdiv(f, td->proto->realmass));
   td->dy += fixmul(fixsin(a), fixdiv(f, td->proto->realmass));
}

void
thingd_apply_torque(ThingDyn *td, fixed t)
{
   if (td->proto->flags & PT_INF_MASS)
      return;
   td->dangle += fixdiv(t, td->proto->realmass);
}

void
thingd_apply_up(ThingDyn *td, fixed f)
{
   if (td->proto->flags & PT_INF_MASS)
      return;
   td->dz += fixdiv(f, td->proto->realmass);
}

/* the angle between each rotation of a sprite in the wadfile */
#define THETA (FIXED_PI/4)
void
thing_rotate_image(const LevData *ld, int th, fixed angle)
{
   ThingDyn *td = ldthingd(ld) + th;
   char rot;
   angle -= td->angle + FIXED_PI;
   angle += THETA / 2;
   NORMALIZE_ANGLE(angle);
   rot = '1' + angle / THETA;
   if (rot < '1' || rot > '8')
      rot = '1';
   td->image = find_phase_sprite(td->proto, td->phase, rot);
   if (td->image &&
       td->image->name[7] == rot &&
       td->image->name[6] == td->phase_tbl[td->phase].spr_phase)
      td->mirror_image = 1;
   else
      td->mirror_image = 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
