/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/gettable.c: Gettables.
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "libdumb/dsound.h"
#include "libdumb/fontmap.h"
#include "libdumb/gettableinwad.h"
#include "libdumb/texture.h"
#include "banner.h"
#include "draw.h"
#include "dumbdefs.pt"		/* for GETT_SPC_* */
#include "gettable.h"
#include "levdata.h"
#include "things.h"

static int ngettables = 0;
static Gettable *gettables = NULL;
static Texture ***gettxt = NULL;

#define NUMICONS(gk) (gettables[gk].iconanim		\
		      ? (1 + gettables[gk].iconanim	\
			   - gettables[gk].iconname[4])	\
		      : 1)
#define ANIMWAIT(gk) (gettables[gk].timing+1)
#define GTEXTURE(gk) get_gktex(ld,gk)

/* Values of n in ld->plinfo[pl][n] */
enum {
   PLINFOPOS_WEAPONIND,
   PLINFOPOS_ITEMIND,		/* must be PLINFOPOS_WEAPONIND+1 */
   PLINFOPOS_BACKPACK,
   PLINFOPOS_POWERWEAPONS,
   PLINFOPOS_GETTCOUNTS		/* must be last */
};

#define PLINFO_OK(pl)           (ld->plinfo[pl]!=NULL)
#define PLINFO_SEL(pl,seltype)  (ld->plinfo[pl][PLINFOPOS_WEAPONIND+(seltype)])
#define PLINFO_BACKPACK(pl)     (ld->plinfo[pl][PLINFOPOS_BACKPACK])
#define PLINFO_POWERWEAPONS(pl) (ld->plinfo[pl][PLINFOPOS_POWERWEAPONS])
/* we don't have PLINFO_GETTCOUNT; use gettcount() instead! */

#define GETT_ID_OK(gtid)        ((gtid) >= 0 && (gtid) < ngettables)

static int has_enough_ammo(LevData *ld, int pl, const Gettable *gt);
static int would_be_useless(LevData *ld, int pl, const Gettable *gt);
static void set_bogot(LevData *ld, int pl, const Gettable *gt);


static int *
gettcount_ptr(const LevData *ld, int pl, int gtid)
{
   assert(GETT_ID_OK(gtid));
   switch ((int) (gettables[gtid].special)) {
   case GETT_SPC_HEALTH:
      return &(ldthingd(ld)[ld->player[pl]].hits);
   case GETT_SPC_ARMOUR:
      return &(ldthingd(ld)[ld->player[pl]].armour);
   case GETT_SPC_INVISIBILITY:
      return &(ldthingd(ld)[ld->player[pl]].tmpinv);
   case GETT_SPC_DAMAGEPROTECTION:
      return &(ldthingd(ld)[ld->player[pl]].tmpgod);
  /* The above were kept in the generic thing structure,
   * but the ones below are specific to players.  */
   case GETT_SPC_BACKPACK:
      return &PLINFO_BACKPACK(pl);
   case GETT_SPC_POWERWEAPONS:
      return &PLINFO_POWERWEAPONS(pl);
   }
   return &(ld->plinfo[pl][PLINFOPOS_GETTCOUNTS+gtid]);
}

static int
gettcount(const LevData *ld, int pl, int gtid)
{
   return *gettcount_ptr(ld, pl, gtid);
}

static void
set_gettcount(LevData *ld, int pl, int gtid, int new_count)
{
   int *countp = gettcount_ptr(ld, pl, gtid);
   int old_count = *countp;
   *countp = new_count;

   /* Do special things if gaining or losing a gettable.  */
   if (!old_count ^ !new_count) {

      /* Select a new weapon automatically, if it is better than the
         currently selected one.  */
      if (gettables[gtid].flags & GK_WEPSELECT
	  && new_count > 0
	  && gtid > PLINFO_SEL(pl, 0)
	  && has_enough_ammo(ld, pl, &gettables[gtid])) {
	 PLINFO_SEL(pl, 0) = gtid;
	 set_bogot(ld, pl, &gettables[gtid]);
      }

      /* Let GETT_SPC_POWERWEAPONS take effect immediately.  */
      if (gettables[gtid].special == GETT_SPC_POWERWEAPONS
	  && GETT_ID_OK(PLINFO_SEL(pl, 0))) {
	 set_bogot(ld, pl, &gettables[PLINFO_SEL(pl, 0)]);
      }
   }
}

/* Return the current maximum of gettable GETID, depending on whether
 * player PL has the backpack.  */
static int
gettmax(LevData *ld, int pl, int gtid)
{
   if (PLINFO_BACKPACK(pl) > 0)
      return gettables[gtid].backpackmax;
   else
      return gettables[gtid].defaultmax;
}

static Texture *
get_gktex(LevData *ld, int gk)
{
   int i, ni;
   if (!gettxt[gk])
      return NULL;
   ni = NUMICONS(gk);
   i = ld->map_ticks / ANIMWAIT(gk);
   if (gettables[gk].flags & GK_REVANIM) {
      i %= 2 * (ni - 1);
      if (i < ni)
	 return gettxt[gk][i];
      i -= ni;
      return gettxt[gk][(ni - 2) - i];
   } else
      return gettxt[gk][i % ni];
}

int
get_plinfo_len(void)
{
   if (ngettables == 0)
      init_gettables();
   return PLINFOPOS_GETTCOUNTS + ngettables;
}

void
init_plinfo(LevData *ld, int pl)
{
   /* This is called by init_player() in levdata.c which callocs the
      data.  Therefore we don't have to zero any fields.  */
   int i;
   PLINFO_SEL(pl, 0) = -1;
   PLINFO_SEL(pl, 1) = -1;
   
   /* ok, then fill up the rest */
   for (i = 0; i < ngettables; i++)
      set_gettcount(ld, pl, i, gettables[i].initial);
}

static void
set_bogot(LevData *ld, int pl, const Gettable *gt)
{
   ThingDyn *b;
   int bogotype = PLINFO_POWERWEAPONS(pl) ? gt->powered_bogotype
                                          : gt->bogotype;
   if (ld->plwep[pl] < 0)
      ld->plwep[pl] = new_thing(ld, bogotype, 0, 0, 0);
   b = ldthingd(ld) + ld->plwep[pl];
   b->owner = ld->player[pl];
   if (b->proto == NULL || b->proto->id != bogotype) {
      b->proto = find_protothing(bogotype);
      if (b->proto) {
	 b->phase_tbl = find_first_thingphase(b->proto->phase_id);
	 b->hits = b->proto->hits;
	 b->phase = b->proto->signals[TS_INIT];
	 b->phase_wait = b->phase_tbl[b->phase].wait;
      }
   }
}

/* Return non-zero if player PL has enough ammo to use gettable GT.  */
static int
has_enough_ammo(LevData *ld, int pl, const Gettable *gt)
{
   return (pickup_gettables(ld, pl, gt->gets, gt->ngets, 1)
	   != GETT_PU_NOTENOUGH);
}

/* Return non-zero if player PL should not use gettable GT.  */
static int
would_be_useless(LevData *ld, int pl, const Gettable *gt)
{
   return !GETT_PU_IS_OK(pickup_gettables(ld, pl, gt->gets, gt->ngets, 1));
}

/* dir must be either +1 or -1 */
void
rotate_selection(LevData *ld, int pl, int seltype, int dir)
{
   int i = PLINFO_SEL(pl, seltype);
   int loops = -1;
   while (1) {
      i += dir;
      /* check bailout */
      if (loops++ > ngettables) {
	 i = -1;
	 break;
      }
      /* wrap around if necessary */
      if (i >= ngettables)
	 i = 0;
      else if (i < 0)
	 i = ngettables - 1;
      /* check selectable */
      if (seltype == 0 && !(gettables[i].flags & GK_WEPSELECT))
	 continue;
      if (seltype == 1 && !(gettables[i].flags & GK_SPESELECT))
	 continue;
      /* check we have one */
      if (gettcount(ld, pl, i) < 1)
	 continue;
      /* This prevents the player from selecting a weapon for which he
         doesn't have enough ammo.  Perhaps that should be allowed?  */
      if (!has_enough_ammo(ld, pl, &gettables[i]))
	 continue;
      /* a strange loop, but it works */
      break;
   }
   PLINFO_SEL(pl, seltype) = i;
   if (GETT_ID_OK(i))
      set_bogot(ld, pl, &gettables[PLINFO_SEL(pl, seltype)]);
}

void
use_selection(int seltype, LevData *ld, int pl)
{
   const Gettable *gt = &gettables[PLINFO_SEL(pl, seltype)];
   if (!GETT_ID_OK(PLINFO_SEL(pl, seltype))) {
      rotate_selection(ld, pl, seltype, +1);
      return;
   }
   if (gettcount(ld, pl, PLINFO_SEL(pl, seltype)) < 1
       || !has_enough_ammo(ld, pl, gt)) {
      /* Gettable lost or not enough ammo; switch to the next
         weapon/item. */
      rotate_selection(ld, pl, seltype, +1);
      return;
   }
   if (would_be_useless(ld, pl, gt)) {
      /* This can happen if GT is a Heretic quartz flask or similar
         and the player already has full health.  In this case, the
         gettable should not be deselected, since the player may need
         it in a moment.  */
      return;
   }
   set_bogot(ld, pl, gt);
   if (!use_item(ld, pl, gt))
      return;
   if (gt->usesound >= 0)
      play_dsound(gt->usesound, 0, 0, 0); /* FIXME: position? */
   /* Take any gettables this one wants to give.  This also decrements
      the ammo count if necessary.  */
   pickup_gettables(ld, pl, gt->gets, gt->ngets, 0);
}


/* pickup */

enum gettable_pickup
pickup_gettable(LevData *ld, int pl, const Gets *gets, int dry_run)
{
   int old_count = gettcount(ld, pl, gets->gtid);
   int new_count = old_count + gets->change;
   int max = gets->maximum==0 ? gettmax(ld, pl, gets->gtid) : gets->maximum;
   if (gets->change > 0) {
      if (new_count > max)
	 new_count = max;
      if (new_count <= old_count)
	 return GETT_PU_USELESS;
   } else if (gets->change < 0) {
      if (new_count < 0)
	 return GETT_PU_NOTENOUGH;
   }
   if (!dry_run)
      set_gettcount(ld, pl, gets->gtid, new_count);
   if (gets->change < 0)
      return GETT_PU_ENOUGH;
   else if (old_count == 0)
      return GETT_PU_GOTFIRST;
   else
      return GETT_PU_GOT;
}

enum gettable_pickup
pickup_gettables(LevData *ld, int pl, const Gets gets[], size_t ngets,
		 int dry_run)
{
   size_t i;
   int first_flag = 0;
   /* USEFUL_FLAG is set if the item succeeds in giving a gettable.
      USELESS_FLAG is set if the item attempts to give a gettable but
      fails.  */
   int useful_flag = 0, useless_flag = 0;
   for (i = 0; i < ngets; i++) {
      enum gettable_pickup pu = pickup_gettable(ld, pl, &gets[i], dry_run);
      if (pu == GETT_PU_NOTENOUGH)
	 return GETT_PU_NOTENOUGH;
      if (GETT_PU_IS_USEFUL(pu))
	 useful_flag = 1;
      if (pu == GETT_PU_USELESS)
	 useless_flag = 1;
      if (i == 0 && pu == GETT_PU_GOTFIRST)
	 first_flag = 1;
   }
   if (useless_flag && !useful_flag)
      return GETT_PU_USELESS;
   else if (first_flag)
      return GETT_PU_GOTFIRST;
   else if (useful_flag)
      return GETT_PU_GOT;
   else
      return GETT_PU_ENOUGH;
}


/* draw funcs */

static Font *count_font = NULL;

static void
draw_count(void *fb, int c, int x, int y)
{
   wchar_t buf[3];
   if (count_font == NULL)
      return;
   buf[0] = L'0' + (c / 100) % 10;
   buf[1] = L'0' + (c / 10) % 10;
   buf[2] = L'0' + c % 10;
   x -= font_wc_text_width(count_font, buf, 3) / 2;
   draw_wc_text(fb, count_font, buf, 3, x, y);
}

void
draw_gettables(LevData *ld, int pl,
	       void *fb, int width, int height)
{
   int i;
   for (i = 0; i < ngettables; i++) {
      const Gettable *gt = &gettables[i];
      int xo = gt->xo, yo = gt->yo;
      int cnt = gettcount(ld, pl, i);
      Texture *gtx = GTEXTURE(i);
      if (gtx == NULL || cnt < 1)
	 continue;
      if (xo < 0) {
	 /* if xo==-1, width==320 and gtx->width==1, then xo=319 */
	 xo += 1 + width - gtx->width;
	 if (gt->flags & GK_XCENTERICON)
	    xo += gtx->width / 2;
      } else {
	 if (gt->flags & GK_XCENTERICON)
	    xo -= gtx->width / 2;
      }
      if (yo < 0) {
	 /* This used to subtract (gettmax(ld,pl,i) > 1 ? 16 : 0),
	    but now we let the .pt author take care of that.  */
	 yo += 1 + height - gtx->height;
	 if (gt->flags & GK_YCENTERICON)
	    yo += gtx->height / 2;
      } else {
	 if (gt->flags & GK_YCENTERICON)
	    yo -= gtx->height / 2;
      }
      /* now draw icon */
      if ((gt->flags & GK_WEPSELECT) && i == PLINFO_SEL(pl, 0))
	 draw_outline(fb, gtx, xo, yo);
      else if ((gt->flags & GK_SPESELECT) && i == PLINFO_SEL(pl, 1))
	 draw_outline(fb, gtx, xo, yo);
      else
	 draw(fb, gtx, xo, yo);
      if (gettmax(ld, pl, i) > 1 && !gt->decay)
	 draw_count(fb, cnt, xo + gtx->width/2, yo + gtx->height + 3);
   }
}


/* do decay stuff */
void
update_gettables(LevData *ld, int ticks)
{
   int pl;
   for (pl = 0; pl < MAXPLAYERS; pl++) {
      int i;
      if (!PLINFO_OK(pl))
	 continue;
      for (i = 0; i < ngettables; i++) {
	 const Gettable *gt = &gettables[i];
	 int count = gettcount(ld, pl, i);
	 if (count > 0 && gt->decay) {
	    count -= ticks * gt->decay;
	    if (count < 0)
	       count = 0;
	    set_gettcount(ld, pl, i, count);
	 }
      }
   }
}

/* cheat */
void
cheat_gettables(LevData *ld, int pl)
{
   int i;
   /* Since the player is (probably) given the backpack in the
    * process, we use the with-backpack maximum.  */
   for (i = 0; i < ngettables; i++)
      set_gettcount(ld, pl, i, gettables[i].backpackmax);
}

/* reset locals */
void
reset_local_gettables(LevData *ld)
{
   int pl;
   for (pl = 0; pl < MAXPLAYERS; pl++) {
      int i;
      if (!PLINFO_OK(pl))
	 continue;
      for (i = 0; i < ngettables; i++) {
	 const Gettable *gt = &gettables[i];
	 if (gt->flags & GK_ONEMAPONLY)
	    set_gettcount(ld, pl, i, gt->initial);
      }
   }
}

/* return non-zero if we have a key of type <keytype> */
int
gettable_chk_key(const LevData *ld, int pl, int keytype)
{
   int i;
   for (i = 0; i < ngettables; i++)
      if (gettcount(ld, pl, i) > 0
	  && gettables[i].key == keytype)
	 return 1;
   return 0;
}


/* init and reset funcs */

void
init_gettables(void)
{
   int i;
   LumpNum gettable_ln;
   /* if already inited, throw away old info first */
   if (ngettables > 0)
      reset_gettables();
   if (count_font != NULL)
      free_font(count_font);
   count_font = load_mapped_font(game_fontmap, FONTMAP_GETTABLECOUNT);
   /* load lump and allocate GetDs */
   gettable_ln = lookup_lump("GETTABLE", NULL, NULL);
   if (!LUMPNUM_OK(gettable_ln)) {
      logprintf(LOG_WARNING, 'G', _("warning: no gettables"));
      return;
   }
   ngettables = decode_gettableinwad_array(&gettables,
					   ((const Gettables_inwad *)
					    load_lump(gettable_ln)),
					   get_lump_len(gettable_ln),
					   "GETTABLE");
   logprintf(LOG_INFO, 'G', _("init %d gettables"), ngettables);
   free_lump(gettable_ln);
   gettxt = (Texture ***) safe_calloc(sizeof(Texture **), ngettables);
   /* initialise */
   for (i = 0; i < ngettables; i++) {
      int j;
      const Gettable *gt = &gettables[i];
      char buf[LUMPNAMELEN+1];
      if (gt->iconname[0] && have_lump(gt->iconname)) {
	 gettxt[i] = (Texture **) safe_calloc(sizeof(Texture *), NUMICONS(i));
	 strcpy(buf, gt->iconname);
	 for (j = NUMICONS(i) - 1; j >= 0; j--) {
	    buf[4] = gt->iconname[4] + j;
	    gettxt[i][j] = get_misc_texture(buf);
	 }
      }
   }
}

void
reset_gettables(void)
{
   if (gettxt) {
      int i;
      for (i = 0; i < ngettables; i++)
	 if (gettxt[i])
	    safe_free(gettxt[i]);
      safe_free(gettxt);
   }
   safe_free(gettables);
   ngettables = 0;
   gettables = NULL;
   gettxt = NULL;
   if (count_font != NULL) {
      free_font(count_font);
      count_font = NULL;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
