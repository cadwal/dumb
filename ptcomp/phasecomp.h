/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/phasecomp.c: PhaseTable compiler.
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

#ifndef PHASECOMP_H
#define PHASECOMP_H

#include <stdio.h>

#include "libdumb/prothingstruct.h"

typedef struct {
   ThingPhase *tp;
   char *tpname;
   char name[NAMELEN];
   int nphases, maxphases;
   LE_int16 signals[NUM_THINGSIGS];
} ThingPhaseRec;

void init_phasecomp(void);
void phasecomp(void);
void wrphases(FILE *fout);

ThingPhaseRec *find_ph_tbl(const char *s);
ThingPhaseRec *parm_ph_tbl(void);
int parm_phase(ThingPhaseRec *p);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
