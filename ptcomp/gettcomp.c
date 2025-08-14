/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/gettcomp.c: Gettable compiler.
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
#include "libdumb/gettableinwad.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "gettcomp.h"
#include "protocomp.h"
#include "soundcomp.h"

typedef struct {
   Gettable g;
   char *name;
} GettableRec;

static int ngetts = 0, maxgetts = 0;
static GettableRec *getts;

static void add_gets(GettableRec *p, int gtid, int change, int maximum);


void
init_gettcomp(void)
{
   maxgetts = ALLOC_BLK;
   getts = (GettableRec *) safe_malloc(maxgetts * sizeof(GettableRec));
}

void
gettcomp(void)
{
   const char *s;
   GettableRec *p;
   /* make new gettable */
   if (ngetts >= maxgetts - 1) {
      maxgetts += ALLOC_BLK;
      getts = (GettableRec *) safe_realloc(getts,
					   sizeof(GettableRec) * maxgetts);
   }
   p = getts + (ngetts++);
   memset(p, 0, sizeof(GettableRec));
   p->g.bogotype = -1;
   p->g.powered_bogotype = -1;
   p->g.usesound = -1;
   /* Gettable -!- NAME */
   p->name = safe_strdup(parm_name(_("Name expected after Gettable")));
   /* now the info */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "XCenter"))
	 p->g.flags |= GK_XCENTERICON;
      else if (!strcasecmp(s, "YCenter"))
	 p->g.flags |= GK_YCENTERICON;
      else if (!strcasecmp(s, "WepSel"))
	 p->g.flags |= GK_WEPSELECT;
      else if (!strcasecmp(s, "SpeSel"))
	 p->g.flags |= GK_SPESELECT;
      else if (!strcasecmp(s, "OneMapOnly"))
	 p->g.flags |= GK_ONEMAPONLY;
      else if (!strcasecmp(s, "KeyType"))
	 p->g.key = parm_int();
      else if (!strcasecmp(s, "Decay"))
	 p->g.decay = parm_int();
      else if (!strcasecmp(s, "Timing"))
	 p->g.timing = parm_time();
      else if (!strcasecmp(s, "UseSound"))
	 p->g.usesound = parm_sound();
      else if (!strcasecmp(s, "Special"))
	 p->g.special = parm_uint();
      else if (!strcasecmp(s, "DefaultMaximum")) {
	 p->g.defaultmax = p->g.backpackmax = parm_uint();
	 if (parm_keyword_opt("WithBackpack"))
	    p->g.backpackmax = parm_uint();
      } else if (!strcasecmp(s, "Initial"))
	 p->g.initial = parm_uint();
      else if (!strcasecmp(s, "Ammo")) {
	 int gtid = parm_gett();
	 int change = -parm_int();
	 add_gets(p, gtid, change, 0);
      } else if (!strcasecmp(s, "Gets")) {
	 int gtid = parm_gett();
	 int change = parm_int();
	 int maximum = 0;
	 if (parm_keyword_opt("Maximum"))
	    maximum = parm_uint();
	 add_gets(p, gtid, change, maximum);
      } else if (!strcasecmp(s, "Bogotype"))
	 p->g.bogotype = parm_proto();
      else if (!strcasecmp(s, "PoweredBogotype"))
	 p->g.powered_bogotype = parm_proto();
      else if (!strcasecmp(s, "Icon"))
	 parm_str(p->g.iconname, 10);
      else if (!strcasecmp(s, "Anim"))
	 p->g.iconanim = parm_ch();
      else if (!strcasecmp(s, "RevAnim")) {
	 p->g.iconanim = parm_ch();
	 p->g.flags |= GK_REVANIM;
      } else if (!strcasecmp(s, "IconPos")) {
	 p->g.xo = parm_int();
	 p->g.yo = parm_int();
      } else if (!strcasecmp(s, "WeaponNumber"))
	 p->g.weaponnumber = parm_uint();
      else if (!strcasecmp(s, "ReplaceWeapon"))
	 p->g.replaceweapon = parm_gett();
      else
	 break;
   } /* while(1) */
   unget_token();

   /* If PoweredBogotype isn't given, it defaults to Bogotype.  */
   if (p->g.powered_bogotype == -1)
      p->g.powered_bogotype = p->g.bogotype;
}

static void
add_gets(GettableRec *p, int gtid, int change, int maximum)
{
   Gets *getsp;
   p->g.gets = (Gets *)
      safe_realloc(p->g.gets, ++(p->g.ngets) * sizeof(Gets));
   getsp = &p->g.gets[p->g.ngets-1];
   getsp->gtid = gtid;
   getsp->change = change;
   getsp->maximum = maximum;
}

void
wrgetts(WADWR *wout)
{
   int i;
   printf(_("%5d gettables"), ngetts);
   fflush(stdout);
   wadwr_lump(wout, "GETTABLE");
   {
      void *block;
      size_t blocklen;
      block = begin_gettablesinwad(&blocklen);
      wadwr_write(wout, block, blocklen);
      safe_free(block);
   }
   for (i = 0; i < ngetts; i++) {
      void *block;
      size_t blocklen;
      block = encode_gettableinwad(&getts[i].g, &blocklen);
      wadwr_write(wout, block, blocklen);
      safe_free(block);
   }
   putchar('\n');
}

int
parm_gett(void)
{
   int i;
   const char *s = parm_name(_("Gettable name parameter expected"));
   /* The Gettable clause doesn't let you specify the ID, so the
      following wouldn't make sense:

      if (isdigit(*s))
         return atoi(s); */
   for (i = 0; i < ngetts; i++)
      if (!strcasecmp(getts[i].name, s))
	 return i;
   err(_("Gettable name `%s' unrecognised"), s);
   return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
