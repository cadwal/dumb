#include <config.h>

#include <string.h>

#include "input.h"

ConfItem input_conf[]={
   CONFB("quit",NULL,0,"dummy input: always quit"),
   {NULL}
};
#define cnf_quit (input_conf[0].intval)

void get_input(PlayerInput *in) {
   memset(in,0,sizeof(PlayerInput));
   in->quit=cnf_quit;
}

void init_input(void) {
}
void reset_input(void) {
}

