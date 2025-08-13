#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libdumbutil/log.h"
#include "things.h"
#include "gettable.h"


#define TARG_ARC \
  (ldthingd(ld)[t].proto->aim_arc+ldthingd(ld)[t].proto->shootarc+FIXED_PI/16) 

/*#define TARG_ARC (FIXED_PI/4)*/

int use_item(LevData *ld,int pl,const Gettable *gt) {
   if(gt->flags&GK_WEPSELECT) {
      int t=ld->plwep[pl];
      /*logprintf(LOG_DEBUG,'U',"use_item: pl=%d t=%d",pl,t);*/
      if(t<0||ldthingd(ld)[t].proto==NULL) return 0;
      /* only do autotarget + shoot if both player and bogothing are ready */
      if(thing_sig_ok(ld,ld->player[pl],TS_SHOOT)&&
	 thing_sig_ok(ld,t,TS_SHOOT)) {
	 thing_autotarget(ld,t,TARG_ARC);
	 thing_send_sig(ld,ld->player[pl],TS_SHOOT);
	 thing_send_sig(ld,t,TS_SHOOT);
	 return 1;
      }
   }
   return 0;
}
