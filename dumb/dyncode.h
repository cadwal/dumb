
#ifndef DYNCODE_H
#define DYNCODE_H

#include "levdata.h"
#include "lib/endian.h"

typedef struct {
   LE_int16 uas,las,mas;
} SideCode;

typedef struct {
   LE_int32 floor,ceiling;
   LE_int16 fas,cas;
} SectorCode;

typedef struct {
   LE_int32 x,y,z,angle,elev;
   LE_int16 proto,phase,owner,sector,hits,armour;
} ThingCode;

#define DECL_ENCODE(struc) DYN_ENCODE_FUNC(struc##_encode)
#define DECL_DECODE(struc) DYN_DECODE_FUNC(struc##_decode)

#define DECL_DYNCODE_FUNCS(struc) DECL_ENCODE(struc);DECL_DECODE(struc)

DECL_DYNCODE_FUNCS(Vertex);
DECL_DYNCODE_FUNCS(Side);
DECL_DYNCODE_FUNCS(Line);
DECL_DYNCODE_FUNCS(Sector);
DECL_DYNCODE_FUNCS(Thing);

#endif
