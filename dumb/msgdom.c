/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/msgdom.c: MessageDomain handling via MSGDOM lump.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111, USA.
 */

#include <config.h>

#include <assert.h>
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "msgdom.h"

static char *msgdom = NULL;

void
init_msgdom(void)
{
   LumpNum msgdom_ln = lookup_lump("MSGDOM", NULL, NULL);
   assert(msgdom == NULL);
   if (LUMPNUM_OK(msgdom_ln)) {
      size_t len = get_lump_len(msgdom_ln);
      const void *data = load_lump(msgdom_ln);
      msgdom = (char *) safe_malloc(len + 1);
      memcpy(msgdom, data, len);
      msgdom[len] = '\0';
   } else {
      logprintf(LOG_WARNING, '_', _("warning: MessageDomain not set"));
      msgdom = safe_strdup("dumb-utf8");
   }
#ifdef ENABLE_NLS
   bindtextdomain(msgdom, LOCALEDIR);
#endif
}

void
reset_msgdom(void)
{
   assert(msgdom != NULL);
   safe_free(msgdom);
   msgdom = NULL;
}

const char *
get_msgdom(void)
{
   assert(msgdom != NULL);	/* init_msgdom() must have been called */
   return msgdom;
}

// Local Variables:
// c-basic-offset: 3
// End:
