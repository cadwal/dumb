/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/things.h: Things.  (Functions defined in many files.)
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

#ifndef THINGS_H
#define THINGS_H

#include "libdumb/view.h"
#include "levdyn.h"

#define SHOOT_HEIGHT(td) ((3*(td)->proto->height)/4)

void touch_player(LevData *ld, int pl);
int thing2player(const LevData *ld, int th);
int find_empty_thing(const LevData *ld);
int safe_find_empty_thing(const LevData *ld);
int new_thing(const LevData *ld, int proto_id, fixed x, fixed y, fixed z);
void thing_take_damage(const LevData *ld, int th, int dmg);
int thing_hurl(LevData *ld, int hurler, int missile_id,
	     fixed angle, fixed elevation, fixed arc, int num, int para);
void thing_shoot(LevData *ld, int shooter, int bullet_id,
	   fixed angle, fixed elevation, fixed horiz_arc, fixed vert_arc,
		 int num, int para);
int thing_can_shoot_at(const LevData *ld, int looker, int target);
void thing_wake_others(const LevData *ld, int th, int tickspassed);
void thing_find_enemy(const LevData *ld, int th);
void thing_autoaim(const LevData *ld, int th,
		   fixed arc, fixed *angle, fixed *elev);
void thing_autotarget(const LevData *ld, int th, fixed arc);
fixed line_dist(const LevData *ld, int wall, fixed x, fixed y);

void player_apply_lookup(const LevData *ld, int th, fixed lookup);

void thing_become(LevData *ld, int th, int id);
void thing_enter_phase(LevData *ld, int th, int ph);

/* return non-zero on fail */
int thing_send_sig(const LevData *ld, int th, ThingSignal sig);
int thing_sig_ok(const LevData *ld, int th, ThingSignal sig);

void thingd_apply_polar(ThingDyn *td, fixed force, fixed angle);
#define thingd_apply_forward(td,f) thingd_apply_polar(td,f,td->angle)
#define thingd_apply_sideways(td,f) thingd_apply_polar(td,f,td->angle+FIXED_HALF_PI)
void thingd_apply_torque(ThingDyn *td, fixed t);
void thingd_apply_up(ThingDyn *td, fixed f);

void thing_to_view(const LevData *ld, int thing, View *view, const ViewTrans *vt);
void camera_to_view(const LevData *ld, int cam, View *v, const ViewTrans *vx);
void update_camera(const LevData *ld, int cam, int follow);

void thing_rotate_image(const LevData *ld, int thing, fixed view_angle);

typedef enum {
   Thumped, Crossed, Activated, Damaged
} WallHitType;

/* back is set iff thing crossed from back to front, or hit back side */
void thing_hit_wall(LevData *ld, int thing,
		    int wall, int back, WallHitType t);

/* player pressed the activate button */
void thing_activate(LevData *ld, int thing, fixed maxdist);

void thing_chk_collide(LevData *ld, int thingnum);
void update_things(LevData *ld, int tickspassed);

void sector_crush_thing(LevData *ld, int sector, int thing);

void thingd_findsector(const LevData *ld, ThingDyn *td);
#define thing_findsector(ld,th) thingd_findsector(ld,ldthingd(ld)+(th))
int findsector(const LevData *ld, fixed x, fixed y, fixed z);

static inline int
reject_sectors(const LevData *ld, int s1, int s2)
{
   int bit, byte;
   if (s1 < 0 || s2 < 0)
      return 1;
   bit = s1 + (s2 * ldnsectors(ld));
#if 0				/* why doesn't this work? */
   asm("bt %%eax,%1\n\t"
       "setc %%al\n\t"
       "movzbl %%al,%%eax\n\t"
       : "=a"(bit)
       : "mr"(ldreject(ld))
       );
   return bit;
#else
   byte = bit / 8;
   bit %= 8;
   return (ldreject(ld)[byte] >> bit) & 1;
#endif
}

static inline int
reject_sector_wall(const LevData *ld, int s, int w)
{
   int side0 = ldline(ld)[w].side[0];
   int side1 = ldline(ld)[w].side[1];
   if (!reject_sectors(ld, s, ldside(ld)[side0].sector))
      return 0;
   if (side1 > 0 && !reject_sectors(ld, s, ldside(ld)[side1].sector))
      return 0;
   return 1;
}

static inline int
reject_sector_thing(const LevData *ld, int s, int t)
{
   return reject_sectors(ld, s, ldthingd(ld)[t].sector);
}

static inline int
reject_thing_thing(const LevData *ld, int t1, int t2)
{
   return reject_sector_thing(ld, ldthingd(ld)[t1].sector, t2);
}

#endif

// Local Variables:
// c-basic-offset: 3
// End:
