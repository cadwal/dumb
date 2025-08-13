#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <aalib.h>

#include "lib/log.h"
#include "input.h"

extern aa_context *aa_ctxt;
#define aa aa_ctxt

static PlayerInput inp_state;
static int run_state,strafe_state;

void get_input(PlayerInput *in) {
   while(1) {
      int speed=run_state?(2*UNIT_SPEED):UNIT_SPEED,pressed=1;
      int ev=aa_getevent(aa,0);
      if(ev==AA_NONE) break;
      if(ev&AA_RELEASE) {
	 pressed=0;
	 ev&=~AA_RELEASE;
      };
      switch(ev) {
      case('Q'):
      case('q'):
      case(AA_ESC):
	 inp_state.quit=pressed;
	 break;
      case(' '):
	 inp_state.jump=pressed*UNIT_SPEED;
	 break;
      case('\n'):
      case('\r'):
	 inp_state.shoot=pressed;
	 break;
      case('\t'):
      case(AA_BACKSPACE):
	 inp_state.w_sel=pressed;
	 break;
	 
      case(AA_LEFT):
	 if(!pressed) inp_state.sideways=inp_state.rotate=0;
	 else if(strafe_state) inp_state.sideways=speed;
	 else inp_state.rotate=speed;
	 break;

      case(AA_RIGHT):
	 if(!pressed) inp_state.sideways=inp_state.rotate=0;
	 else if(strafe_state) inp_state.sideways=-speed;
	 else inp_state.rotate=-speed;
	 break;

      case(AA_UP):
	 inp_state.forward=speed;
	 break;
      case(AA_DOWN):
	 inp_state.forward=-speed;
	 break;

	 /*
      case(XK_KP_9):
      case(XK_KP_Page_Up):
      case(XK_Page_Up):
	 inp_state.lookup=pressed?UNIT_SPEED:0;
	 break;
      case(XK_KP_3):
      case(XK_KP_Page_Down):
      case(XK_Page_Down):
	 inp_state.lookup=pressed?-UNIT_SPEED:0;
	 break;

      case(XK_Shift_L):
      case(XK_Shift_R):
	 run_state=pressed;
	 break;
      case(XK_Alt_L):
      case(XK_Alt_R):
      case(XK_Meta_L):
      case(XK_Meta_R):
      case(XK_slash):
	 strafe_state=pressed;
	 break;

#define NK(x) case(XK_##x): inp_state.select[x]=pressed; break
      NK(0);NK(1);NK(2);NK(3);NK(4);NK(5);NK(6);NK(7);NK(8);NK(9);
#undef NK
         */
      };
   };
   /* return current state */
   memcpy(in,&inp_state,sizeof(inp_state));
};

void init_input(void) {
   if(aa==NULL) logfatal('I',
			 "aalib_input must be initialised after aalib_video");
   aa_autoinitkbd(aa,AA_SENDRELEASE);
};
void reset_input(void) {
};


