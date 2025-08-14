/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/prothingstruct.h: In-memory format of PhaseTables and ProtoThings.
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

#ifndef PROTHINGSTRUCT_H
#define PROTHINGSTRUCT_H

#include "libdumbutil/endiantypes.h"
#include "libdumbutil/fixed.h"
#include "libdumb/texture.h"
#include "libdumb/gettablestruct.h"

typedef struct {
   LE_int16 id;		/* an identifier matched up with values in PT */
   LE_int16 next;	/* next state to go to, if nothing else happens */
   LE_int16 wait;	/* mapticks to wait before next state */
   LE_int16 sound;	/* <0 if none, otherwise offset from value in PT */
   LE_flags32 flags, _spare1;
   LE_int16 rwait;
   char sprite[5];	/* if null, use the sprite name in PT */
   char spr_phase;
} ThingPhase;

#define TPH_NONE      0          /* having this makes table more readable */
#define TPH_GLOW      0x00000001 /* sprite won't be affected by darkness */
#define TPH_DESTROY   0x00000002 /* destroy object after this phase */
#define TPH_NOSIGS    0x00000004 /* no signals are allowed during phase */
#define TPH_STRATEGY  0x00000008 /* run monster strategy routine now */
#define TPH_HEATSEEK  0x00000010 /* run "heatseeker" strategy now */
#define TPH_SHOOT     0x00000020 /* shoot or hurl a missile */
#define TPH_EXPLODE   0x00000040 /* do explosion damage */
#define TPH_MELEE     0x00000080 /* do melee (or bullet) damage */
#define TPH_SPAWN2    0x00000100 /* hurl or spawn some object */
#define TPH_RSPAWN2   0x00000200 /* 1/2 chance, hurl or spawn some object */
#define TPH_BECOME    0x00000400 /* become new type of object (usually a corpse) */
#define TPH_BECOME2   0x00000800 /* become other type (often an exploded corpse) */
/*#define TPH_NOISY   0x00001000    wake up monsters that can see me now */
#define TPH_IDLE      0x00002000 /* idle phase (allow TS_DETECTs) */
#define TPH_BFGEFFECT 0x00004000 /* (NYI) make like the big friendly gun */
#define TPH_TINVIS    0x00008000 /* phase is totally invisible */
#define TPH_WHIRLY    0x00010000 /* (NYI) throw target up in the air (if in range) */
#define TPH_CHARGE    0x00020000 /* lostsoul-like charge */
#define TPH_RSKIP     0x00040000 /* 1/2 the time, skip this phase entirely */

typedef enum {
   TS_INIT,
   TS_DETECT, TS_FIGHT, TS_SHOOT, TS_SPECIAL,
   TS_OUCH, TS_DIE, TS_EXPLODE,
   TS_ANIMATE,
   TS_EXTRA,
   NUM_THINGSIGS, TS_NOSIG = -1
} ThingSignal;

/* The ProtoThings have different format in memory and in the WAD
   file.  DUMB converts them when it starts up.  The in-WAD format is
   defined in protoinwad.c -- nothing else should need it.  The
   in-memory structure is defined below.  */

typedef struct {
   int id;			/* id referred to by THINGS lump */
   int hits;
   int damage;   /* how much damage to do in melee, or when exploding */
   fixed realmass, friction;
   fixed radius, height;
   int sound;
   unsigned flags;
   fixed speed;
   fixed shootarc_h, shootarc_v, see_arc, aim_arc;
   int shootnum;
   int spawn1, spawn2;		/* prothing id to be spawned by TPH_SPAWN */
   int spawnmax;
   int become1, become2;
   int bloodtype;
   int phase_id;
   int signals[NUM_THINGSIGS];
   char sprite[6];
   Texture **sprites;		/* use via get_sprite_table() only! */
   /* The rest of the members say what happens when the player tries
      to pick this up.  */
   int pickup_sound;
   int ngets;			/* how many gettables this will give */
   Gets *gets;
   /* These point to untranslated dynamically allocated strings.  */
   char *firstpickupmsg;
   char *pickupmsg;
   char *ignoremsg;
} ProtoThing;

#define PT_EXPLOSIVE     0x00000001 /* explode on collision */
#define PT_BEASTIE       0x00000002 /* a monster or player */
#define PT_CAN_CLIMB     PT_BEASTIE /* can climb stairs */
#define PT_ZCENTER       0x00000004 /* object's z pos'n is at center of sprite */
#define PT_PHANTOM       0x00000008 /* never collides with anything */
#define PT_SKIRT_CLIFFS  0x00000010 /* stop me from falling off */
#define PT_CAN_FLY       0x00000020 /* (mon) has controlled flight */
#define PT_USE_EMNITY    0x00000040 /* (NYI) (mon) hates other things besides player */
#define PT_PINVIS        0x00000080 /* object is partially invisible */
#define PT_BECOMES_BLOOD 0x00000100 /* convert bullet pocks to bloodspurts */
#define PT_SHOOTER       0x00000200 /* shoot, don't hurl on SPAWN1 */
#define PT_NASTY         0x00000400 /* especially agressive monster */
#define PT_MINE          0x00000800 /* explode when I collide w/ monster */
#define PT_HANGING       0x00001000 /* hanging from ceiling */
#define PT_INF_MASS      0x00002000 /* thing is infinitely massive */
#define PT_PLAYER        0x00004000 /* thing is allowed to pickup objects, etc */
#define PT_TAKESECTDMG   PT_PLAYER  /* for now, only players hurt by goo */
#define PT_TARGET        0x00008000 /* autotarget on this thing */
#define PT_BOSS          0x00010000 /* (NYI) do 666 effect when these are all dead */
#define PT_SPAWNSPOT     0x00020000 /* (NYI) good place to spawn a monster */
#define PT_BULLETPROOF   0x00040000 /* bullets will just pass through */
#define PT_PARA_SHOOT    0x00080000 /* "parallel shoot", shootarc is separation */
#define PT_YMOVE_ONLY    0x00100000 /* (NYI) thing can only move vertically */
#define PT_FLOATSUP      0x00200000 /* thing floats up */
#define PT_ZPEG          0x00400000 /* object's z pos'n is at top of sprite */
#define PT_BOUNCY        0x00800000 /* object is kinda elastic */
#define PT_BLOCKING      0x01000000 /* object can get in the way */
#define PT_BOGUS         0x02000000 /* object is bogus */
#define PT_NOHURTO       0x04000000 /* don't damage my owner */
#define PT_TURN_OWNER    0x08000000 /* weapon bogot turns user when hitting */
#define PT_BULLET        0x10000000 /* do melee damage like bullet
                                     * ie. skip range check in melee */
#define PT_IMMUNETOSUCH  0x20000000 /* immune to own missile type (ala Doom) */
#define PT_STUCKDOWN     0x40000000 /* stuck to floor */

#endif

// Local Variables:
// c-basic-offset: 3
// End:
