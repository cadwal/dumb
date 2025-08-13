
#ifndef LINETYPE_H
#define LINETYPE_H 

#include "libdumb/linetypestruct.h"
#include "levdata.h"

void init_linetypes(void);
void reset_linetypes(void);

const LineType *lookup_linetype(int id);
const SectorType *lookup_sectortype(int id);

fixed get_term_type(const LevData *ld,LT_TermType ltt,int sector);

#endif
