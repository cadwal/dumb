/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/licomp.c: Level compiler.
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
#include "libdumb/levinfostruct.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "licomp.h"

typedef struct {
   LevInfo l;
   char next[10];		/* FIXME: why 10? */
   char secret[10];
} LevInfoRec;

static int nlinfos = 0, maxlinfos = 0;
static LevInfoRec *linfos;

static int lookup_linfo(const char *n);


void
init_licomp(void)
{
   maxlinfos = ALLOC_BLK;
   linfos = (LevInfoRec *) safe_malloc(maxlinfos * sizeof(LevInfoRec));
}

void
licomp(void)
{
   const char *s;
   LevInfoRec *li;
   /* make new li */
   if (nlinfos >= maxlinfos - 1) {
      maxlinfos += ALLOC_BLK;
      linfos = (LevInfoRec *)
	 safe_realloc(linfos, sizeof(LevInfoRec) * maxlinfos);
   }
   li = linfos + (nlinfos++);
   memset(li, 0, sizeof(LevInfoRec));
   li->l.secret = -1;
   li->l.next = -1;
   /* one parameter, name */
   parm_str(li->l.name, 8+1);
   /* now parse other keywords */
   while (1) {
      s = next_token();
      if (s == NULL)
	 return;
      else if (*s == '\n')
	 ;
      else if (!strcasecmp(s, "Music"))
	 parm_str(li->l.music, 9);
      else if (!strcasecmp(s, "Sky"))
	 parm_str(li->l.sky, 9);
      else if (!strcasecmp(s, "StartGame"))
	 li->l.flags |= LI_START;
      else if (!strcasecmp(s, "EndGame"))
	 li->l.flags |= LI_END;
      else if (!strcasecmp(s, "Secret"))
	 parm_str(li->secret, 9);
      else if (!strcasecmp(s, "Next"))
	 parm_str(li->next, 9);
      else
	 break;
   }
   unget_token();
}

void
wrlinfos(WADWR *wout)
{
   int i;
   printf(_("%5d levinfos\n"), nlinfos);
   wadwr_lump(wout, "LEVINFO");
   for (i = 0; i < nlinfos; i++) {
      if (linfos[i].next[0])
	 linfos[i].l.next = linfos[i].l.secret = lookup_linfo(linfos[i].next);
      if (linfos[i].secret[0])
	 linfos[i].l.secret = lookup_linfo(linfos[i].secret);
      wadwr_write(wout, &linfos[i].l, sizeof(LevInfo));
   }
}


static int
lookup_linfo(const char *n)
{
   int i;
   LevInfoRec *li;
   for (i = 0, li = linfos; i < nlinfos; i++, li++)
      if (!strcasecmp(li->l.name, n))
	 return i;
   printf(_("unrecognised levelname: %s\n"), n);
   return -1;
}

// Local Variables:
// c-basic-offset: 3
// End:
