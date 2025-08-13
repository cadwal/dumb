
#ifndef PROTHING_H
#define PROTHING_H

#include "libdumb/prothingstruct.h"
#include "libdumb/texture.h"

/* XProtoThing needs these functions.  */
void init_prothings(void);
void reset_prothings(void);
const ProtoThing *find_protothing(int id);
const ThingPhase *find_first_thingphase(int id);
#define find_thingphase(id,offset) (find_first_thingphase(id)+(offset))
Texture *find_phase_sprite(const ProtoThing *proto,
			   int phase,
			   char rot);

#endif


