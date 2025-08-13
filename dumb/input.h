
#ifndef INPUT_H
#define INPUT_H

#include "libdumbutil/confdef.h"
#include "libdumbutil/endiantypes.h"

extern ConfItem input_conf[];

/* The PlayerInput structure contains a series of fields each of which
 * (rougly) corresponds to a command key.  The fields are integers so that
 * the input module can report the intensity with which the command was
 * issued.  This comes in handy for mice and joysticks.  Fields like
 * rotate, forward, sideways, and jump permit negative values.
 */

#define UNIT_SPEED 16

typedef struct {
   LE_int32 forward;
   LE_int32 rotate;
   LE_int32 sideways;
   LE_int32 lookup;
   LE_int32 jump;
   char shoot;
   char action;
   char quit;
   char w_sel;
   char s_sel;
   char use;
   char dummy1,dummy2;
   char select[16]; 
} PlayerInput;

void get_input(PlayerInput *in);

void init_input(void);
void reset_input(void);

#endif
