/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/log.h: Logging.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 * Copyright (C) 1998 by Kalle O. Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>

enum log_level {
   LOG_FATAL,
   LOG_BANNER,
   LOG_ERROR,
   LOG_WARNING,
   LOG_INFO,
   LOG_XINFO,
   LOG_DEBUG,
   LOG_ALL = LOG_DEBUG
};

void logvprintf(int lvl, char cl, const char *fmt, va_list argl);

void logprintf(int lvl, char cl, const char *fmt,...)
     __attribute__((format(printf, 3, 4)));

void logfatal(char cl, const char *fmt,...)
     __attribute__((noreturn, format(printf, 2, 3)));

void log_stream(FILE *f, enum log_level, const char *cl);
void log_file(const char *fname, enum log_level, const char *cl);
void log_close_all(void);	/* called automatically by log_exit() */

void log_exit(void);
void log_chkerror(int maxerrs);

#define log_stdout() log_stream(stdout, LOG_ALL, NULL)

#endif

// Local Variables:
// c-basic-offset: 3
// End:
