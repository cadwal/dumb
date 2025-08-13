#ifndef PARM_H
#define PARM_H

#include "libdumbutil/fixed.h"

void parm_str(char *buf, size_t n);
char parm_ch(void);
int parm_num(void);
double parm_dbl(void);
int parm_time(void);
int parm_speed(void);
fixed parm_arc(void);
fixed parm_arc_opt(fixed def);
void parm_msg(char *buf, size_t n);

extern fixed default_speed;

void change_time_units(void);
void change_default_speed(void);

#endif
