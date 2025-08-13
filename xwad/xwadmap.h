
#ifndef XWADMAP_H
#define XWADMAP_H

#define MYSHR(x,y) ((y)<0?((x)<<-(y)):((x)>>(y)))

/* scale from map to screen */
#define SCALE(i) MYSHR(i,inst->scale)
#define UNSCALE(i) MYSHR(i,-inst->scale)

/* convert map coords to screen ones */
#define VER_SCREENX(v) (SCALE((int)(v)->x-inst->xoffset)+inst->map_width/2)
#define VER_SCREENY(v) (SCALE(-(int)(v)->y+inst->yoffset)+inst->map_height/2)

#define TH_SCREENX VER_SCREENX
#define TH_SCREENY VER_SCREENY

/* convert screen to map */
#define MAPX(x) (UNSCALE((x)-inst->map_width/2)+inst->xoffset)
#define MAPY(x) (-(UNSCALE((x)-inst->map_height/2)-inst->yoffset))

/* as this is, things are a bit strange around the origin: should fix! */
#define GRIDIFY(x) ((x/inst->gridsize)*inst->gridsize)

#endif

