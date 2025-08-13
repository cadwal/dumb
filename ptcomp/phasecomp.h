#ifndef PHASECOMP_H
#define PHASECOMP_H

#include <stdio.h>

#include "libdumb/prothingstruct.h"

typedef struct {
   ThingPhase *tp;
   char *tpname;
   char name[NAMELEN];
   int nphases,maxphases;
   LE_int16 signals[NUM_THINGSIGS];
} ThingPhaseRec;

void init_phasecomp(void);
void phasecomp(void);
void wrphases(FILE *fout);

ThingPhaseRec *find_ph_tbl(const char *s);
ThingPhaseRec *parm_ph_tbl(void);
int parm_phase(ThingPhaseRec *p);

#endif
