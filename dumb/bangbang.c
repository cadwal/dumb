/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/bangbang.c: Shooting and hurling.
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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/fixed.h"
#include "things.h"

int
thing_hurl(LevData *ld, int thnum, int mtype, fixed a, fixed elev,
	   fixed arc, int num, int para)
{
   ThingDyn *td = ldthingd(ld) + thnum, *md;
   int missile = -1, i;
   fixed ofs = -arc / 2, step = 0;
   /* check that we aren't overspawning */
   if (td->spawncount >= td->proto->spawnmax) {
      logprintf(LOG_DEBUG, 'O', "spawncount exceeded for %d", thnum);
      return -1;
   };
   if (num < 1)
      num = 1;
   if (num > 1)
      step = arc / (num - 1);
   for (i = 0; i < num; i++) {
      fixed sina, cosa, sine;
      /* make missile */
      missile = new_thing(ld, mtype, td->x, td->y, td->z + SHOOT_HEIGHT(td));
      md = ldthingd(ld) + missile;
      if (td->owner >= 0)
	 md->owner = td->owner;	/* FIXME: interactions with spawncount? */
      else
	 md->owner = thnum;
      if (md->owner >= 0)
	 ldthingd(ld)[md->owner].spawncount++;
      /* work out angle to shoot it */
      md->angle = a + td->angle;
      if (!para)
	 md->angle += ofs;
      md->dx = fixmul(md->proto->speed, cosa = fixcos(md->angle));
      md->dy = fixmul(md->proto->speed, sina = fixsin(md->angle));
      md->dz = fixmul(md->proto->speed, sine = fixsin(elev));
#if 0
      /* make sure that missile won't be crashing into us next round */
      r = md->proto->radius + td->proto->radius;
      md->x += fixmul(r, cosa);
      md->y += fixmul(r, sina);
      md->z += fixmul(r, sine);
#endif
      /* offset missile if para_shoot */
      if (para) {
	 md->x += fixmul(ofs, sina);
	 md->y += fixmul(ofs, cosa);
      }
      /* find sector that missile ended up in */
      thingd_findsector(ld, md);
      /* early check for collisions */
      thing_chk_collide(ld, missile);
      /* next missile */
      ofs += step;
   }
   return missile;
}


#define WALL_VER1(i) (ldvertexd(ld)[ldline(ld)[i].ver1])
#define WALL_VER2(i) (ldvertexd(ld)[ldline(ld)[i].ver2])
#define WALL_SIDE(i,j) (ldsided(ld)[ldline(ld)[i].side[j]])
#define WALL_SCT(i,j) (ldsectord(ld)[ldside(ld)[ldline(ld)[i].side[j]].sector])
#define WALL_SCTN(i,j) (ldside(ld)[ldline(ld)[i].side[j]].sector)
#define WALL_2S(i) (ldline(ld)[i].side[1]>=0)
static fixed
wall_ray_intersection(const LevData *ld, fixed Vx, fixed Vy,
		      fixed Ox, fixed Oy, int wall, int *side)
{
   fixed denominator, numerator;

#ifndef HAVE_MMX

   const fixed Nx = -Vy;
   const fixed Ny = Vx;
   const fixed Wx = WALL_VER2(wall).x - WALL_VER1(wall).x;
   const fixed Wy = WALL_VER2(wall).y - WALL_VER1(wall).y;
   denominator = fixmul(Nx, Wx) + fixmul(Ny, Wy);	/* N dot W */
   numerator = (fixmul(Nx, WALL_VER1(wall).x - Ox) +
		fixmul(Ny, WALL_VER1(wall).y - Oy));
  
#else  /* HAVE_MMX */
   
   __asm__(
	   /* load (Nx,Ny) into mm0 */
	   "movd %3, %%mm0\n\t"
	   "movd %2, %%mm2\n\t"
	   "psllq $32, %%mm0\n\t"
	   "por %%mm2, %%mm0\n\t"
	   
	   /* load (Wx,Wy) and (W1x,W1y) into mm2 and mm3 */
	   "movq %4, %%mm2\n\t"
	   "movq %5, %%mm3\n\t"
	   "psubd %%mm3, %%mm2\n\t"
	   "psubd %6, %%mm3\n\t"

	   /* shift everything right by 8 bits to adjust for fixed point mul */
	   "psrad $8, %%mm0\n\t"
	   "psrad $8, %%mm2\n\t"
	   "psrad $8, %%mm3\n\t"

	   /* pack the regs up right ready for pmaddwd */
	   "packssdw %%mm0, %%mm0\n\t"
	   "packssdw %%mm3, %%mm2\n\t"
	   
	   /* two dot products in one insn: who says CISC is dead? */
	   "pmaddwd %%mm2, %%mm0\n\t"
	   
	   /* now unpack the results */
	   "movd %%mm0, %0\n\t"
	   "psrlq $32, %%mm0\n\t"
	   "movd %%mm0, %1\n\t"
	   "emms\n\t"

	   : "=mr"(denominator), "=mr"(numerator)
	   : "mr"(-Vy), "mr"(Vx),
	   "m"(WALL_VER2(wall).x), "m"(WALL_VER1(wall).x),
	   "m"(Ox)
	   );

#endif /* HAVE_MMX */

   if (denominator > FIXED_ONE) {
      if (numerator > 0 || numerator < -denominator)
	 return FIXED_ZERO;
      *side = 0;
      return -fixdiv(numerator, denominator);
   } else if (denominator < -FIXED_ONE) {
      if (numerator > 0 || numerator < -denominator)
	 return FIXED_ZERO;
      *side = 1;
      return FIXED_ONE + fixdiv(numerator, denominator);
   } else
      return FIXED_ZERO;
}


inline double
pyth_sq(fixed x, fixed y)
{
   double dx = FIXED_TO_FLOAT(x), dy = FIXED_TO_FLOAT(y);
   return dx * dx + dy * dy;
}

int
thing_can_shoot_at(const LevData *ld, int looker, int target)
{
   /* mostly borrowed from bullet code, below */
   int i;
   const ThingDyn *std = ldthingd(ld) + looker;
   const ThingDyn *ttd = ldthingd(ld) + target;

   double pdist2, dist2 = pyth_sq(ttd->x - std->x, ttd->y - std->y);
   fixed pdist, dist = FLOAT_TO_FIXED(sqrt(dist2));
   fixed vx, vy, vz;

   if (dist < FIXED_EPSILON)
      return 1;
   vx = fixdiv(ttd->x - std->x, dist);
   vy = fixdiv(ttd->y - std->y, dist);
   vz = fixdiv((ttd->z + SHOOT_HEIGHT(ttd)) - (std->z + SHOOT_HEIGHT(std)), 
	       dist);

   for (i = 0; i < ldnlines(ld); i++) {
      const SectorDyn *front = &WALL_SCT(i, 0), *back = &WALL_SCT(i, 1);
      int side;
      fixed t, sx, sy, sz;

      /* avoid w-r-i for walls entirely behind ray */
      if ((FIXED_PRODUCT_SIGN(WALL_VER1(i).x - std->x, vx) ||
	   FIXED_PRODUCT_SIGN(WALL_VER1(i).y - std->y, vy)) &&
	  (FIXED_PRODUCT_SIGN(WALL_VER2(i).x - std->x, vx) ||
	   FIXED_PRODUCT_SIGN(WALL_VER2(i).y - std->y, vy)))
	 continue;

      t = wall_ray_intersection(ld, vx, vy, std->x, std->y, i, &side);
      if (t <= FIXED_ZERO || t > FIXED_ONE)
	 continue;

      /* (sx,sy) will be the exact point where bullet would strike line */
      sx = (fixmul(WALL_VER1(i).x, FIXED_ONE - t) + fixmul(WALL_VER2(i).x, t));
      sy = (fixmul(WALL_VER1(i).y, FIXED_ONE - t) + fixmul(WALL_VER2(i).y, t));

      /* check point isn't behind origin of ray */
      if (FIXED_PRODUCT_SIGN(sx - std->x, vx) ||
	  FIXED_PRODUCT_SIGN(sy - std->y, vy))
	 continue;

      /* now calculate distance */
      pdist2 = pyth_sq(sx - std->x, sy - std->y);

      /* bail if wall is behind target */
      if (pdist2 > dist2)
	 continue;
      
      /* can't ever see through one-sided wall */
      if (!WALL_2S(i)) return 0;

      /* calculate z posn of line of sight on wall */
      pdist = FLOAT_TO_FIXED(sqrt(pdist2));
      sz = std->z + SHOOT_HEIGHT(std) + fixmul(pdist, vz);

      if(sz <= front->floor || sz <= back->floor) return 0;
      if(sz >= front->ceiling || sz >= back->ceiling) return 0;

      /* we saw through this wall, continue */
   }

   /* no walls seemed to intervene; they can shoot each other, if they want */
   return 1;
}

void
thing_activate(LevData *ld, int thingnum, fixed maxdist)
{
   double mindist_sq = FIXED_TO_FLOAT(maxdist) * FIXED_TO_FLOAT(maxdist);
   const ThingDyn *std = ldthingd(ld) + thingnum;
   int i, wall = -1, wall_side = 0;
   fixed vx, vy;
   vy = fixsin(std->angle);
   vx = fixcos(std->angle);
   for (i = 0; i < ldnlines(ld); i++) {
      int side;
      fixed t, d, sx, sy;
      double dsq;

      if (ldline(ld)[i].type == 0)
	 continue;

      t = wall_ray_intersection(ld, vx, vy, std->x, std->y, i, &side);
      if (t <= FIXED_ZERO || t > FIXED_ONE)
	 continue;

      /* (sx,sy) will be the exact point where bullet would strike line */
      sx = (fixmul(WALL_VER1(i).x, FIXED_ONE - t) + fixmul(WALL_VER2(i).x, t));
      sy = (fixmul(WALL_VER1(i).y, FIXED_ONE - t) + fixmul(WALL_VER2(i).y, t));

      /* check point isn't behind origin of ray */
      if (FIXED_PRODUCT_SIGN(sx - std->x, vx) ||
	  FIXED_PRODUCT_SIGN(sy - std->y, vy))
	 continue;

      /* now calculate distance */
      dsq = pyth_sq(sx - std->x, sy - std->y);
      if (mindist_sq <= dsq)
	 continue;
      d = FLOAT_TO_FIXED(sqrt(dsq));

      /* 2S walls get special treatment */
      if (WALL_2S(i)) {
	 const SectorDyn *front = &WALL_SCT(i, side);
	 const SectorDyn *back = &WALL_SCT(i, 1 - side);
	 /* skip walls with no lower or middle surface */
	 if (front->floor >= back->floor)
	    continue;
      }
      /* phew! we might hit this wall */
      wall = i;
      wall_side = side;
      mindist_sq = dsq;
   }

   /* TODO: allow activation of things, floors, ceilings? */

   /* now do the activation */
   if (wall >= 0)
      thing_hit_wall(ld, thingnum, wall, wall_side, Activated);
}

static int
thing_shoot_single(LevData *ld, int shooter, int bullet_id,
		   fixed angle, fixed elev)
{
   double mindist_sq = FIXED_TO_FLOAT(FIXED_MAX) * FIXED_TO_FLOAT(FIXED_MAX);
   fixed bx = 0, by = 0, bz = 0, oz;
   int wall = -1, thing = -1, fsect = -1, csect = -1;
   const ThingDyn *std = ldthingd(ld) + shooter;
   int nohurto = -1;
   int i, wall_side = 0;
   fixed vx, vy, vz;
   angle += std->angle;
   vy = fixsin(angle);
   vx = fixcos(angle);
   vz = fixtan(elev);
   oz = std->z + SHOOT_HEIGHT(std);
   if (std->proto == NULL)
      return -1;
   if (std->proto->flags & PT_NOHURTO)
      nohurto = std->owner;
   for (i = 0; i < ldnlines(ld); i++) {
      int side;
      fixed t, d, sx, sy, sz;
      double dsq;

      /* avoid w-r-i for walls entirely behind ray */
      if ((FIXED_PRODUCT_SIGN(WALL_VER1(i).x - std->x, vx) ||
	   FIXED_PRODUCT_SIGN(WALL_VER1(i).y - std->y, vy)) &&
	  (FIXED_PRODUCT_SIGN(WALL_VER2(i).x - std->x, vx) ||
	   FIXED_PRODUCT_SIGN(WALL_VER2(i).y - std->y, vy)))
	 continue;

      t = wall_ray_intersection(ld, vx, vy, std->x, std->y, i, &side);
      if (t <= FIXED_ZERO || t > FIXED_ONE)
	 continue;

      /* (sx,sy) will be the exact point where bullet would strike line */
      sx = (fixmul(WALL_VER1(i).x, FIXED_ONE - t) + fixmul(WALL_VER2(i).x, t));
      sy = (fixmul(WALL_VER1(i).y, FIXED_ONE - t) + fixmul(WALL_VER2(i).y, t));

      /* check point isn't behind origin of ray */
      if (FIXED_PRODUCT_SIGN(sx - std->x, vx) ||
	  FIXED_PRODUCT_SIGN(sy - std->y, vy))
	 continue;

      /* now calculate distance */
      dsq = pyth_sq(sx - std->x, sy - std->y);
      if (mindist_sq <= dsq)
	 continue;
      d = FLOAT_TO_FIXED(sqrt(dsq));

      /* calculate where z is up to */
      sz = oz + fixmul(d, vz);

      /* 2S walls get special treatment */
      if (WALL_2S(i)) {
	 const SectorDyn *front = &WALL_SCT(i, side);
	 const SectorDyn *back = &WALL_SCT(i, 1 - side);
	 /* OK, we're hitting this wall.  would we hit: */
	 /* 1) the floor */
	 /* 2) the ceiling */
	 if (sz < front->floor || sz > front->ceiling);
	 /* 3) the upper segment */
	 else if (sz > back->ceiling) {
	    if (front->sky & back->sky & 1)
	       continue;
	 }
	 /* 4) the lower segment */
	 else if (sz < back->floor) {
	    if (front->sky & back->sky & 2)
	       continue;
	 }
	 /* 5) the middle */
	 else
	    continue;
      }
      /* will we hit a floor or ceiling? */
      if (sz > WALL_SCT(i, side).ceiling) {
	 fixed diff;
	 /* if we're travelling parallel to the sector, there's
	    another wall we'll hit first */
	 if (FIXED_ABS(vz) <= FIXED_EPSILON)
	    continue;
	 /* can't hit sky */
	 if (WALL_SCT(i, side).sky & 1)
	    continue;
	 /* otherwise we need to wind back distance a bit */
	 diff = fixdiv((sz - WALL_SCT(i, side).ceiling) << 8, vz << 8);
	 sz = WALL_SCT(i, side).ceiling;
	 sx -= fixmul(diff, vx);
	 sy -= fixmul(diff, vy);
	 /* record what we did */
	 fsect = wall = -1;
	 csect = WALL_SCTN(i, side);
      } else if (sz < WALL_SCT(i, side).floor) {
	 fixed diff;
	 /* if we're travelling parallel to the sector, there's
	    another wall we'll hit first */
	 if (FIXED_ABS(vz) <= FIXED_EPSILON)
	    continue;
	 /* can't hit sky */
	 if (WALL_SCT(i, side).sky & 2)
	    continue;
	 /* otherwise we need to wind back distance a bit */
	 diff = fixdiv((WALL_SCT(i, side).floor - sz) << 8, vz << 8);
	 sz = WALL_SCT(i, side).floor;
	 sx -= fixmul(diff, vx);
	 sy -= fixmul(diff, vy);
	 /* record what we did */
	 csect = wall = -1;
	 fsect = WALL_SCTN(i, side);
      }
      /* phew! we might hit this wall */
      else {
	 csect = fsect = -1;
	 wall = i;
	 wall_side = side;
      }

      /* these divisions put the puff FIXED_ONE_HALF in front of the wall
         that's about 8 pixels worth */
      bx = sx - vx / 2;
      by = sy - vy / 2;
      bz = sz - vz / 2;
      mindist_sq = dsq;
   }

   /* now see if things will be hit */
   for (i = 0; i < ldnthings(ld); i++) {
      fixed d, sx, sy, sz;
      double dsq;
      const ThingDyn *td = ldthingd(ld) + i;
      if (i == shooter || i == nohurto || td->proto == NULL ||
	  (td->proto->flags & (PT_PHANTOM | PT_BULLETPROOF)))
	 continue;
      /* calculate distance squared */
      dsq = pyth_sq(td->x - std->x, td->y - std->y);
      if (mindist_sq <= dsq)
	 continue;
      /* calculate real distance */
      d = FLOAT_TO_FIXED(sqrt(dsq));
      if (d < td->proto->radius / 2)
	 continue;
      sx = fixmul(vx, d) + std->x;
      sy = fixmul(vy, d) + std->y;
      /* will this object get hit? */
      if (fix_pythagoras(td->x - sx, td->y - sy) > td->proto->radius)
	 continue;
      /* check z dimension */
      sz = oz + fixmul(vz, d);
      if (sz < td->z || sz > td->z + td->proto->height)
	 continue;
      /* we got one */
      wall = fsect = csect = -1;
      thing = i;
      mindist_sq = dsq;
      bx = sx - (10 * vx) / 8;
      by = sy - (10 * vy) / 8;
      bz = sz - (10 * vz) / 8;
   }

   /* bloodspurt "kludge" */
   if (thing >= 0 &&
       (find_protothing(bullet_id)->flags & PT_BECOMES_BLOOD) &&
       ldthingd(ld)[thing].proto->bloodtype > 0)
      bullet_id = ldthingd(ld)[thing].proto->bloodtype;

   /* create bullet puff */
   i = new_thing(ld, bullet_id, bx, by, bz);
   ldthingd(ld)[i].angle = angle;
   ldthingd(ld)[i].target = thing;
   ldthingd(ld)[i].owner = shooter;

   /* shot a wall? */
   if (wall >= 0)
      thing_hit_wall(ld, i, wall, wall_side, Damaged);

   /*
      if(wall>=0)
      logprintf(LOG_DEBUG, 'O', _("bullet (%d) struck wall %d"), i, wall);
      if(thing>=0)
      logprintf(LOG_DEBUG, 'O', _("bullet (%d) struck thing %d"), i, thing);
      if(fsect>=0)
      logprintf(LOG_DEBUG, 'O', _("bullet (%d) struck floor %d"), i, fsect);
      if(csect>=0)
      logprintf(LOG_DEBUG, 'O', _("bullet (%d) struck ceiling %d"), i, csect);
    */

   return i;
}

void
thing_shoot(LevData *ld, int th, int bullet_id,
	    fixed angle, fixed elev, fixed horiz_arc, fixed vert_arc,
	    int num, int para)
{
   while (num-- > 0) {
      fixed a = 0, e = 0;
      if (horiz_arc > FIXED_EPSILON)
	 a = (rand() % horiz_arc) - horiz_arc / 2;
      if (vert_arc > FIXED_EPSILON)
	 e = (rand() % vert_arc) - vert_arc / 2;
      thing_shoot_single(ld, th, bullet_id, angle + a, elev + e);
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
