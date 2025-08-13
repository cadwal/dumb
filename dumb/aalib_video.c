/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/aalib_video.c: AA-lib video driver.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#include <aalib.h>

#include "libdumbutil/dumb-nls.h"

static void set_aa_font(struct aa_hardware_params *p,
			const char *fontname);

#include "libdumbutil/log.h"
#include "video.h"

#define ALLOW_FASTRENDER

static const ConfEnum dither_choices[] =
{
   {"none", AA_NONE},
   {"error-distribution", AA_ERRORDISTRIB},
   {"floyd-steinberg", AA_FLOYD_S},
   CONFENUM_END
};

ConfItem video_conf[] =
{
#ifdef ALLOW_FASTRENDER
   CONFB("fastrender", NULL, 0, N_("very fast (but not as perfect)")),
#else
   CONFB("fastrender", NULL, 0, N_("<disabled at compile time>")),
#endif
   CONFI("brightness", NULL, 0, N_("brightness (0-255)"), 96),
   CONFI("contrast", NULL, 0, N_("contrast (0-255)"), 96),
   /* TODO: make configuration system support floating point directly */
   CONFI("gamma", NULL, 0, N_("gamma (1000 corresponds to 1.0)"), 1000),
   CONFI("dimmul", NULL, 0, N_("multiply factor for dim color (ditto)"), 5300),
   CONFI("boldmul", NULL, 0, N_("multiply factor for bold color (ditto)"), 2700),
   CONFE("dither", NULL, 0, N_("dithering type"), AA_FLOYD_S, dither_choices),
   CONFI("randomval", NULL, 0, N_("range of random value to add to each pixel (0-inf)"), 0),
   CONFB("inverse", NULL, 0, N_("invert the whole screen")),
   CONFS("font", NULL, 0, N_("name of the font you use"), NULL, 10),
   CONFNB("attr-normal", NULL, 0, N_("use the normal attribute")),
   CONFNB("attr-bold", NULL, 0, N_("use the bold (double bright) attribute")),
   CONFB("attr-boldfont", NULL, 0, N_("use the boldfont attribute")),
   CONFB("attr-dim", NULL, 0, N_("use the dim (half bright) attribute")),
   CONFB("attr-reverse", NULL, 0, N_("use the reverse attribute")),
   CONFB("font-extended", NULL, 0, N_("use all 256 characters")),
   CONFB("font-eight", NULL, 0, N_("use eight-bit ASCII")),
   CONFITEM_END
};
#define cnf_fastrender    (video_conf[0].intval)
#define cnf_brightness    (video_conf[1].intval)
#define cnf_contrast      (video_conf[2].intval)
#define cnf_gamma         (video_conf[3].intval)
#define cnf_dimmul        (video_conf[4].intval)
#define cnf_boldmul       (video_conf[5].intval)
#define cnf_dither        (video_conf[6].intval)
#define cnf_randomval     (video_conf[7].intval)
#define cnf_inverse       (video_conf[8].intval)
#define cnf_font          (video_conf[9].strval)
#define cnf_attr_normal   (video_conf[10].intval)
#define cnf_attr_bold     (video_conf[11].intval)
#define cnf_attr_boldfont (video_conf[12].intval)
#define cnf_attr_dim      (video_conf[13].intval)
#define cnf_attr_reverse  (video_conf[14].intval)
#define cnf_font_extended (video_conf[15].intval)
#define cnf_font_eight    (video_conf[16].intval)

aa_context *aa_ctxt = NULL;
#define aa aa_ctxt

#ifdef ALLOW_FASTRENDER
static unsigned char graypal[256];
#endif
static aa_palette aapal;
static aa_renderparams aarp;

void
video_preinit(void)
{
}

void
init_video(int *width, int *height, int *bpp, int *real_width)
{
   if (*width > 0 && *width < 256 && *height > 0 && *height < 256) {
      aa_defparams.width = *width;
      aa_defparams.height = *height;
   }
   /* Parse AA-lib environment options from $AAOPTS */
   aa_parseoptions(NULL, NULL, NULL, NULL);
   /* Overwrite them with our values :) */
   aa_defparams.dimmul = cnf_dimmul / 1000.0;
   aa_defparams.boldmul = cnf_boldmul / 1000.0;
   if (cnf_font)
      set_aa_font(&aa_defparams, cnf_font);
   if (cnf_attr_normal)
      aa_defparams.supported |= AA_NORMAL_MASK;
   else
      aa_defparams.supported &= ~AA_NORMAL_MASK;
   if (cnf_attr_bold)
      aa_defparams.supported |= AA_BOLD_MASK;
   else
      aa_defparams.supported &= ~AA_BOLD_MASK;
   if (cnf_attr_boldfont)
      aa_defparams.supported |= AA_BOLDFONT_MASK;
   else
      aa_defparams.supported &= ~AA_BOLDFONT_MASK;
   if (cnf_attr_dim)
      aa_defparams.supported |= AA_DIM_MASK;
   else
      aa_defparams.supported &= ~AA_DIM_MASK;
   if (cnf_attr_reverse)
      aa_defparams.supported |= AA_REVERSE_MASK;
   else
      aa_defparams.supported &= ~AA_REVERSE_MASK;
   if (cnf_font_extended)
      aa_defparams.supported |= AA_EXTENDED;
   else
      aa_defparams.supported &= ~AA_EXTENDED;
   if (cnf_font_eight)
      aa_defparams.supported |= AA_EIGHT;
   else
      aa_defparams.supported &= ~AA_EIGHT;
   /* try it out */
   aa = aa_autoinit(&aa_defparams);
   if (!aa)
      logfatal('V', _("AA-lib initialisation failed"));
   *bpp = 1;
   *real_width = *width = aa_imgwidth(aa);
   *height = aa_imgheight(aa);
   /* warm up the renderer */
#ifdef ALLOW_FASTRENDER
   if (cnf_fastrender)
      aa_fastrender(aa, 0, 0, aa_scrwidth(aa), aa_scrheight(aa));
   else
#endif
   {
      memset(&aarp, 0, sizeof(aarp));
      aarp.bright = cnf_brightness;
      aarp.contrast = cnf_contrast;
      aarp.gamma = cnf_gamma / 1000.0;
      aarp.dither = cnf_dither;
      aarp.randomval = cnf_randomval;
      aarp.inversion = cnf_inverse;
      aa_renderpalette(aa, aapal, &aarp, 0, 0, aa_scrwidth(aa), aa_scrheight(aa));
   }
}

void
reset_video(void)
{
   aa_close(aa);
   aa = NULL;
}

void
video_setpal(unsigned char idx,
	     unsigned char red, unsigned char green, unsigned char blue)
{
#ifdef ALLOW_FASTRENDER
   graypal[idx] = red;
   /* I guess aa_setpalette() is fast enough to be always called.  */
#endif
   aa_setpalette(aapal, idx, red, green, blue);
}

void *
video_newframe(void)
{
   return aa_image(aa);
}

void
video_updateframe(void *v)
{
#ifdef ALLOW_FASTRENDER
   if (cnf_fastrender) {
      int i = aa_imgwidth(aa) * aa_imgheight(aa);
      unsigned char *fb = aa_image(aa);
      while (i > 0) {
	 i--;
	 fb[i] = graypal[fb[i]];
      }
      aa_fastrender(aa, 0, 0, aa_scrwidth(aa), aa_scrheight(aa));
   } else
#endif
      aa_renderpalette(aa, aapal, &aarp, 0, 0,
		       aa_scrwidth(aa), aa_scrheight(aa));
   aa_flush(aa);
}

void
video_winstuff(const char *desc, int xdim, int ydim)
{
}

static void
set_aa_font(struct aa_hardware_params *p, const char *fontname)
{
   /* Adapted from aa_parseoptions() in aaparse.c of AA-lib 1.2 */
   int i;
   for (i = 0; aa_fonts[i] != NULL; i++) {
      if (!strcmp(fontname, aa_fonts[i]->name)
	  || !strcmp(fontname, aa_fonts[i]->shortname)) {
	 p->font = aa_fonts[i];
	 return;
      }
   }
   logprintf(LOG_ERROR, 'V', _("AA-lib doesn't know of font `%s'"),
	     fontname);
   logprintf(LOG_INFO, 'V', _("The following fonts are available:"));
   for (i = 0; aa_fonts[i] != NULL; i++)
      logprintf(LOG_INFO, 'V', "%-10s %s",
		aa_fonts[i]->shortname, aa_fonts[i]->name);
}

// Local Variables:
// c-basic-offset: 3
// End:
