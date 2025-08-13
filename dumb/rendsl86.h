/* DUMB: A Doom-like 3D game engine.
 *
 * dumb/rendsl86.h: Drawing slices of walls and sprites.  Intel x86 version.
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

#define PIXEL_GUARD_VAL ((TPixel) ~0)

static inline void
SLICE_STRING(int rshift,
	     int count, const TPixel *tex_base,
	     Pixel *end,
	     fixed tex_y, fixed tex_dy
)
{
   asm("\t1:\n\t"
       "movl %4, %%eax\n\t"
       "shrl %5, %%eax\n\t"
       "addl %2, %4\n\t"

#if BPP==1
       "movb (%%ecx,%%eax), %%al\n\t"
       "xlatb\n\t"
       "movb %%al, (%%esi,%%edi)\n\t"
#elif BPP==2
       "movzbl (%%ecx,%%eax), %%eax\n\t"
       "movw (%%ebx,%%eax,2), %%ax\n\t"
       "movw %%ax, (%%esi,%%edi,2)\n\t"
#elif BPP==4
       "movzbl (%%ecx,%%eax), %%eax\n\t"
       "movl (%%ebx,%%eax,4), %%eax\n\t"
       "movl %%eax, (%%esi,%%edi,4)\n\t"
#endif

       "subl %6, %%edi\n\t"
       "jns   1b\n\t"
       :
       : "D"(count), "c"(tex_base), "g"(tex_dy),
       "S"(end), "d"(tex_y), "g"(rshift),
       "g"(fb_width), "b"(colormap)
       : "eax", "edi"
       );
}

static void
draw_wall_slice(Pixel *start, Pixel *end, const TPixel *tex_base,
		fixed tex_y, fixed tex_dy,
		int log2_tex_height)
{
   int count = start - end;

   tex_y <<= (16 - log2_tex_height);
   tex_dy <<= (16 - log2_tex_height);

   /* Since SLICE_STRING is a macro that builds the asm string, we
      can't pass an expression, we have to pass the desired integer.
      This case statement will still disappear due to inlining.  - DSF */

   switch (log2_tex_height) {
   case 0:
   case 1:
   case 2:
   case 3:
      SLICE_STRING(29, count, tex_base, end, tex_y, tex_dy);
      break;
   case 4:
      SLICE_STRING(28, count, tex_base, end, tex_y, tex_dy);
      break;
   case 5:
      SLICE_STRING(27, count, tex_base, end, tex_y, tex_dy);
      break;
   case 6:
      SLICE_STRING(26, count, tex_base, end, tex_y, tex_dy);
      break;
   case 7:
      SLICE_STRING(25, count, tex_base, end, tex_y, tex_dy);
      break;
   case 8:
      SLICE_STRING(24, count, tex_base, end, tex_y, tex_dy);
      break;
   case 9:
      SLICE_STRING(23, count, tex_base, end, tex_y, tex_dy);
      break;
   case 10:
      SLICE_STRING(22, count, tex_base, end, tex_y, tex_dy);
      break;
   }
}

static inline void
TSLICE_STRING(int rshift,
	      int count, const TPixel *tex_base,
	      Pixel *end,
	      fixed tex_y, fixed tex_dy
)
{
   asm("\t1:\n\t"
       "movl %4, %%eax\n\t"
       "shrl %5, %%eax\n\t"
       "addl %2, %4\n\t"
       "movzbl (%%ecx,%%eax), %%eax\n\t"
       "cmpb $-1,%%al\n\t"
       "je 2f\n\t"

#if BPP==1
       "xlatb\n\t"
       "movb %%al, (%%esi,%%edi)\n\t"
#elif BPP==2
       "movw (%%ebx,%%eax,2), %%ax\n\t"
       "movw %%ax, (%%esi,%%edi,2)\n\t"
#elif BPP==4
       "movl (%%ebx,%%eax,4), %%eax\n\t"
       "movl %%eax, (%%esi,%%edi,4)\n\t"
#endif

       "\t2:\n\t"
       "subl %6, %%edi\n\t"
       "jns   1b\n\t"
       :
       : "D"(count), "c"(tex_base), "g"(tex_dy),
       "S"(end), "d"(tex_y), "g"(rshift),
#ifdef USE_COLORMAP
       "g"(fb_width), "b"(colormap)
#else
       "b"(fb_width)
#endif
       : "eax", "edi"
       );
}

static void
draw_trans_slice(Pixel *start, Pixel *end, const TPixel *tex_base,
		 fixed tex_y, fixed tex_dy,
		 int log2_tex_height)
{
   int count = start - end;

   tex_y <<= (16 - log2_tex_height);
   tex_dy <<= (16 - log2_tex_height);

   switch (log2_tex_height) {
   case 0:
   case 1:
   case 2:
   case 3:
      TSLICE_STRING(29, count, tex_base, end, tex_y, tex_dy);
      break;
   case 4:
      TSLICE_STRING(28, count, tex_base, end, tex_y, tex_dy);
      break;
   case 5:
      TSLICE_STRING(27, count, tex_base, end, tex_y, tex_dy);
      break;
   case 6:
      TSLICE_STRING(26, count, tex_base, end, tex_y, tex_dy);
      break;
   case 7:
      TSLICE_STRING(25, count, tex_base, end, tex_y, tex_dy);
      break;
   case 8:
      TSLICE_STRING(24, count, tex_base, end, tex_y, tex_dy);
      break;
   case 9:
      TSLICE_STRING(23, count, tex_base, end, tex_y, tex_dy);
      break;
   case 10:
      TSLICE_STRING(22, count, tex_base, end, tex_y, tex_dy);
      break;
   }
}


static void
draw_invis_slice(Pixel *start, Pixel *end,
		 const TPixel *tex_base,
		 fixed tex_y, fixed tex_dy,
		 int log2_tex_height)
{
   unsigned int utex_y;
   TPixel p;

   utex_y = (unsigned int) tex_y;

   utex_y <<= (16 - log2_tex_height);
   tex_dy <<= (16 - log2_tex_height);

   while (start >= end) {
      p = tex_base[utex_y >> (32 - log2_tex_height)];
#if BPP==1
      if (p != PIXEL_GUARD_VAL)
	 *start = invismap[*start];
#elif BPP==2
      if (p != PIXEL_GUARD_VAL)
	 *start &= 0x0eff;
#elif BPP==4
      if (p != PIXEL_GUARD_VAL)
	 *start &= 0x0000ffff;
#endif
      utex_y += tex_dy;
      start -= fb_width;
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
