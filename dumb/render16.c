#include <config.h>

#include "render.h"

#define USE_COLORMAP
#define COLORMAP "COLORM16"
typedef Pixel16 Pixel;
typedef Pixel8 TPixel;
#define render render16
#define init_renderer init_renderer16
#define reset_renderer reset_renderer16
#define BPP 2

#include "renderc.h"
