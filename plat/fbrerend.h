/* 
   fbrerend.h

   Copyright (C) 1997 Marcus Sundberg (e94_msu@e.kth.se)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef WANT_1BPP
void *fbrerender8(Pixel8 *fb, Pixel8 *rendfb, int xsize, int ysize, int xfact, int yfact, int xlace, int ylace);
#endif
#ifdef WANT_2BPP
void *fbrerender16(Pixel16 *fb, Pixel16 *rendfb, int xsize, int ysize, int xfact, int yfact, int xlace, int ylace);
#endif
#ifdef WANT_4BPP
void *fbrerender32(Pixel32 *fb, Pixel32 *rendfb, int xsize, int ysize, int xfact, int yfact, int xlace, int ylace);
#endif
