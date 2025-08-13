#ifndef CONFENG_H
#define CONFENG_H

#include "libdumbutil/confdef.h"

void set_conf(ConfItem *ci,char *val,int dirt);
void conf_clear_list(ConfItem *ci);

int conf_greatest_dirtlevel(const ConfModule conf[]);

ConfItem *conf_lookup_longname(const ConfModule conf[], const char *s);
ConfItem *conf_lookup_shortname(const ConfModule conf[], char ch);

#endif
