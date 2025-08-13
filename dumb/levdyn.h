
#ifndef LEVDYN_H
#define LEVDYN_H

#include "levdata.h"
#include "lib/fixed.h"
#include "texture.h"
#include "prothing.h"

#define LD_DYNDECL(type) DYN_INIT_FUNC(type##_dyninit)
#define LD_DYNINIT(type,dyn,lump) type##Dyn *dyn=_dyn;const type##Data *lump=_lump

struct LineDyn_struct;

typedef struct {
   fixed x,y,tx,ty,proj;
} VertexDyn;
LD_DYNDECL(Vertex);

typedef struct {
   Texture *utex,*mtex,*ltex;
   struct LineDyn_struct *lined;
   int uanim,manim,lanim;
   int uanimstate,manimstate,lanimstate;
} SideDyn;
LD_DYNDECL(Side);

typedef struct LineDyn_struct {
   fixed length;
   fixed cent_x,cent_y;
   char block_activation,had_keymsg,_spare1,_spare2;
} LineDyn;
LD_DYNDECL(Line);

/* values for crush_effect are defined in levdata.h */

typedef struct {
   Texture *ftex,*ctex;
   fixed floor,ceiling;
   fixed cent_x,cent_y;
   fixed cent_r;
   int fanim,canim;
   fixed dark; /* 0 is infinite light (see forever), 1 is very dark */
   int fanimstate,canimstate;
   int type;
   char sky,crush_effect,_spare1,_spare2;
   /* sky: bit 1 is ceiling, bit 2 floor */
} SectorDyn;
LD_DYNDECL(Sector);

#define MAX_SPARE_THINGS 64
typedef struct {
   const ProtoThing *proto;
   const ThingPhase *phase_tbl;
   fixed x,y,z;
   fixed angle,elev;
   int sector;
   int hits,armour,tmpinv,tmpgod;
   int phase;
   /* changes in members after this don't require an update */
   fixed dx,dy,dz,dangle,delev;
   fixed bouncemax;
   ThingSignal pending_signal;
   Texture *image;
   int mirror_image;
   int phase_wait;
   int target;
   int owner;
   int wakeness;
} ThingDyn;
LD_DYNDECL(Thing);

/* ahg! a horrible kludge */
/*#define THING_MAGIC_JELLYBEAN \
 ( sizeof(int)*6 + sizeof(fixed)*5 + sizeof(void*)*2 )*/

/* no quite so horrible (thanks, Kalle) */
#define THING_MAGIC_JELLYBEAN offsetof(ThingDyn,dx)

#define ldvertexd(ld) ((VertexDyn *)(ld->dyn[ML_VERTEX]))
#define ldsectord(ld) ((SectorDyn *)(ld->dyn[ML_SECTOR]))
#define ldlined(ld) ((LineDyn *)(ld->dyn[ML_LINE]))
#define ldsided(ld) ((SideDyn *)(ld->dyn[ML_SIDE]))
#define ldthingd(ld) ((ThingDyn *)(ld->dyn[ML_THING]))

#define ldthingph(ld,on) (ldthingd(ld)[on].phase_tbl+ldthingd(ld)[on].phase)

#define wall_length(ld,ln) (ldlined(ld)[ln].length)

#endif

// Local Variables:
// c-basic-offset: 3
// End:
