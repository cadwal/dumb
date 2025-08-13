/* DUMB: A Doom-like 3D game engine.
 *
 * tool/mkpnames.c: Compile a pnames script to PNAMES and TEXTURE1 lumps.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"

#include "libdumbutil/endiantypes.h"
#include "libdumbutil/align.h"
#include "libdumbwad/wadstruct.h"

FILE *fin, *fpn, *ft;
char *fin_name;
int line;

PNameTable *pnames;
TextureData **td;
int ntexs;

int debug = 0;

int
get_pnum(const char *s)
{
   int i;
   for (i = 0; i < pnames->num; i++)
      if (!strncasecmp(s, pnames->name + 8 * i, 8))
	 return i;
   /* if we made it to here, pname wasn't found */
   pnames->num += 1;
   strncpy(pnames->name + 8 * i, s, 8);
   return i;
}

#define TDSIZE(t) ((t)->npatches*10+22)

void
add_texture(const char *s, int x, int y)
{
   if (debug)
      printf("add_texture(%s,%d,%d)\n", s, x, y);
   /* shrink last texture data */
   if (ntexs > 0)
      td[ntexs - 1] = realloc(td[ntexs - 1], TDSIZE(td[ntexs - 1]));
   /* make a new texture */
   td[ntexs] = (TextureData *) calloc(1, sizeof(TextureData));
   strncpy(td[ntexs]->name, s, 8);
   td[ntexs]->dx = x;
   td[ntexs]->dy = y;
   ntexs++;
}

void
add_patch(const char *s, int x, int y)
{
   TextureData *t = td[ntexs - 1];
   if (debug)
      printf("add_patch(%s,%d,%d)\n", s, x, y);
   if (ntexs <= 0) {
      fprintf(stderr, _("%s:%d: patch with no texture (ignored)\n"),
	      fin_name, line);
      return;
   }
   t->patch[t->npatches].x = x;
   t->patch[t->npatches].y = y;
   t->patch[t->npatches].pnum = get_pnum(s);
   t->npatches += 1;
}

/* return 1 if there's a problem */
int
eatws(FILE *f)
{
   int c;
   do {
      c = getc(f);
      if (c == EOF)
	 return 1;
   } while (c == ' ');
   ungetc(c, f);
   return 0;
}

/* return 1 if there's more to come, 0 if done */
int
scan(char **name, char *sep, int *x, int *y)
{
   static char buf[16];
   int i = 0, ch;
   *name = NULL;
   /* scan for name */
   if (eatws(fin))
      return 0;
   while (i < 8) {
      buf[i] = ch = getc(fin);
      if (ch == EOF)
	 return 0;
      if (ch == ' ')
	 break;
      if (ch == '\n')
	 return 1;
      if (ch == '#') {
	 while (!feof(fin) && getc(fin) != '\n');
	 return 1;
      }
      i++;
   }
   buf[i] = '\0';
   /* scan for seperator */
   if (eatws(fin))
      return 0;
   *sep = getc(fin);
   /* scan for coords */
   if (eatws(fin))
      return 0;
   fscanf(fin, "%d %d", x, y);
   /* scan for newline */
   while (!feof(fin) && getc(fin) != '\n');
   *name = buf;
   return 1;
}

int
main(int argc, char **argv)
{
   int i, o;
   LE_int32 lebuf;
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   if (argc < 4) {
      printf(_("Usage: mkpnames <source> <pnames.lump> <textures.lump> [option]"
	       "\n\n"
	       "mkpnames is used to compile metatexture information\n"
	       "into the PNAMES and TEXTURE1 lumps used by DUMB.\n"
	       "The only option is -d, which prints debugging info.\n\n"));
      return 2;
   }
   if (argc > 4)
      debug = 1;
   /* allocate some huge arrays */
   td = malloc(8 * 65536);
   ntexs = 0;
   pnames = malloc(8 * 65536);
   pnames->num = 0;
   /* go! */
   fin_name = argv[1];
   fin = fopen(fin_name, "r");
   fpn = fopen(argv[2], "wb");
   ft = fopen(argv[3], "wb");
   /* scan the input */
   if (fin && fpn && ft) {
      char *name, sep;
      int x, y;
      line = 1;
      while (scan(&name, &sep, &x, &y)) {
	 if (name == NULL || *name == 0)
	    continue;
	 if (sep == ':')
	    add_texture(name, x, y);
	 else if (sep == '@')
	    add_patch(name, x, y);
	 else
	    fprintf(stderr, _("%s:%d: syntax error (ignoring)\n"),
		    fin_name, line);
	 line++;
      }
   } else {
      printf(_("mkpnames: couldn't open the files specified\n"));
      exit(EXIT_FAILURE);
   }
   /* squirt out lumps */
   fwrite(pnames, 1, sizeof(int) + 8 * pnames->num, fpn);
   lebuf = ntexs;
   fwrite(&lebuf, sizeof(int), 1, ft);
   o = (ntexs + 1) * sizeof(int);
   for (i = 0; i < ntexs; i++) {
      o = ALIGN(o, ALIGN_TEXTURE1);
      lebuf = o;
      fwrite(&lebuf, sizeof(int), 1, ft);
      o += TDSIZE(td[i]);
   }
   for (i = 0; i < ntexs; i++) {
      fseek(ft, ALIGN(ftell(ft), ALIGN_TEXTURE1), SEEK_SET);
      fwrite(td[i], TDSIZE(td[i]), 1, ft);
   }

   printf(_("mkpnames: %ld pnames, %d textures\n"), (long) pnames->num, ntexs);

   /* clean up */
   fclose(ft);
   fclose(fpn);
   fclose(fin);
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
