#include <config.h>

#include "render.h"

#define USE_COLORMAP
#define COLORMAP "COLORMAP"
typedef Pixel8 Pixel;
typedef Pixel8 TPixel;
#define render render8
#define init_renderer init_renderer8
#define reset_renderer reset_renderer8
#define BPP 1

#include "renderc.h"
