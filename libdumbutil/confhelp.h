#ifndef CONFHELP_H
#define CONFHELP_H

#include "libdumbutil/confdef.h"

int conf_help(const ConfModule conf[], const char *prog, const char *mod);
int conf_usage(const ConfModule conf[], const char *prog, const char *err);

#endif
