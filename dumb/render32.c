#include <config.h>

#include "render.h"

#define USE_COLORMAP
#define COLORMAP "COLORM32"
typedef Pixel32 Pixel;
typedef Pixel8 TPixel;
#define render render32
#define init_renderer init_renderer32
#define reset_renderer reset_renderer32
#define BPP 4

#include "renderc.h"
