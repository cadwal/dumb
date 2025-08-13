#ifndef VIEW_H
#define VIEW_H

#include "libdumbutil/fixed.h"

typedef struct {
   fixed x, y, height;
   fixed angle;
   fixed arc;
   fixed view_plane_size;
   fixed eye_distance;
   fixed horizon;
   int sector;
} View;

typedef struct {
   fixed angle,offset;
} ViewTrans;

#define VIEW_ARC FIXED_HALF_PI
#define VIEW_EYE FIXED_ONE

void init_view(View *v);

#endif /* VIEW_H */
