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
#include "libdumb/texture.h"
#include "libdumb/dsound.h"
#include "banner.h"
#include "draw.h"
#include "dumbdefs.pt"		/* for GETT_SPC_* */
#include "gettable.h"
#include "levdata.h"
#include "things.h"

static LumpNum gettable_ln = BAD_LUMPNUM;
static int ngetts = 0;
static const Gettable **gett = NULL;
static Texture ***gettxt = NULL;

#define NUMICONS(gk) (gett[gk]->iconanim? \
		      (1+gett[gk]->iconanim-gett[gk]->iconname[4]): \
		      1)
#define ANIMWAIT(gk) (gett[gk]->timing+1)
#define GTEXTURE(gk) get_gktex(ld,gk)

/* Values of n in ld->plinfo[pl][n] */
enum {
   PLINFOPOS_WEAPONIND,
   PLINFOPOS_ITEMIND,		/* must be PLINFOPOS_WEAPONNUM+1 */
   PLINFOPOS_BACKPACK,
   PLINFOPOS_GETTCOUNTS		/* must be last */
};

#define PLINFO_OK(pl) (ld->plinfo[pl]!=NULL)
#define PLINFO_SEL(pl,type) (ld->plinfo[pl][PLINFOPOS_WEAPONIND+(type)])
#define PLINFO_BACKPACK(pl) (ld->plinfo[pl][PLINFOPOS_BACKPACK])
/* we don't have PLINFO_GETTCOUNT; use getcount() instead! */

/* in fact this should be overridden for both const and non-const
   LevData *, and return const or non-const int *.  */
static int *
getcount(const LevData *ld, int pl, int type)
{
   assert(type >= 0 && type < ngetts);
   switch ((int) (gett[type]->special)) {
   case GETT_SPC_HEALTH:
      return &(ldthingd(ld)[ld->player[pl]].hits);
   case GETT_SPC_ARMOUR:
      return &(ldthingd(ld)[ld->player[pl]].armour);
   case GETT_SPC_INVISIBILITY:
      return &(ldthingd(ld)[ld->player[pl]].tmpinv);
   case GETT_SPC_DAMAGEPROTECTION:
      return &(ldthingd(ld)[ld->player[pl]].tmpgod);
   case GETT_SPC_BACKPACK:
      /* The above were kept in the generic thing structure, but this
       * one is specific to the player.  */
      return &PLINFO_BACKPACK(pl);
   }
   return &(ld->plinfo[pl][PLINFOPOS_GETTCOUNTS+type]);
}

/* Return the current maximum of gettable TYPE, depending on whether
 * player PL has the backpack.  */
static int
gettmax(LevData *ld, int pl, int type)
{
   if (PLINFO_BACKPACK(pl) > 0)
      return gett[type]->backpackmax;
   else
      return gett[type]->defaultmax;
}

static Texture *
get_gktex(LevData *ld, int gk)
{
   int i, ni;
   if (!gettxt[gk])
      return NULL;
   ni = NUMICONS(gk);
   i = ld->map_ticks / ANIMWAIT(gk);
   if (gett[gk]->flags & GK_REVANIM) {
      i %= 2 * (ni - 1);
      if (i < ni)
	 return gettxt[gk][i];
      i -= ni;
      return gettxt[gk][(ni - 2) - i];
   } else
      return gettxt[gk][i % ni];
}

/* Scan the memory area beginning at DATA and having length LENGTH for
   gettables.  Return the number of gettables.  If PTRS is not NULL,
   save the addresses of gettables there.  */
static int
scan_gettables(const void *data, size_t length, const Gettable **ptrs)
{
   int count = 0;
   while (length > 0) {
      size_t blk_length = ((const Gettable *) data)->block_length;
      if (blk_length < sizeof(Gettable) || blk_length > length) {
	 logprintf(LOG_ERROR, 'G', _("Gettable block chain corrupted"));
	 return count;
      }
      if (ptrs)
	 ptrs[count] = (const Gettable *) data;
      count++;
      data = ((const char *) data) + blk_length;
      length -= blk_length;	/* cannot underflow */
   }
   return count;
}

int
get_plinfo_len(void)
{
   if (ngetts == 0)
      init_gettables();
   return PLINFOPOS_GETTCOUNTS + ngetts;
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
   for (i = 0; i < ngetts; i++)
      *getcount(ld, pl, i) = gett[i]->initial;
}

static void
set_bogot(LevData *ld, int pl, const Gettable *gt)
{
   ThingDyn *b;
   if (ld->plwep[pl] < 0)
      ld->plwep[pl] = new_thing(ld, gt->bogotype, 0, 0, 0);
   b = ldthingd(ld) + ld->plwep[pl];
   b->owner = ld->player[pl];
   if (b->proto == NULL || b->proto->id != gt->bogotype) {
      b->proto = find_protothing(gt->bogotype);
      if (b->proto) {
	 b->phase_tbl = find_first_thingphase(b->proto->phase_id);
	 b->hits = b->proto->hits;
	 b->phase = b->proto->signals[TS_INIT];
	 b->phase_wait = b->phase_tbl[b->phase].wait;
      }
   }
}

/* dir must be either +1 or -1 */
void
rotate_selection(LevData *ld, int pl, int type, int dir)
{
   int i = PLINFO_SEL(pl, type);
   int loops = -1;
   while (1) {
      i += dir;
      /* check bailout */
      if (loops++ > ngetts) {
	 i = -1;
	 break;
      }
      /* wrap around if necessary */
      if (i >= ngetts)
	 i = 0;
      else if (i < 0)
	 i = ngetts - 1;
      /* check selectable */
      if (type == 0 && !(gett[i]->flags & GK_WEPSELECT))
	 continue;
      if (type == 1 && !(gett[i]->flags & GK_SPESELECT))
	 continue;
      /* check we have one */
      if (*getcount(ld, pl, i) < 1)
	 continue;
      if (gett[i]->ammotype >= 0
	  && *getcount(ld, pl, gett[i]->ammotype) < 1)
	 continue;
      /* a strange loop, but it works */
      break;
   }
   PLINFO_SEL(pl, type) = i;
   if (i >= 0)
      set_bogot(ld, pl, gett[PLINFO_SEL(pl, type)]);
}

void
use_selection(int type, LevData *ld, int pl)
{
   const Gettable *gt = gett[PLINFO_SEL(pl, type)];
   int *itemptr, *ammoptr;
   if (PLINFO_SEL(pl, type) < 0) {
      rotate_selection(ld, pl, type, +1);
      return;
   }
   itemptr = getcount(ld, pl, PLINFO_SEL(pl, type));
   if (gt->ammotype >= 0)
      ammoptr = getcount(ld, pl, gt->ammotype);
   else
      ammoptr = itemptr;
   if (*itemptr < 1 || *ammoptr < gt->ammocount) {
      rotate_selection(ld, pl, type, +1);
      return;
   }
   set_bogot(ld, pl, gett[PLINFO_SEL(pl, type)]);
   *ammoptr -= gt->ammocount * use_item(ld, pl, gt);
   if (*ammoptr < 0)
      *ammoptr = 0;
   if (gt->usesound >= 0)
      play_dsound(gt->usesound, 0, 0, 0);
}

/* pickup */

enum gettable_pickup
pickup_gettable(LevData *ld, int pl, int type, int num, int max)
{
   if (type < 0 || type >= ngetts) {
      logprintf(LOG_ERROR, 'G', _("Strange gettable type=%d num=%d"),
		type, num);
      return GETT_PU_NO_ERROR;
   } else {
      int *cntp = getcount(ld, pl, type);
      if (max == 0)
	 max = gettmax(ld, pl, type);
      logprintf(LOG_DEBUG, 'G', "pickup_gettable: pl=%d type=%d num=%d max=%d",
		pl, type, num, max);
      if (*cntp >= max)
	 return GETT_PU_NO_HASMAX;
      else {
	 /* do the pickup */
	 int oldcnt = *cntp;
	 *cntp += num;
	 if (*cntp > max)
	    *cntp = max;
	 return (oldcnt==0) ? GETT_PU_YES_FIRST : GETT_PU_YES_MORE;
      }
   }
}


/* draw funcs */

static int count_font = -1;

static void
init_cfont(void)
{
   if (count_font < 0) {
      if (have_lump("IN0"))
	 count_font = init_font("SMALLIN%d", 10, 0);
      else
	 count_font = init_font("STYSNUM%d", 10, 0);
   }
}

static void
draw_count(void *fb, int c, int x, int y)
{
   char buf[3];
   init_cfont();
   buf[0] = (c / 100) % 10;
   buf[1] = (c / 10) % 10;
   buf[2] = c % 10;
   drawtext(fb, buf, 3, count_font, x, y);
}

void
draw_gettables(LevData *ld, int pl,
	       void *fb, int width, int height)
{
   int i;
   for (i = 0; i < ngetts; i++) {
      const Gettable *gt = gett[i];
      int xo = gt->xo, yo = gt->yo;
      int cnt = *getcount(ld, pl, i);
      Texture *gtx = GTEXTURE(i);
      if (gtx == NULL || cnt < 1)
	 continue;
      if (xo < 0)
	 xo += width - gtx->width;
      if (yo < 0)
	 yo += height - gtx->height - (gettmax(ld, pl, i) > 1 ? 16 : 0);
      /* center icon */
      if (gt->flags & GK_XCENTERICON)
	 xo -= gtx->width / 2;
      if (gt->flags & GK_YCENTERICON)
	 yo -= gtx->height / 2;
      /* now draw icon */
      if ((gt->flags & GK_WEPSELECT) && i == PLINFO_SEL(pl, 0))
	 draw_outline(fb, gtx, xo, yo);
      else if ((gt->flags & GK_SPESELECT) && i == PLINFO_SEL(pl, 1))
	 draw_outline(fb, gtx, xo, yo);
      else
	 draw(fb, gtx, xo, yo);
      if (gettmax(ld, pl, i) > 1 && !gt->decay)
	 draw_count(fb, cnt, xo, yo + gtx->height + 3);
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
      for (i = 0; i < ngetts; i++) {
	 const Gettable *gt = gett[i];
	 int *count = getcount(ld, pl, i);
	 if (gt->decay && (*count > 0))
	    *count -= ticks * gt->decay;
	 if (*count < 0)
	    *count = 0;
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
   for (i = 0; i < ngetts; i++)
      *getcount(ld, pl, i) = gett[i]->backpackmax;
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
      for (i = 0; i < ngetts; i++) {
	 const Gettable *gt = gett[i];
	 if (gt->flags & GK_ONEMAPONLY)
	    *getcount(ld, pl, i) = gt->initial;
      }
   }
}

/* return non-zero if we have a key of type <keytype> */
int
gettable_chk_key(const LevData *ld, int pl, int keytype)
{
   int i;
   for (i = 0; i < ngetts; i++)
      if (*getcount(ld, pl, i) > 0
	  && gett[i]->key == keytype)
	 return 1;
   return 0;
}


/* init and reset funcs */

void
init_gettables(void)
{
   int i;
   const void *gettable_data;
   /* if already inited, throw away old info first */
   if (ngetts > 0)
      reset_gettables();
   /* load lump and allocate GetDs */
   count_font = -1;
   gettable_ln = lookup_lump("GETTABLE", NULL, NULL);
   if (!LUMPNUM_OK(gettable_ln))
      return;
   gettable_data = load_lump(gettable_ln);
   ngetts = scan_gettables(gettable_data, get_lump_len(gettable_ln),
			   NULL);
   logprintf(LOG_INFO, 'G', _("init %d gettables"), ngetts);
   gett = (const Gettable **) safe_malloc(ngetts * sizeof(const Gettable *));
   scan_gettables(gettable_data, get_lump_len(gettable_ln),
		  gett);
   gettxt = (Texture ***) safe_calloc(sizeof(Texture **), ngetts);
   /* initialise */
   for (i = 0; i < ngetts; i++) {
      int j;
      char buf[10];
      const Gettable *gt = gett[i];
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
      for (i = 0; i < ngetts; i++)
	 if (gettxt[i])
	    safe_free(gettxt[i]);
      safe_free(gettxt);
   }
   safe_free(gett);
   if (LUMPNUM_OK(gettable_ln))
      free_lump(gettable_ln);
   ngetts = 0;
   gett = NULL;
   gettxt = NULL;
   gettable_ln = BAD_LUMPNUM;
}

// Local Variables:
// c-basic-offset: 3
// End:
