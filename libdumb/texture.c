/* DUMB: A Doom-like 3D game engine.
 *
 * libdumb/texture.c: Textures.
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
#include <string.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadstruct.h"
#include "texture.h"

/* Uncomment this for loads of debugging msgs */
/*#define TXDEBUG */

/* playpal stuff */

static LumpNum ppln;
static const unsigned char *playpal = NULL;

static void
init_playpal(void)
{
   if (playpal == NULL) {
      ppln = getlump("PLAYPAL");
      playpal = (const unsigned char *) load_lump(ppln);
   }
}

static void
reset_playpal(void)
{
   if (playpal != NULL) {
      free_lump(ppln);
      playpal = NULL;
   }
}

void
set_playpal(int idx, SetPalFunc func)
{
   int i = 256;
   const unsigned char *p;
   init_playpal();
   p = playpal + (idx + 1) * 256 * 3;
   while (i > 0) {
      i--;
      p -= 3;
      func(i, p[0], p[1], p[2]);
   }
}

/* pixcvt stuff */

inline unsigned short
pix8topix16(unsigned char p)
{
   const unsigned char *pp = playpal + (int) p * 3;
   LE_int16 _r;
   unsigned short r;
   if (p == 255)
      return 0xffff;
   _r = (pp[2] >> 3) | ((pp[1] >> 2) << 5) | ((pp[0] >> 3) << 11);
   r = *(const short *) &_r;
   /* 0xffff means transparent, so never produce it to mean white */
   if (r == 0xffff)
      r = 0xfffe;
   return r;
}

inline unsigned int
pix8topix32(unsigned char p)
{
   const unsigned char *pp = playpal + (int) p * 3;
   union {
      unsigned int r;
      struct {
	 char a, b, c;
      } i;
   } u;
   if (p == 255)
      return 0xffffffff;
   u.r = 0;
   u.i.a = pp[2];
   u.i.b = pp[1];
   u.i.c = pp[0];
   return u.r;
}

#define Pixel unsigned char
#define ppot ppot8
#define jpot jpot8
#define pixcvt(p) p
#include "pastepic.h"
#undef Pixel
#undef ppot
#undef jpot
#undef pixcvt

#define Pixel unsigned short
#define ppot ppot16
#define jpot jpot16
#define pixcvt pix8topix16
#include "pastepic.h"
#undef Pixel
#undef ppot
#undef jpot
#undef pixcvt

#define Pixel unsigned int
#define ppot ppot32
#define jpot jpot32
#define pixcvt pix8topix32
#include "pastepic.h"
#undef Pixel
#undef ppot
#undef jpot
#undef pixcvt

static void
paste_pict_on_texture(Texture *t, const PictData *p, int x, int y)
{
   switch (t->bpp) {
   case (1):
      ppot8(t, p, x, y);
      break;
   case (2):
      init_playpal();
      ppot16(t, p, x, y);
      break;
   case (4):
      init_playpal();
      ppot32(t, p, x, y);
      break;
   default:
      logprintf(LOG_ERROR, 'T', _("Bad BPP value in paste_pict"));
   }
}
static void
paste_jpatch_on_texture(Texture *t, const AltPictData *p,
			int x, int y)
{
   switch (t->bpp) {
   case (1):
      jpot8(t, p, x, y);
      break;
   case (2):
      init_playpal();
      jpot16(t, p, x, y);
      break;
   case (4):
      init_playpal();
      jpot32(t, p, x, y);
      break;
   default:
      logprintf(LOG_ERROR, 'T', _("Bad BPP value in paste_jpatch"));
   }
}

/*** PNAMES stuff ***/

/* *not* the same format as in the .WAD --- these are null-terminated */
static int npnames = 0;
static char *pnames = NULL;

static void
reset_pnames(void)
{
   if (pnames) {
      npnames = 0;
      safe_free(pnames);
      pnames = NULL;
   }
}

static void
init_pnames(void)
{
   LumpNum ln;
   const char *pn;
   int i;
   if (pnames != NULL)
      return;
   ln = getlump("PNAMES");
   pn = (const char *) load_lump(ln);
   npnames = *(const LE_int32 *) (pn);
   pn += 4;
   pnames = (char *) safe_calloc(npnames, 9);
   for (i = 0; i < npnames; i++)
      strncpy(pnames + 9 * i, pn + 8 * i, 8);
   free_lump(ln);
   logprintf(LOG_DEBUG, 'T', _("npnames=%d (should be %d)"),
	     npnames, (get_lump_len(ln) - 4) / 8);
}
#define lookup_pname(i) (pnames+9*i)

/*** TEXTURE1 stuff ***/

static int nwalltexs = 0;
static LumpNum t1ln = BAD_LUMPNUM, t2ln = BAD_LUMPNUM;
static const TextureTable *texture1 = NULL, *texture2 = NULL;

static void
init_texture_lumps(void)
{
   if (texture1 == NULL) {
      t1ln = lookup_lump("TEXTURE1", NULL, NULL);
      t2ln = lookup_lump("TEXTURE2", NULL, NULL);
      if (t1ln == BAD_LUMPNUM)
	 logprintf(LOG_FATAL, 'T', _("No metatexture lumps in WAD"));
      texture1 = (const TextureTable *) load_lump(t1ln);
      if (t2ln == BAD_LUMPNUM)
	 texture2 = NULL;
      else
	 texture2 = (const TextureTable *) load_lump(t2ln);
      nwalltexs = texture1->UMEMB(hdr).ntxts;
      logprintf(LOG_DEBUG, 'T', _("%s has %d wall textures"),
		"TEXTURE1", (int) (texture1->UMEMB(hdr).ntxts));
      if (texture2) {
	 nwalltexs += texture2->UMEMB(hdr).ntxts;
	 logprintf(LOG_DEBUG, 'T', _("%s has %d wall textures"),
		   "TEXTURE2", (int) (texture2->UMEMB(hdr).ntxts));
      }
   }
}
static void
reset_texture_lumps(void)
{
   if (t1ln != BAD_LUMPNUM)
      free_lump(t1ln);
   if (t2ln != BAD_LUMPNUM)
      free_lump(t2ln);
   t1ln = t2ln = BAD_LUMPNUM;
   texture1 = texture2 = NULL;
   nwalltexs = 0;
}

static const TextureData *
find_tdata(int i)
{
   const TextureTable *tt = texture1;
   if (i >= texture1->UMEMB(hdr).ntxts) {
      tt = texture2;
      i -= texture1->UMEMB(hdr).ntxts;
   }
   return (const TextureData *) (tt->data + (tt->UMEMB(hdr).idx[i]));
}


/*** Some generic Texture helpers ***/

static int
mylog2(int i)
{
   int j = 0;
   while (j < 32 && i > (1 << j))
      j++;
   return j;
}

static void
free_many_textures(Texture *tt, int i)
{
   while (i > 0)
      free_texels(tt + (--i));
}

static Texture *
find_texture(const char *name, Texture *tt, int n)
{
   int i;
   for (i = 0; i < n; i++)
      if (!strncasecmp(tt[i].name, name, 8))
	 return tt + i;
   return NULL;
}


/*** sprites ***/

static int nsprites = 0;
static Texture *sprites = NULL;

static void
guess_sprite_size(Texture *t)
{
   const PictData *pd;
   if (t->width > 0)
      return;
   pd = (const PictData *) load_lump(t->lumpnum);
   if (IS_JPATCH(pd)) {
      t->width = pd->UMEMB(alt).width;
      t->height = pd->UMEMB(alt).height;
      t->log2width = pd->UMEMB(alt).log2width;
      t->log2height = pd->UMEMB(alt).log2height;
   } else {
      t->width = pd->UMEMB(hdr).width;
      t->height = pd->UMEMB(hdr).height;
      t->leftofs = pd->UMEMB(hdr).xoffset;
      t->topofs = pd->UMEMB(hdr).yoffset;
      t->log2width = mylog2(t->width);
      t->log2height = mylog2(t->height);
   }
   release_lump(t->lumpnum);
}

static void
init_sprites(void)
{
   LumpNum ln = 0;
   int topsprite = 0;
   if (sprites)
      return;
   nsprites = count_lumps_between("S_START", "S_END");
   if (nsprites <= 0)
      logprintf(LOG_FATAL, 'T', _("Can't find any sprites in WAD"));
   sprites = (Texture *) safe_calloc(nsprites, sizeof(Texture));
   while (topsprite < nsprites) {
      ln = lumplook(ln, "S_START");
      if (!LUMPNUM_OK(ln))
	 break;
      while (topsprite < nsprites) {
	 Texture *t = sprites + topsprite;
	 t->lumpnum = ln = lumpnext(ln, 0);
	 if (!LUMPNUM_OK(ln))
	    break;
	 get_lump_name(ln, t->name);
	 if (!strcasecmp(t->name, "S_END"))
	    break;
	 if (get_lump_len(ln) < 8) {
#ifdef TXDEBUG
	    logprintf(LOG_DEBUG, 'T', _("too short sprite: %s"), t->name);
#endif
	    continue;
	 }
	 t->opaque = 0;
	 t->type = TT_SPRITE;
	 topsprite++;
      }
   }
   logprintf(LOG_INFO, 'T', _("Init %d sprites"), nsprites = topsprite);
}

static void
reset_sprites(void)
{
   if (sprites) {
      free_many_textures(sprites, nsprites);
      safe_free(sprites);
      sprites = NULL;
      nsprites = 0;
   }
}

static void
load_sprite(Texture *t, int bpp)
{
   const PictData *pd;
   size_t l;
   if (!LUMPNUM_OK(t->lumpnum))
      logfatal('T', _("Bad lumpnum for texture %s in load_sprite()"), t->name);
   pd = (const PictData *) load_lump(t->lumpnum);
   l = bpp << (t->log2width + t->log2height);
   if (IS_JPATCH(pd) && bpp == 1) {
      t->bpp = 1;
      t->texels = (void *) pd->UMEMB(alt).data;
      t->alloced_texels = 0;
   } else {
      t->bpp = bpp;
      t->texels = safe_malloc(l);
      t->alloced_texels = 1;
      memset(t->texels, -1, l);
      if (IS_JPATCH(pd))
	 paste_jpatch_on_texture(t, &(pd->UMEMB(alt)), 0, 0);
      else
	 paste_pict_on_texture(t, pd, 0, 0);
      free_lump(t->lumpnum);
   }
}

static void
free_sprite(Texture *t)
{
   if (t->alloced_texels)
      safe_free(t->texels);
   else
      free_lump(t->lumpnum);
   t->texels = NULL;
   t->alloced_texels = 0;
   t->bpp = 0;
}

Texture *
get_sprite_texture(const char *name)
{
   Texture *t;
   init_sprites();
   t = find_texture(name, sprites, nsprites);
   if (!t)
      logfatal('T', _("Can't find sprite texture %s"), name);
   guess_sprite_size(t);
   return t;
}

Texture *
get_sprite(const char *sp, char ph, char r, char *mptr)
{
   int i;
   Texture *t;
   char mirror = 0;
   for (i = 0, t = sprites; i < nsprites; i++, t++) {
      if (strncmp(t->name, sp, 4))
	 continue;
      if (t->name[4] == ph && t->name[5] == '0')
	 break;
      if (t->name[6] == ph && t->name[7] == '0') {
	 mirror++;
	 break;
      }
      if (t->name[4] == ph && t->name[5] == r)
	 break;
      if (t->name[6] == ph && t->name[7] == r) {
	 mirror++;
	 break;
      }
   }
   if (i >= nsprites)
      return NULL;
   guess_sprite_size(t);
   if (mptr)
      *mptr = mirror;
   return t;
}

/*** flats ***/

static int nflats = 0;
static Texture *flats = NULL;

static void
init_flats(void)
{
   LumpNum ln = 0;
   int topflat = 0;
   if (flats)
      return;
   nflats = count_lumps_between("F_START", "F_END");
   if (nflats <= 0)
      logprintf(LOG_FATAL, 'T', _("Can't find any flats in WAD"));
   flats = (Texture *) safe_calloc(nflats, sizeof(Texture));
   while (topflat < nflats) {
      ln = lumplook(ln, "F_START");
      if (!LUMPNUM_OK(ln))
	 break;
      while (topflat < nflats) {
	 int i;
	 Texture *t = flats + topflat;
	 t->lumpnum = ln = lumpnext(ln, 0);
	 if (!LUMPNUM_OK(ln))
	    break;
	 get_lump_name(ln, t->name);
	 if (!strcasecmp(t->name, "F_END"))
	    break;
	 t->type = TT_FLAT;
	 t->opaque = 1;
	 switch (i = get_lump_len(t->lumpnum)) {
	    /* F_SKY1 */
	 case (4):
	 case (0):
	    break;
	    /* big josh flats */
	 case (128 * 128):
	    t->width = t->height = 128;
	    t->log2width = t->log2height = 7;
	    topflat++;
	    break;
	    /* some heretic flats are this length! why? */
	 case (4160):
	    /* plain doom flat */
	 case (64 * 64):
	    t->width = t->height = 64;
	    t->log2width = t->log2height = 6;
	    topflat++;
	    break;
	    /* none of the above */
	 default:
	    logprintf(LOG_ERROR, 'T', _("funny length (%d) for flat %s"),
		      i, t->name);
	 }
      }
   }
   logprintf(LOG_INFO, 'T', _("Init %d flats"), nflats = topflat);
}

static void
reset_flats(void)
{
   if (flats) {
      free_many_textures(flats, nflats);
      safe_free(flats);
      flats = NULL;
      nflats = 0;
   }
}

static void
load_flat(Texture *t, int bpp)
{
   t->bpp = bpp;
   if (bpp == 1)
      t->texels = (void *) load_lump(t->lumpnum);
   else {
      int i = t->width * t->height;
      const unsigned char *data = (const unsigned char *) load_lump(t->lumpnum);
      t->texels = safe_malloc(i * bpp);
      while (i > 0) {
	 i--;
	 if (bpp == 2)
	    ((unsigned short *) (t->texels))[i] = pix8topix16(data[i]);
	 else if (bpp == 4)
	    ((unsigned int *) (t->texels))[i] = pix8topix32(data[i]);
      }
      free_lump(t->lumpnum);
   }
}

static void
free_flat(Texture *t)
{
   if (t->texels) {
      if (t->bpp == 1)
	 free_lump(t->lumpnum);
      else
	 safe_free(t->texels);
   }
   t->texels = NULL;
   t->bpp = 0;
}

Texture *
get_flat_texture(const char *name)
{
   Texture *t;
   init_flats();
   t = find_texture(name, flats, nflats);
   if (t == NULL)
      logfatal('T', _("Can't find flat texture %s"), name);
   return t;
}

/*** patches ***/

static LumpNum *patlns = NULL;
static const PictData **patlumps = NULL;

static void
init_patches(void)
{
   int i;
   if (patlns)
      return;
   init_pnames();
   patlns = (LumpNum *) safe_calloc(npnames, sizeof(LumpNum));
   /* PictData can be a 1-byte pseudo-union, whereas PictHeader is the
    * real thing.  */
   patlumps = (const PictData **)
      safe_calloc(npnames, sizeof(const PictHeader *));
   for (i = 0; i < npnames; i++)
      patlns[i] = lookup_lump(lookup_pname(i), "P_START", "P_END");
}

static void
reset_patches(void)
{
   int i;
   /*
      NB: though it is OK to free id-style patches at any time,
      J1 patches need to stay resident, as they may be the sole
      patch of a 1bpp texture, in which case the raw patch data will
      be referenced by the texture.
      Earlier versions of texture.c had this function as a non-static
      so that patches could be freed as part of a periodic garbage
      collection.  This is no longer safe.
    */
   if (!patlns)
      return;
   for (i = 0; i < npnames; i++)
      if (LUMPNUM_OK(patlns[i]))
	 free_lump(patlns[i]);
   safe_free(patlns);
   safe_free(patlumps);
   patlns = NULL;
   patlumps = NULL;
   reset_pnames();
}

static const PictData *
get_patch_data(int num)
{
   if (patlumps[num] == NULL) {
      if (!LUMPNUM_OK(patlns[num]))
	 logprintf(LOG_FATAL, 'T', _("Patchnum %d (%s) doesn't exist"),
		   num, pnames + num * 9);
      else {
#ifdef TXDEBUG
	 logprintf(LOG_DEBUG, 'T', _("loading patchnum %d (%s)"),
		   num, pnames + num * 9);
#endif
	 patlumps[num] = (const PictData *) load_lump(patlns[num]);
      }
   }
   return patlumps[num];
}


/*** walls ***/

static Texture *walltexs = NULL;

static void
init_walltexs(void)
{
   int i;
   init_texture_lumps();
   init_patches();
   if (walltexs)
      return;
#ifdef TXDEBUG
   logprintf(LOG_DEBUG, 'T', _("init_walltexs(): %d textures"), nwalltexs);
#endif
   walltexs = (Texture *) safe_calloc(sizeof(Texture), nwalltexs);
   for (i = 0; i < nwalltexs; i++) {
      Texture *t = walltexs + i;
      const TextureData *td = find_tdata(i);
      strncpy(t->name, td->name, 8);
      t->type = TT_WALL;
      t->opaque = 1;		/* a lie, but good enough for now */
      t->alloced_texels = 0;
      t->width = td->dx;
      t->height = td->dy;
      t->log2width = mylog2(td->dx);
      t->log2height = mylog2(td->dy);
   }
}
static void
reset_walltexs(void)
{
   if (walltexs) {
      free_many_textures(walltexs, nwalltexs);
      safe_free(walltexs);
   }
}

Texture *
get_wall_texture(const char *name)
{
   Texture *t;
   if (*name == '&')
      return get_flat_texture(name + 1);
   init_walltexs();
   t = find_texture(name, walltexs, nwalltexs);
   if (t == NULL)
      logprintf(LOG_FATAL, 'T', _("Can't find wall texture %s"), name);
   return t;
}

static void
load_wtex(Texture *t, int bpp)
{
   const TextureData *td;
   int i;
   init_walltexs();
   t->bpp = bpp;
   td = find_tdata(t - walltexs);
   /* if this is a one-patch texture, try this kludge */
   if (((long) td) % 4 != 0) {
      logprintf(LOG_DEBUG, 'T',
		_("wtex \"%s\" is unaligned with offset %ld!"),
		td->name, ((long) td) % 4);
   } else {
      if (bpp == 1
	  && td->npatches == 1
	  && td->patch[0].x == 0
	  && td->patch[0].y == 0) {
	 const PictData *p = get_patch_data(td->patch[0].pnum);
	 if (IS_JPATCH(p) && p->UMEMB(alt).log2height == t->log2height) {
	    t->texels = (void *) p->UMEMB(alt).data;
#ifdef TXDEBUG
	    logprintf(LOG_DEBUG, 'T',
		      _("using one-patch kludge for %s"), td->name);
#endif
	    return;
	 }
      }
   }
#ifdef TXDEBUG
   logprintf(LOG_DEBUG, 'T', _("load_walltex %s at %d bpp"),
	     t->name, bpp);
   logprintf(LOG_DEBUG, 'T', "l2w=%d l2h=%d",
	     t->log2width, t->log2height);
#endif
   t->texels = safe_malloc(i = (bpp << (t->log2width + t->log2height)));
   t->alloced_texels = 1;
   memset(t->texels, -1, i);
#ifdef TXDEBUG
   logprintf(LOG_DEBUG, 'T', _("%s: %d patches"),
	     t->name, (int) (td->npatches));
#endif
   for (i = 0; i < td->npatches; i++) {
      const PictData *p = get_patch_data(td->patch[i].pnum);
      if (IS_JPATCH(p))
	 paste_jpatch_on_texture(t, &(p->UMEMB(alt)),
				 td->patch[i].x, td->patch[i].y);
      else if (p->UMEMB(hdr).height < 255)
	 paste_pict_on_texture(t, p, td->patch[i].x, td->patch[i].y);
      else
	 logfatal('T', _("Can't make sense of patch #%d, texture %s"),
		  (int) (td->patch[i].pnum), td->name);
   }
#ifdef TXDEBUG
   logprintf(LOG_DEBUG, 'T', _("%s: loaded"), t->name);
#endif
}

static void
free_wtex(Texture *t)
{
   if (t->alloced_texels)
      safe_free(t->texels);
   t->texels = NULL;
   t->alloced_texels = 0;
   t->bpp = 0;
}


/*** FONTS ***/

#define MAXFONTS 16
#define MAXFONTNAME 16

static Texture *fonts[MAXFONTS];
static char fontname[MAXFONTS * MAXFONTNAME];
static int nfontents[MAXFONTS], fontofs[MAXFONTS], nfonts = 0;

int
init_font(const char *format, int nchars, int ofs)
{
   int i;
   for (i = 0; i < nfonts; i++) {
      if (!strncmp(fontname + i * MAXFONTNAME, format, MAXFONTNAME)) {
	 logprintf(LOG_DEBUG, 'T', _("found font %s already loaded"),
		   format);
	 return i;
      }
   }
   if (nfonts >= MAXFONTS)
      logfatal('T', _("Too many fonts"));
   fonts[nfonts] = (Texture *) safe_calloc(nfontents[nfonts] = nchars,
					   sizeof(Texture));
   fontofs[nfonts] = ofs;
   strncpy(fontname + nfonts * MAXFONTNAME, format, MAXFONTNAME);
   /* OK, now set up each character to be loaded when needed */
   logprintf(LOG_INFO, 'T', _("Loading font #%d (%s)"), nfonts, format);
   for (i = 0; i < nchars; i++) {
      Texture *t = fonts[nfonts] + i;
      t->opaque = 0;
      t->type = TT_FONT;
      sprintf(t->name, format, i);
      t->lumpnum = lookup_lump(t->name, NULL, NULL);
      if (LUMPNUM_OK(t->lumpnum))
	 guess_sprite_size(t);
   }
   /* done */
   return nfonts++;
}

static void
reset_fonts(void)
{
   while (nfonts > 0) {
      nfonts--;
      free_many_textures(fonts[nfonts], nfontents[nfonts]);
      safe_free(fonts[nfonts]);
   }
}

Texture *
get_font_texture(int fontnum, unsigned char _ch)
{
   int ch = _ch - fontofs[fontnum];
   if (fontnum < 0 || fontnum >= nfonts)
      logfatal('T', _("Request for bad fontnum (%d)"), fontnum);
   if (ch >= nfontents[fontnum])
      logfatal('T', _("Request for bad font char (%d:%d)"), fontnum, ch);
   return fonts[fontnum] + ch;
}


/*** MISCELLANEOUS TEXTURES ***/

/* hmm.... it's hard to know how many we'll need, but at the same time,
   textures are mostly passed about as pointers, so... */
#define MAXMISC 256
static Texture *misc = NULL;
static int nmisc = 0;

static void
init_misc(void)
{
   if (misc)
      return;
   misc = (Texture *) safe_vcalloc(sizeof(Texture) * MAXMISC);
   nmisc = 0;
}

static void
reset_misc(void)
{
   free_many_textures(misc, nmisc);
   safe_vfree(misc, sizeof(Texture) * MAXMISC);
   misc = NULL;
   nmisc = 0;
}

Texture *
get_misc_texture(const char *name)
{
   Texture *t;
   if (!misc)
      init_misc();
   t = find_texture(name, misc, nmisc);
   if (t)
      return t;
   if (nmisc >= MAXMISC)
      logfatal('T', _("too many misc textures"));
   t = misc + nmisc++;
   t->type = TT_MISC;
   t->opaque = 0;
   strncpy(t->name, name, 8);
   t->lumpnum = safe_lookup_lump(name, NULL, NULL, LOG_FATAL);
   guess_sprite_size(t);
   return t;
}

/*** OTHER STUFF ***/

int
get_texture_num(TextureType tt, const char *name)
{
   switch (tt) {
   case (TT_FLAT):
      return find_texture(name, flats, nflats) - flats;
      break;
   case (TT_WALL):
      return find_texture(name, walltexs, nwalltexs) - walltexs;
      break;
   case (TT_SPRITE):
      return find_texture(name, sprites, nsprites) - sprites;
      break;
   default:
      logprintf(LOG_ERROR, 'T',
		_("get_texture_num: unsupported texture type (%d)"),
		(int) tt);
   }
   return 0;
}

Texture *
get_texture_bynum(TextureType tt, int num)
{
   switch (tt) {
   case (TT_FLAT):
      return flats + num;
   case (TT_WALL):
      return walltexs + num;
   case (TT_SPRITE):
      guess_sprite_size(sprites + num);
      return sprites + num;
   default:
      logprintf(LOG_ERROR, 'T',
		_("get_texture_bynum: unsupported texture type (%d)"),
		(int) tt);
   }
   return NULL;
}

int
count_textures(TextureType tt)
{
   switch (tt) {
   case (TT_FLAT):
      return nflats;
   case (TT_WALL):
      return nwalltexs;
   case (TT_SPRITE):
      return nsprites;
   case (TT_PATCH):
      return npnames;
   default:
      logprintf(LOG_ERROR, 'T',
		_("count_textures: unsupported texture type (%d)"),
		(int) tt);
   }
   return 0;
}

void
init_textures(void)
{
   init_playpal();
   init_flats();
   init_sprites();
   init_walltexs();
#ifdef TXDEBUG
   logprintf(LOG_DEBUG, 'T', _("init_textures(): done"));
#endif
}

void
reset_textures(void)
{
   reset_flats();
   reset_sprites();
   reset_walltexs();
   reset_patches();
   reset_fonts();
   reset_misc();
   reset_texture_lumps();
   reset_pnames();
   reset_playpal();
}

void
free_texels(Texture *t)
{
   if (t->texels == NULL)
      return;
   switch (t->type) {
   case (TT_FLAT):
      free_flat(t);
      break;
   case (TT_WALL):
      free_wtex(t);
      break;
   case (TT_MISC):
   case (TT_FONT):
   case (TT_SPRITE):
      free_sprite(t);
      break;
   default:
      logprintf(LOG_FATAL, 'T',
		_("Strange texture type (0x%x) in free_texels"),
		t->type);
   }
}

void
load_texels(Texture *t, int bpp)
{
   /* some error checking */
   if (bpp != 1 && bpp != 2 && bpp != 4)
      logprintf(LOG_FATAL, 'T',
		_("Strange BPP value (%d) in load_texels"),
		bpp);
   if (t->texels && t->bpp == bpp)
      return;
   if (t->texels)
      free_texels(t);
   /* now go do it */
   switch (t->type) {
   case (TT_FLAT):
      load_flat(t, bpp);
      break;
   case (TT_WALL):
      load_wtex(t, bpp);
      break;
   case (TT_MISC):
   case (TT_FONT):
   case (TT_SPRITE):
      load_sprite(t, bpp);
      break;
   default:
      logprintf(LOG_FATAL, 'T',
		_("Strange texture type (0x%x) in load_texels"),
		t->type);
   }
   /* find average texel */
   switch (bpp) {
   case (1):
      t->avg_texel = *(unsigned char *) (t->texels);
      break;
   case (2):
      t->avg_texel = *(unsigned short *) (t->texels);
      break;
   case (4):
      t->avg_texel = *(unsigned int *) (t->texels);
      break;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
