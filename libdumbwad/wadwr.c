/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbwad/wadwr.c: Writing WAD files.
 * Copyright (C) 1998-1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
 * Copyright (C) 1998 by Josh Parsons <josh@schlick.anu.edu.au>
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "libdumbutil/align.h"
#include "wadwr.h"

#define ALLOCBLK 256		/* how many dirents to alloc at a time */

#define sigcpy(x,y) memcpy(x,y,4)

static long int aligned_pos(WADWR *wr);

WADWR *
wadwr_open(const char *fname, char wadtype)
{
   WADWR *w;
   w = (WADWR *) safe_calloc(1, sizeof(WADWR));
   w->fname = safe_strdup(fname);
   w->type = tolower(wadtype);
   if (w->type == 'd')
      w->f = NULL;
   else {
      w->f = fopen(fname, "wb");
      if (w->f == NULL) {
	 logprintf(LOG_ERROR, 'W', _("%s: can't open: %s"),
		   fname, strerror(errno));
	 free(w->fname);
	 free(w);
	 return NULL;
      }
      if (w->type == 'i')
	 sigcpy(w->hdr.sig, "IWAD");
      else
	 sigcpy(w->hdr.sig, "PWAD");
      w->maxdir = ALLOCBLK;
      w->dir = (WadDirEntry *) malloc(w->maxdir * sizeof(WadDirEntry));
      if (fwrite(&w->hdr, sizeof(WadHeader), 1, w->f) != 1) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', _("%s: short write: %s"),
		   w->fname, strerror(errno));
      }
   }
   return w;
}

int
wadwr_close(WADWR *w)
{
   if (w->type != 'd') {
      w->hdr.diroffset = aligned_pos(w);
      if (fwrite(w->dir, sizeof(WadDirEntry), w->hdr.nlumps, w->f)
	  != w->hdr.nlumps) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', _("%s: short write: %s"),
		   w->fname, strerror(errno));
      }
      if (fseek(w->f, 0, SEEK_SET) != 0) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', _("%s: can't rewind: %s"),
		   w->fname, strerror(errno));
      }
      if (fwrite(&w->hdr, sizeof(WadHeader), 1, w->f) != 1) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', _("%s: short write: %s"),
		   w->fname, strerror(errno));
      }
   }
   if (w->f) {
      if (fclose(w->f) != 0) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', "%s: %s", w->fname, strerror(errno));
      }
   }
   if (w->dir)
      free(w->dir);
   free(w->fname);

   {
      int error_flag = w->error_flag;
      free(w);
      return error_flag;
   }
}

void
wadwr_lump(WADWR *w, const char *lumpname)
{
   if (w->type == 'd') {
      char buf[256];
      if (w->f != NULL) {
	 if (fclose(w->f) != 0) {
	    w->error_flag = 1;
	    logprintf(LOG_ERROR, 'W', "%s: %s", w->fname, strerror(errno));
	 }
      }
      strcpy(buf, w->fname);
      if (buf[strlen(buf) - 1] != '/')
	 strcat(buf, "/");
      strcat(buf, lumpname);
      if (!strstr(buf, ".lump"))
	 strcat(buf, ".lump");
      w->f = fopen(buf, "wb");
      if (w->f == NULL) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', _("%s: can't open: %s"),
		   buf, strerror(errno));
      }
   } else {			/* not debug */
      if (w->hdr.nlumps >= w->maxdir) {
	 w->maxdir += ALLOCBLK;
	 w->dir = (WadDirEntry *) realloc(w->dir,
					  w->maxdir * sizeof(WadDirEntry));
      }
      w->current = w->dir + w->hdr.nlumps;
      w->hdr.nlumps++;
      memset(w->current, 0, sizeof(WadDirEntry));
      strncpy(w->current->name, lumpname, 8);
      w->current->offset = aligned_pos(w);
   }
}

void
wadwr_write(WADWR *w, const void *lump, size_t len)
{
   if (w->type != 'd') {
      if (w->current == NULL)
	 return;
      w->current->size += len;
   }
   if (w->f != NULL) {
      /* This used to do fwrite(lump, len, 1, w->f) but that gave
       * false alarms about short writes when len was zero.  */
      /* FIXME: Does a short write set errno?  */
      if (fwrite(lump, 1, len, w->f) != len) {
	 w->error_flag = 1;
	 logprintf(LOG_ERROR, 'W', _("%s: short write: %s"),
		   w->fname, strerror(errno));
      }
   }
}

static long int
aligned_pos(WADWR *wr)
{
   long int pos = ftell(wr->f);
   if (pos == -1)
      logfatal('W', _("%s: can't get position: %s"),
	       wr->fname, strerror(errno));
#if ALIGN_LUMP>1
   if (!IS_ALIGNED(pos, ALIGN_LUMP)) {
      pos = ALIGN(pos, ALIGN_LUMP);
      if (fseek(wr->f, pos, SEEK_SET) != 0)
	 logfatal('W', _("%s: can't align: %s"),
		  wr->fname, strerror(errno));
   }
#endif /* ALIGN_LUMP>1 */
   return pos;
}

// Local Variables:
// c-basic-offset: 3
// End:
