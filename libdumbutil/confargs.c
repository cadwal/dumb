/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/confargs.c: Parsing command-line arguments.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#include <config.h>

#include <stdlib.h>
#include <ctype.h>

#include "confargs.h"
#include "confeng.h"
#include "confhelp.h"

/* parse argument list in argc,argv using array of modules conf */
void
conf_args(const ConfModule *conf, int argc, char **argv)
{
   int i;
   ConfItem *ci = NULL;
   for (i = 1; i < argc; i++) {
      if (argv[i][0] != '-' || isdigit(argv[i][1])) {
	 if (ci)
	    set_conf(ci, argv[i], DIRT_ARGS);
	 else {
	    conf_usage(conf, argv[0], argv[i]);
	    exit(EXIT_FAILURE);	/* parameter without option */
	 }
      } else if (!strcmp(argv[i], "--help") || argv[i][1] == '?') {
	 if (argc > i + 1)
	    conf_help(conf, argv[0], argv[i + 1]);
	 else
	    conf_usage(conf, argv[0], NULL);
	 exit(EXIT_SUCCESS);	/* help requested */
      } else if (!strcmp(argv[i], "--version")) {
	 print_program_version();
	 exit(EXIT_SUCCESS);	/* version requested */
      } else if (argv[i][1] == '-') {
	 ci = conf_lookup_longname(conf, argv[i] + 2);
	 if (!ci) {
	    conf_usage(conf, argv[0], argv[i]);
	    exit(EXIT_FAILURE);	/* invalid long option */
	 }
	 if (ci->type == CONF_TYPE_BOOL)
	    set_conf(ci, NULL, DIRT_ARGS);
      } else {
	 ci = conf_lookup_shortname(conf, argv[i][1]);
	 if (!ci) {
	    conf_usage(conf, argv[0], argv[i]);
	    exit(EXIT_FAILURE);	/* invalid short option */
	 }
	 if (argv[i][2])
	    set_conf(ci, argv[i] + 2, DIRT_ARGS);
	 else if (ci->type == CONF_TYPE_BOOL)
	    set_conf(ci, NULL, DIRT_ARGS);
      }
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
