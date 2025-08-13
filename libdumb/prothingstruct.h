#ifndef PROTHINGSTRUCT_H
#define PROTHINGSTRUCT_H

#include "libdumbutil/endiantypes.h"
#include "libdumbutil/fixed.h"

typedef struct  {
   LE_int16 id; /* an identifier matched up with values in PT */
   LE_int16 next; /* next state to go to, if nothing else happens */
   LE_int16 wait; /* mapticks to wait before next state */
   LE_int16 sound; /* <0 if none, otherwise offset from value in PT */
   LE_flags32 flags,_spare1;
   LE_int16 rwait;
   char sprite[5]; /* if null, use the sprite name in PT */
   char spr_phase;
} ThingPhase;

#define TPH_NONE 0           /* having this makes table more readable */
#define TPH_GLOW 0x0001      /* sprite won't be affected by darkness */
#define TPH_DESTROY 0x0002   /* destroy object after this phase */
#define TPH_NOSIGS 0x0004    /* no signals are allowed during phase */
#define TPH_STRATEGY 0x0008  /* run monster strategy routine now */
#define TPH_HEATSEEK 0x0010  /* run "heatseeker" strategy now */
#define TPH_SHOOT 0x0020     /* shoot or hurl a missile */
#define TPH_EXPLODE 0x0040   /* do explosion damage */
#define TPH_MELEE 0x0080     /* do melee (or bullet) damage */
#define TPH_SPAWN2 0x0100    /* hurl or spawn some object */
#define TPH_RSPAWN2 0x0200   /* 1/2 chance, hurl or spawn some object */
#define TPH_BECOME 0x0400    /* become new type of object (usually a corpse) */
#define TPH_BECOME2 0x0800   /* become other type (often an exploded corpse) */
/*#define TPH_NOISY 0x1000      wake up monsters that can see me now */
#define TPH_IDLE 0x2000      /* idle phase (allow TS_DETECTs) */
#define TPH_BFGEFFECT 0x4000 /* make like the big friendly gun */

typedef enum  {
   TS_INIT,
   TS_DETECT,TS_FIGHT,TS_SHOOT,TS_SPECIAL,
   TS_OUCH,TS_DIE,TS_EXPLODE,
   TS_ANIMATE,
   TS_EXTRA,
   NUM_THINGSIGS,TS_NOSIG=-1
} ThingSignal;

typedef struct {
   LE_int16 id;      /* id referred to by THINGS lump */
   LE_int16 _spare1;    
   LE_int16 hits;
   LE_int16 damage;  /* how much damage to do in melee, or when exploding */
   LE_int32 /* fixed */ realmass,friction; 
   LE_int32 /* fixed */ radius,height;  
   LE_int32 sound;
   LE_flags32 flags;
   LE_int32 /* fixed */ speed;
   LE_int32 /* fixed */ shootarc,see_arc,aim_arc; 
   LE_int16 shootnum;
   LE_int16 artitype;
   LE_int16 artinum;
   LE_int16 spawn1,spawn2; /* prothing id to be spawned by TPH_SPAWN */
   LE_int16 become1,become2;
   LE_int16 bloodtype,_spare2;
   LE_int16 phase_id;
   LE_int16 signals[NUM_THINGSIGS];
   char sprite[6];
} ProtoThing;

#define PT_EXPLOSIVE 0x001     /* explode on collision */
#define PT_CAN_CLIMB 0x002     /* can climb stairs */
#define PT_BEASTIE 0x002       /* a monster or player */
#define PT_ZCENTER 0x004       /* object's z pos'n is at center of sprite */
#define PT_PHANTOM 0x008       /* never collides with anything */
#define PT_SKIRT_CLIFFS 0x010  /* stop me from falling off */
#define PT_CAN_FLY 0x020       /* (mon) has controlled flight */
#define PT_USE_EMNITY 0x040    /* (mon) hates other things besides player */
#define PT_PINVIS 0x080        /* object is partially invisible */
#define PT_BULLET_KLUDGE 0x100 /* convert bullet pocks to bloodspurts */
#define PT_SHOOTER 0x200       /* shoot, don't hurl on SPAWN1 */
#define PT_FAST_SHOOTER 0x400  /* "chaingun" effect: always shoot when I can */
/*#define PT_NOISY 0x800          wake up monsters when I come across them */ 
#define PT_HANGING 0x1000      /* hanging from ceiling */
#define PT_INF_MASS 0x2000     /* thing is infinitely massive */
#define PT_PLAYER 0x4000       /* thing is allowed to pickup objects, etc */
#define PT_TARGET 0x8000       /* autotarget on this thing */
#define PT_SPAWNSPOT 0x20000   /* good place to spawn a monster */
#define PT_BULLETPROOF 0x40000 /* bullets will just pass through */
#define PT_PARA_SHOOT 0x80000  /* "parallel shoot", shootarc is seperation */
#define PT_YMOVE_ONLY 0x100000 /* thing can only move vertically */
#define PT_FLOATSUP 0x200000   /* thing floats up */
#define PT_ZPEG 0x400000       /* object's z pos'n is at top of sprite */
#define PT_BOUNCY 0x800000     /* object is kinda elastic */
#define PT_BLOCKING 0x1000000  /* object can get in the way */
#define PT_BOGUS 0x2000000     /* object is bogus */
#define PT_NOHURTO 0x4000000   /* don't damage my owner */
#define PT_TURNWHENHITTING 0x8000000 /* weapon bogot turns user when hitting */
#define PT_BULLET 0x10000000   /* do melee damage like bullet */
                               /* ie. skip range check in melee */



#endif
