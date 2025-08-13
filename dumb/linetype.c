#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "linetype.h"

static LumpNum linetype_ln=BAD_LUMPNUM;
static const LineType *lt=NULL;
static int nlts=0;

static LumpNum sectortype_ln=BAD_LUMPNUM;
static const LineType *st=NULL;
static int nsts=0;

void init_linetypes(void) {
   /* load linetypes */
   linetype_ln=lookup_lump("LINETYPE",NULL,NULL);
   lt=NULL; nlts=0;
   if(LUMPNUM_OK(linetype_ln)) {
      lt=load_lump(linetype_ln);
      nlts=get_lump_len(linetype_ln)/sizeof(LineType);
      logprintf(LOG_INFO,'M',"Loaded %d linetypes",nlts);
   }
   else logprintf(LOG_ERROR,'M',"Failed to find LINETYPE in wad");
   /* now sectortypes */
   sectortype_ln=lookup_lump("SECTTYPE",NULL,NULL);
   st=NULL; nsts=0;
   if(LUMPNUM_OK(sectortype_ln)) {
      st=load_lump(sectortype_ln);
      nsts=get_lump_len(sectortype_ln)/sizeof(SectorType);
      logprintf(LOG_INFO,'M',"Loaded %d sectortypes",nsts);
   } else 
      logprintf(LOG_ERROR,'M',"Failed to find SECTTYPE in wad");
}

void reset_linetypes(void) {
   if(LUMPNUM_OK(linetype_ln)&&lt) free_lump(linetype_ln);
   if(LUMPNUM_OK(sectortype_ln)&&st) free_lump(sectortype_ln);
   sectortype_ln=linetype_ln=BAD_LUMPNUM;
   st=lt=NULL;
   nsts=nlts=0;
}

#ifdef FAKE_LINETYPE_LUMP
static const LineType *fake_linetype(int id) {
   static LineType mylt;
   switch(id) {
   case(1):
   case(26):
   case(28):
   case(27):
   case(31):
   case(32):
   case(33):
   case(34):
   case(47):
   case(117):
   case(118):
      /* return a manual door-ish linetype structure */
      memset(&mylt,0,sizeof(LineType));
      mylt.flags=LT_REPEATABLE|
	 LT_ALLOW_PLAYER|LT_ALLOW_NONPLAYER|
	 LT_ON_THUMPED|LT_ON_ACTIVATED;
      mylt.action[0].flags=LTA_MANUAL;
      mylt.action[0].lumptype=ML_SECTOR;
      mylt.action[0].eventtype=ME_CEILING;
      mylt.action[0].term_type[0]=LowestAdjacentCeiling;
      mylt.action[0].term_offset[0]=-(4<<12);
      mylt.action[0].speed[0]=1<<11;
      mylt.action[1].flags=LTA_MANUAL;
      mylt.action[1].lumptype=ML_SECTOR;
      mylt.action[1].eventtype=ME_CEILING;
      mylt.action[1].delay=128;
      mylt.action[1].term_type[0]=Floor;
      mylt.action[1].speed[0]=-(1<<11);
      return &mylt;
   }
   return NULL;
}
#endif

const LineType *lookup_linetype(int id) {
#ifdef FAKE_LINETYPE_LUMP
   if(lt==NULL) return fake_linetype(id);
#endif
   if(id>0&&id<=nlts) return lt+id;
   else return NULL;
}

const SectorType *lookup_sectortype(int id) {
   if(id>0&&id<=nsts) return st+id;
   else return NULL;
}

// Local Variables:
// c-basic-offset: 3
// End:
