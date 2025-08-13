#ifndef GETTABLESTRUCT_H
#define GETTABLESTRUCT_H

#include "libdumbutil/endiantypes.h"

#define GK_MSG_LEN 56

typedef struct {
   LE_flags32 flags;
   LE_int32 xo,yo;
   LE_int16 collect;
   LE_int16 initial;
   LE_int16 key;
   LE_int16 bulletkind;
   LE_int16 special;
   LE_int16 bogotype;
   LE_int16 weaponnumber;	/* which key to press; 0=none, 1...10 */
   LE_int16 decay;
   LE_int16 usenum,bulletadd;
   LE_int16 replaceweapon;	/* gettable number to replace/disable */
   LE_int16 _spare2;
   char iconname[10];
   char string[GK_MSG_LEN];
   char iconanim,_spare;
   LE_int16 pickup_sound;
   LE_int16 timing;
   LE_int16 sound;
} Gettable;

#define GK_XCENTERICON 0x0001  
#define GK_YCENTERICON 0x0002  
#define GK_GOT_THE 0x0004      /* display string as "You got the..." */
#define GK_WEPSELECT 0x0008    /* can use as weapon */
#define GK_SPESELECT 0x0010    /* can use as special item */
#define GK_GOT_A 0x0020        /* "You got a *" and "That * would be wasted" */
#define GK_LOCAL 0x0040        /* loose this type between levels */
#define GK_REVANIM 0x0080 

#endif

// Local Variables:
// c-basic-offset: 3
// End:
