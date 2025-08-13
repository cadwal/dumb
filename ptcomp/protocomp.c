/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/protocomp.c: Proto compiler.
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
   const char *s;
   ProtoThingRec *p;
   ThingPhaseRec *tpr;
   /* make new proto */
   if (nprotos >= maxprotos - 1) {
      maxprotos += ALLOC_BLK;
      protos = (ProtoThingRec *) safe_realloc(protos,
				      sizeof(ProtoThingRec) * maxprotos);
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
   tpr = find_ph_tbl(p->name);
   if (tpr)
      set_phid(p, tpr);
   s = next_token();
   if (s != NULL && isdigit(s[0]))
      chkuniqueid(p->pt.id = atoi(s));
   else {
      unget_token();
      p->pt.id = top_assigned_proto_id++;
   }
   /* now info for this proto */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "Phases"))
	 set_phid(p, tpr = parm_ph_tbl());
      else if (!strcasecmp(s, "SigDetect"))
	 p->pt.signals[TS_DETECT] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigFight"))
	 p->pt.signals[TS_FIGHT] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigShoot"))
	 p->pt.signals[TS_SHOOT] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigSpecial"))
	 p->pt.signals[TS_SPECIAL] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigOuch"))
	 p->pt.signals[TS_OUCH] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigDie"))
	 p->pt.signals[TS_DIE] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigExplode"))
	 p->pt.signals[TS_EXPLODE] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigReanimate"))
	 p->pt.signals[TS_ANIMATE] = parm_phase(tpr);
      else if (!strcasecmp(s, "SigInit"))
	 p->pt.signals[TS_INIT] = parm_phase(tpr);
      else if (!strcasecmp(s, "Splat"))
	 p->pt.bloodtype = parm_proto();
      else if (!strcasecmp(s, "Shoots"))
	 p->pt.spawn1 = parm_proto();
      else if (!strcasecmp(s, "Spawns"))
	 p->pt.spawn1 = parm_proto();
      else if (!strcasecmp(s, "Spawn2"))
	 p->pt.spawn2 = parm_proto();
      else if (!strcasecmp(s, "SpawnMax"))
	 p->pt.spawnmax = parm_num();
      else if (!strcasecmp(s, "Becomes"))
	 p->pt.become1 = parm_proto();
      else if (!strcasecmp(s, "Become2"))
	 p->pt.become2 = parm_proto();
      else if (!strcasecmp(s, "Sprite"))
	 parm_str(p->pt.sprite, 6);
      else if (!strcasecmp(s, "Damage"))
	 p->pt.damage = parm_num();
      else if (!strcasecmp(s, "Speed"))
	 p->pt.speed = parm_num() << 12;
      else if (!strcasecmp(s, "Mass")) {
	 int i = parm_num();
	 p->pt.flags &= ~PT_INF_MASS;
	 if (i) {
	    p->pt.realmass = (FIXED_ONE * i) / 100;
	    p->pt.friction = (61 * FIXED_ONE) / 64;
	 } else {
	    p->pt.realmass = p->pt.friction = FIXED_ONE;
	    p->pt.flags |= PT_CAN_FLY;
	 }
      } else if (!strcasecmp(s, "Hits"))
	 p->pt.hits = parm_num();
      else if (!strcasecmp(s, "Shooter"))
	 p->pt.flags |= PT_SHOOTER;
      else if (!strcasecmp(s, "Mine")) 
	 p->pt.flags |= PT_MINE;
      else if (!strcasecmp(s, "Beastie"))
	 p->pt.flags |= PT_BEASTIE | PT_TARGET;
      else if (!strcasecmp(s, "Explosive"))
	 p->pt.flags |= PT_EXPLOSIVE | PT_MINE;
      else if (!strcasecmp(s, "WExplosive"))
	 p->pt.flags |= PT_EXPLOSIVE;
      else if (!strcasecmp(s, "ZCenter"))
	 p->pt.flags |= PT_ZCENTER;
      else if (!strcasecmp(s, "Phantom"))
	 p->pt.flags |= PT_PHANTOM;
      else if (!strcasecmp(s, "SkirtCliffs"))
	 p->pt.flags |= PT_SKIRT_CLIFFS;
      else if (!strcasecmp(s, "CanFly"))
	 p->pt.flags |= PT_CAN_FLY;
      else if (!strcasecmp(s, "Flying"))
	 p->pt.flags |= PT_CAN_FLY;
      else if (!strcasecmp(s, "PInvis"))
	 p->pt.flags |= PT_PINVIS;
      else if (!strcasecmp(s, "BulletKludge"))
	 p->pt.flags |= PT_BULLET_KLUDGE;
      else if (!strcasecmp(s, "FastShoot")) {
	 warn(_("FastShoot is obsolete; use Nasty"));
	 p->pt.flags |= PT_NASTY;
      } else if (!strcasecmp(s, "Nasty"))
	 p->pt.flags |= PT_NASTY;
      else if (!strcasecmp(s, "Hanging"))
	 p->pt.flags |= PT_HANGING;
      else if (!strcasecmp(s, "StuckDown"))
	 p->pt.flags |= PT_STUCKDOWN;
      else if (!strcasecmp(s, "YMoveOnly"))
	 p->pt.flags |= PT_YMOVE_ONLY;
      else if (!strcasecmp(s, "Player"))
	 p->pt.flags |= PT_PLAYER;
      else if (!strcasecmp(s, "Target"))
	 p->pt.flags |= PT_TARGET;
      else if (!strcasecmp(s, "BulletProof"))
	 p->pt.flags |= PT_BULLETPROOF;
      else if (!strcasecmp(s, "FloatsUp"))
	 p->pt.flags |= PT_FLOATSUP;
      else if (!strcasecmp(s, "ZPegged"))
	 p->pt.flags |= PT_ZPEG;
      else if (!strcasecmp(s, "Bouncy"))
	 p->pt.flags |= PT_BOUNCY;
      else if (!strcasecmp(s, "Blocking"))
	 p->pt.flags |= PT_BLOCKING;
      else if (!strcasecmp(s, "Bogus"))
	 p->pt.flags |= PT_BOGUS | PT_PHANTOM;
      else if (!strcasecmp(s, "SpawnSpot"))
	 p->pt.flags |= PT_SPAWNSPOT;
      else if (!strcasecmp(s, "NoHurtOwner"))
	 p->pt.flags |= PT_NOHURTO;
      else if (!strcasecmp(s, "ImmuneToSuch"))
	 p->pt.flags |= PT_IMMUNETOSUCH;
      else if (!strcasecmp(s, "TurnWhenHitting"))
	 p->pt.flags |= PT_TURNWHENHITTING;
      else if (!strcasecmp(s, "BulletAttack"))
	 p->pt.flags |= PT_BULLET;
      else if (!strcasecmp(s, "SeeArc"))
	 p->pt.see_arc = parm_arc();
      else if (!strcasecmp(s, "AimArc"))
	 p->pt.aim_arc = parm_arc();
      else if (!strcasecmp(s, "ShootMany")) {
	 p->pt.shootnum = parm_num();
	 p->pt.shootarc_h = parm_arc();
	 p->pt.shootarc_v = parm_arc_opt(p->pt.shootarc_h / 8);
      } else if (!strcasecmp(s, "ShootPara")) {
	 p->pt.flags |= PT_PARA_SHOOT;
	 p->pt.shootnum = parm_num();
	 p->pt.shootarc_h = parm_num() << 12;
	 p->pt.shootarc_v = 0;	/* just in case */
      } else if (!strcasecmp(s, "Size")) {
	 p->pt.radius = parm_num() << 11;
	 p->pt.height = parm_num() << 12;
      } else if (!strcasecmp(s, "Gets")) {
	 ProtoThing_Gets *artip;
	 p->pt.gets = (ProtoThing_Gets *)
	    realloc(p->pt.gets, ++(p->pt.ngets) * sizeof(ProtoThing_Gets));
	 artip = &p->pt.gets[p->pt.ngets-1];
	 artip->artitype = parm_gett();
	 artip->artinum = parm_num();
	 if (parm_keyword_opt("Maximum"))
	    artip->artimax = parm_num();
	 else
	    artip->artimax = 0;	/* means use default from Gettable */
      } else if (!strcasecmp(s, "PickupSound"))
	 p->pt.pickup_sound = parm_sound();
      else if (!strcasecmp(s, "FirstPickupMessage"))
	 p->pt.firstpickupmsg = parm_strdup();
      else if (!strcasecmp(s, "PickupMessage"))
	 p->pt.pickupmsg = parm_strdup();
      else if (!strcasecmp(s, "IgnoreMessage"))
	 p->pt.ignoremsg = parm_strdup();
      else
	 break;
   } /* while (1) */
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
