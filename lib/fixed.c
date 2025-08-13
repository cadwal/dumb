#include <math.h>
#include <stdio.h>
#include "fixed.h"

/* 
   these could be done with tables: might be worthwhile if someone wants
   to run it on a 386 or 486SX
*/
fixed fixsin(fixed theta) {return FIXSIN(theta);};
fixed fixcos(fixed theta) {return FIXCOS(theta);};
fixed fixtan(fixed theta) {return FIXTAN(theta);};

fixed fix_pyth3d(fixed x,fixed y,fixed z) {
   const double dx=FIXED_TO_FLOAT(x),dy=FIXED_TO_FLOAT(y),dz=FIXED_TO_FLOAT(z);
   return FLOAT_TO_FIXED(sqrt(dx*dx+dy*dy+dz*dz));
};

fixed fix_pythagoras(fixed x,fixed y) {
   const double dx=FIXED_TO_FLOAT(x),dy=FIXED_TO_FLOAT(y);
   return FLOAT_TO_FIXED(sqrt(dx*dx+dy*dy));
};

fixed fix_vec2angle(fixed x,fixed y) {
   const double dx=FIXED_TO_FLOAT(x),dy=FIXED_TO_FLOAT(y);
   return FLOAT_TO_FIXED(atan2(dy,dx));
};

fixed fix_sqrt(fixed f) {
   const double d= FIXED_TO_FLOAT(f);
   return FLOAT_TO_FIXED(sqrt(d));
};
