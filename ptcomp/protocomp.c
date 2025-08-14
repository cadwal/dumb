/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/protocomp.c: ProtoThing compiler.
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
#include "libdumb/prothingstruct.h"
#include "libdumb/protoinwad.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "gettcomp.h"
#include "phasecomp.h"
#include "protocomp.h"
#include "soundcomp.h"

typedef struct {
   ProtoThing pt;
   char *name;
} ProtoThingRec;

static int nprotos = 0, maxprotos = 0;
static ProtoThingRec *protos;

static int top_assigned_proto_id = 16385;

static int proto_parse_phases(ProtoThingRec *p, ThingPhaseRec **tpr,
			      const char *token);
static int proto_parse_appearance(ProtoThingRec *p, const char *token);
static int proto_parse_pain(ProtoThingRec *p, const char *token);
static int proto_parse_attack(ProtoThingRec *p, const char *token);
static int proto_parse_ai(ProtoThingRec *p, const char *token);
static int proto_parse_powerup(ProtoThingRec *p, const char *token);
static int proto_parse_physics(ProtoThingRec *p, const char *token);
static int proto_parse_misc(ProtoThingRec *p, const char *token);

static void chkuniqueid(int id);
static void set_phid(ProtoThingRec *p, ThingPhaseRec *tpr);
static int ptcmp(const void *p1, const void *p2);


void
init_protocomp(void)
{
   maxprotos = ALLOC_BLK;
   protos = (ProtoThingRec *) safe_malloc(maxprotos * sizeof(ProtoThingRec));
}

void
protocomp(void)
{
   const char *token;
   ProtoThingRec *p;
   ThingPhaseRec *tpr;
   /* make new proto */
   if (nprotos >= maxprotos - 1) {
      maxprotos += ALLOC_BLK;
      protos = (ProtoThingRec *)
	 safe_realloc(protos, maxprotos * sizeof(ProtoThingRec));
   }
   p = protos + (nprotos++);
   memset(p, 0, sizeof(ProtoThingRec));
   p->pt.hits = -1;
   p->pt.shootnum = 1;
   p->pt.flags |= PT_INF_MASS;
   p->pt.speed = FIXED_ONE;
   p->pt.see_arc = (FIXED_PI - FIXED_PI / 4);
   p->pt.aim_arc = (FIXED_PI / 4);
   p->pt.spawnmax = 5;
   p->pt.become1 = -1;
   p->pt.become2 = -1;
   /* Proto -!- NAME [ID:INTEGER] */
   p->name = safe_strdup(parm_name(_("Name expected after Proto")));
   /* FIXME: check uniqueness */
   tpr = find_ph_tbl(p->name);
   if (tpr)
      set_phid(p, tpr);
   token = next_token();
   if (token != NULL && isdigit(token[0]))
      chkuniqueid(p->pt.id = atoi(token));
   else {
      unget_token();
      p->pt.id = top_assigned_proto_id++;
   }
   /* now info for this proto */
   do {
      token = next_token();
      if (token == NULL)
	 return;
   } while (class_of_token(token) == TOKENCL_NEWLINE
	    || proto_parse_phases(p, &tpr, token)
	    || proto_parse_appearance(p, token)
	    || proto_parse_pain(p, token)
	    || proto_parse_attack(p, token)
	    || proto_parse_ai(p, token)
	    || proto_parse_powerup(p, token)
	    || proto_parse_physics(p, token)
	    || proto_parse_misc(p, token));
   unget_token();
}

void
wrprotos(WADWR *wout)
{
   int i;
   printf(_("%5d protos: sorting"), nprotos);
   fflush(stdout);
   qsort(protos, nprotos, sizeof(ProtoThingRec), ptcmp);
   printf(_(", writing"));
   fflush(stdout);
   wadwr_lump(wout, "PROTOS");
   for (i = 0; i < nprotos; i++) {
      void *block;
      size_t blocklen;
      if (protos[i].pt.phase_id < 1)
	 printf(_("warning: proto %s (%d) has no phasetable\n"),
		protos[i].name, (int) (protos[i].pt.id));
      block = encode_protoinwad(&protos[i].pt, &blocklen);
      wadwr_write(wout, block, blocklen);
      safe_free(block);
   }
   printf("\n");
}

int
parm_proto(void)
{
   int i;
   const char *s = parm_name(_("Proto name parameter expected"));
   if (isdigit(*s))
      return atoi(s);
   for (i = 0; i < nprotos; i++)
      if (!strcasecmp(protos[i].name, s))
	 return protos[i].pt.id;
   err(_("Proto name `%s' unrecognised"), s);
   return -1;
}


/* Each of these returns true if it understands TOKEN.  */

static int
proto_parse_phases(ProtoThingRec *p, ThingPhaseRec **tpr, const char *token)
{
   if (!strcasecmp(token, "Phases"))
      set_phid(p, *tpr = parm_ph_tbl());
   else if (!strcasecmp(token, "SigDetect"))
      p->pt.signals[TS_DETECT] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigFight"))
      p->pt.signals[TS_FIGHT] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigShoot"))
      p->pt.signals[TS_SHOOT] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigSpecial"))
      p->pt.signals[TS_SPECIAL] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigOuch"))
      p->pt.signals[TS_OUCH] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigDie"))
      p->pt.signals[TS_DIE] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigExplode"))
      p->pt.signals[TS_EXPLODE] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigReanimate"))
      p->pt.signals[TS_ANIMATE] = parm_phase(*tpr);
   else if (!strcasecmp(token, "SigInit"))
      p->pt.signals[TS_INIT] = parm_phase(*tpr);
   else
      return 0;
   return 1;
}

static int
proto_parse_appearance(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "Sprite"))
      parm_str(p->pt.sprite, 6);
   else if (!strcasecmp(token, "ZCenter"))
      p->pt.flags |= PT_ZCENTER;
   else if (!strcasecmp(token, "ZPegged"))
      p->pt.flags |= PT_ZPEG;
   else if (!strcasecmp(token, "PInvis"))
      p->pt.flags |= PT_PINVIS;
   else
      return 0;
   return 1;
}

static int
proto_parse_pain(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "Hits"))
      p->pt.hits = parm_int();
   else if (!strcasecmp(token, "Splat"))
      p->pt.bloodtype = parm_proto();
   else if (!strcasecmp(token, "Becomes"))
      p->pt.become1 = parm_proto();
   else if (!strcasecmp(token, "Become2"))
      p->pt.become2 = parm_proto();
   else if (!strcasecmp(token, "Boss"))
      p->pt.flags |= PT_BOSS;
   else
      return 0;
   return 1;
}

static int
proto_parse_attack(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "AimArc"))
      p->pt.aim_arc = parm_arc();
   else if (!strcasecmp(token, "NoHurtOwner"))
      p->pt.flags |= PT_NOHURTO;
   /* Melee */
   else if (!strcasecmp(token, "Damage"))
      p->pt.damage = parm_int();
   else if (!strcasecmp(token, "TurnWhenHitting")) {
      warn(_("`TurnWhenHitting' is now called `TurnOwner'"));
      p->pt.flags |= PT_TURN_OWNER;
   } else if (!strcasecmp(token, "TurnOwner"))
      p->pt.flags |= PT_TURN_OWNER;
   else if (!strcasecmp(token, "BulletAttack"))
      p->pt.flags |= PT_BULLET;
   else if (!strcasecmp(token, "BulletKludge")) {
      warn(_("`BulletKludge' is now called `BecomesBlood'"));
      p->pt.flags |= PT_BECOMES_BLOOD;
   } else if (!strcasecmp(token, "BecomesBlood"))
      p->pt.flags |= PT_BECOMES_BLOOD;
   /* Missiles */
   else if (!strcasecmp(token, "Shooter"))
      p->pt.flags |= PT_SHOOTER;
   else if (!strcasecmp(token, "Shoots"))
      p->pt.spawn1 = parm_proto();
   else if (!strcasecmp(token, "Spawns"))
      p->pt.spawn1 = parm_proto();
   else if (!strcasecmp(token, "Spawn2"))
      p->pt.spawn2 = parm_proto();
   else if (!strcasecmp(token, "ImmuneToSuch"))
      p->pt.flags |= PT_IMMUNETOSUCH;
   else if (!strcasecmp(token, "ShootMany")) {
      p->pt.shootnum = parm_uint();
      p->pt.shootarc_h = parm_arc();
      p->pt.shootarc_v = parm_arc_opt(p->pt.shootarc_h / 8);
   } else if (!strcasecmp(token, "ShootPara")) {
      p->pt.flags |= PT_PARA_SHOOT;
      p->pt.shootnum = parm_uint();
      p->pt.shootarc_h = parm_uint() << 12;
      p->pt.shootarc_v = 0;	/* just in case */
   } else if (!strcasecmp(token, "SpawnMax"))
      p->pt.spawnmax = parm_uint();
   else
      return 0;
   return 1;
}

static int
proto_parse_ai(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "SkirtCliffs"))
      p->pt.flags |= PT_SKIRT_CLIFFS;
   else if (!strcasecmp(token, "FastShoot")) {
      warn(_("`FastShoot' is obsolete; `Nasty' is the closest we have"));
      p->pt.flags |= PT_NASTY;
   } else if (!strcasecmp(token, "Nasty"))
      p->pt.flags |= PT_NASTY;
   else
      return 0;
   return 1;
}

static int
proto_parse_powerup(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "Gets")) {
      Gets *getsp;
      p->pt.gets = (Gets *)
	 safe_realloc(p->pt.gets, ++(p->pt.ngets) * sizeof(Gets));
      getsp = &p->pt.gets[p->pt.ngets-1];
      getsp->gtid = parm_gett();
      getsp->change = parm_int();
      if (parm_keyword_opt("Maximum"))
	 getsp->maximum = parm_uint();
      else
	 getsp->maximum = 0;	/* means use default from Gettable */
   } else if (!strcasecmp(token, "PickupSound"))
      p->pt.pickup_sound = parm_sound();
   else if (!strcasecmp(token, "FirstPickupMessage"))
      p->pt.firstpickupmsg = parm_strdup();
   else if (!strcasecmp(token, "PickupMessage"))
      p->pt.pickupmsg = parm_strdup();
   else if (!strcasecmp(token, "IgnoreMessage"))
      p->pt.ignoremsg = parm_strdup();
   else
      return 0;
   return 1;
}

static int
proto_parse_physics(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "Speed"))
      p->pt.speed = parm_speed();
   else if (!strcasecmp(token, "Mass")) {
      int i = parm_int();
      p->pt.flags &= ~PT_INF_MASS;
      if (i) {
	 p->pt.realmass = (FIXED_ONE * i) / 100;
	 p->pt.friction = (61 * FIXED_ONE) / 64;
      } else {
	 p->pt.realmass = p->pt.friction = FIXED_ONE;
	 p->pt.flags |= PT_CAN_FLY;
      }
   } else if (!strcasecmp(token, "Size")) {
      p->pt.radius = parm_uint() << 11;
      p->pt.height = parm_uint() << 12;
   } else if (!strcasecmp(token, "Hanging"))
      p->pt.flags |= PT_HANGING;
   else if (!strcasecmp(token, "StuckDown"))
      p->pt.flags |= PT_STUCKDOWN;
   else if (!strcasecmp(token, "FloatsUp"))
      p->pt.flags |= PT_FLOATSUP;
   else if (!strcasecmp(token, "YMoveOnly"))
      p->pt.flags |= PT_YMOVE_ONLY;
   else if (!strcasecmp(token, "Blocking"))
      p->pt.flags |= PT_BLOCKING;
   else if (!strcasecmp(token, "Phantom"))
      p->pt.flags |= PT_PHANTOM;
   else if (!strcasecmp(token, "BulletProof"))
      p->pt.flags |= PT_BULLETPROOF;
   else if (!strcasecmp(token, "Flying")) {
      warn(_("`Flying' is obsolete; use `CanFly'"));
      p->pt.flags |= PT_CAN_FLY;
   } else if (!strcasecmp(token, "CanFly"))
      p->pt.flags |= PT_CAN_FLY;
   else if (!strcasecmp(token, "Bouncy"))
      p->pt.flags |= PT_BOUNCY;
   else
      return 0;
   return 1;
}

static int
proto_parse_misc(ProtoThingRec *p, const char *token)
{
   if (!strcasecmp(token, "Bogus"))
      p->pt.flags |= PT_BOGUS | PT_PHANTOM;
   else if (!strcasecmp(token, "Target"))
      p->pt.flags |= PT_TARGET;
   else if (!strcasecmp(token, "Beastie"))
      p->pt.flags |= PT_BEASTIE | PT_TARGET;
   else if (!strcasecmp(token, "Player"))
      p->pt.flags |= PT_PLAYER;
   else if (!strcasecmp(token, "Mine")) 
      p->pt.flags |= PT_MINE;
   else if (!strcasecmp(token, "Explosive"))
      p->pt.flags |= PT_EXPLOSIVE | PT_MINE;
   else if (!strcasecmp(token, "WExplosive"))
      p->pt.flags |= PT_EXPLOSIVE;
   else if (!strcasecmp(token, "SpawnSpot"))
      p->pt.flags |= PT_SPAWNSPOT;
   else if (!strcasecmp(token, "SeeArc"))
      p->pt.see_arc = parm_arc();
   else
      return 0;
   return 1;
}


static void
chkuniqueid(int id)
{
   int i;
   for (i = nprotos - 2; i >= 0; i--)
      if (protos[i].pt.id == id)
	 err(_("ID %d not unique (clashes with %s)"),
	     id, protos[i].name);
}

static void
set_phid(ProtoThingRec *p, ThingPhaseRec *tpr)
{
   int i;
   p->pt.phase_id = tpr->tp->id;
   /* can't use memcpy: the types differ */
   for (i = 0; i < NUM_THINGSIGS; i++)
      p->pt.signals[i] = tpr->signals[i];
}

static int
ptcmp(const void *p1, const void *p2)
{
   if (((const ProtoThingRec *) p1)->pt.id > ((const ProtoThingRec *) p2)->pt.id)
      return 1;
   if (((const ProtoThingRec *) p1)->pt.id < ((const ProtoThingRec *) p2)->pt.id)
      return -1;
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
