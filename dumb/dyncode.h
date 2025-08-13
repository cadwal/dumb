
#ifndef DYNCODE_H
#define DYNCODE_H

#include "libdumbutil/endiantypes.h"
#include "levdata.h"

typedef struct {
   LE_int16 uas,las,mas,_spare;
} SideCode;

typedef struct {
   LE_int32 floor,ceiling;
   LE_int16 fas,cas;
} SectorCode;

typedef struct {
   LE_int32 x,y,z,angle,elev;
   LE_int16 proto,phase,owner,sector;
   LE_int16 hits,armour,tmpinv,tmpgod;
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
