#ifndef CONFARGS_H
#define CONFARGS_H

#include "libdumbutil/confdef.h"

/* return non-zero if we ought to quit (eg. if args didn't make sense) */
int conf_args(const ConfModule conf[], int argc, char **argv);

#endif
