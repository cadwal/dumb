#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "libdumbutil/fixed.h"
#include "libdumbutil/log.h"
#include "view.h"

void init_view(View *v) {
   memset(v,0,sizeof(View));
   v->arc = VIEW_ARC;
   v->eye_distance = VIEW_EYE;
   v->view_plane_size = FIXED_ONE;
/*     fixmul(FLOAT_TO_FIXED(tan(FIXED_TO_FLOAT(v->arc) / 2.0)), 
	    v->eye_distance);*/
/*   logprintf(LOG_INFO,'R',"init_view: arc=%f focal dist=%f width=%f",
	     FIXED_TO_FLOAT(v->arc),
	     FIXED_TO_FLOAT(v->eye_distance),
	     FIXED_TO_FLOAT(v->view_plane_size)
	     );*/
   v->sector=-1;
}
