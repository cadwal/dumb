#include <config.h>

#include <stddef.h>

#include "draw.h"
#include "things.h"
#include "bogothing.h"

void draw_bogothings(const LevData *ld,void *fb,int width,int height) {
   int i;
   ThingDyn *td=ldthingd(ld);
   for(i=0;i<ldnthings(ld);i++,td++) {
      if(!td->proto||!(td->proto->flags&PT_BOGUS)) continue;
      if(td->owner!=ld->player[ld->localplayer]) continue;
      thing_rotate_image(ld,i,0);
      if(td->image) draw(fb,td->image,
			 (width-td->image->width)/2,
			 height-td->image->height);
   }
}
