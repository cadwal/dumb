#ifndef CONFFILE_H
#define CONFFILE_H

#include "libdumbutil/confdef.h"

char *find_conf_file(char *buf, const char *basename);

int load_conf(const ConfModule conf[], const char *fn);
int save_conf(const ConfModule conf[], const char *fn, int dirt);


#endif
