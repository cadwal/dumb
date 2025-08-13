#include <config.h>

#include <string.h>
#include <pc.h>
#include <keys.h>

#include "input.h"

/* Uh oh, this badly needs bringing up to date! */

void get_input(PlayerInput *in) {
   memset(in,0,sizeof(PlayerInput));
   while(kbhit()) switch(getkey())  {
    case(K_Control_Up):
      in->forward=2*UNIT_SPEED;
      break;
    case(K_Up):
      in->forward=UNIT_SPEED;
      break;
    case(K_Control_Down):
      in->forward=-2*UNIT_SPEED;
      break;
    case(K_Down):
      in->forward=-UNIT_SPEED;
      break;
    case(K_Control_Left):
      in->sideways=UNIT_SPEED;
      break;
    case(K_Control_Right):
      in->sideways=-UNIT_SPEED;
      break;
    case(K_Left):
      in->rotate=UNIT_SPEED;
      break;
    case(K_Right):
      in->rotate=-UNIT_SPEED;
      break;
    case(K_Space):
      in->jump=UNIT_SPEED;
      break;
    case(K_Return):
      in->shoot=1;
      break;
    case(K_Escape):
      in->quit=1;
      break;
   }
}

void init_input(void) {}
void reset_input(void) {}

