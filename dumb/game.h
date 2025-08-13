
#ifndef GAME_H

#include "plat/input.h"
#include "levdata.h"

/* mostly what's declared here gets defined in dumb.c
   it will be moved to game.c when the main loop gets tidied up */

void game_message(int player,const char *fmt,...)
 __attribute__((format (printf, 2, 3)));

void game_want_newlvl(int secret);
void game_want_quit(int really);

 /* player stuff */

extern ConfItem playconf[];
void slave_input(LevData *ld,const PlayerInput *in,int tickspassed);
void process_input(LevData *ld,const PlayerInput *in,int tickspassed,int pl);


#endif
