/* DUMB: A Doom-like 3D game engine.
 *
 * ptcomp/token.h: Reading tokens from the source file.
 * Copyright (C) 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

void begin_file(FILE *, const char *name);
void end_file(void);

/* There are four kind of tokens:
 * - eof: NULL
 * - newline: "\n"
 * - string: "\"text with backslashes (\\\\) and quotes (\\\")\""
 * - name: "symbol", "42"
 */

enum TokenClass {
   TOKENCL_EOF,
   TOKENCL_NEWLINE,
   TOKENCL_STRING,
   TOKENCL_NAME
};
enum TokenClass class_of_token(const char *);

const char *next_token(void);
void unget_token(void);

void print_error_prefix(void);
void warn(const char *format, ...)
     __attribute__((format(printf, 1, 2)));
void synerr(const char *format, ...)
     __attribute__((format(printf, 1, 2), noreturn));
void err(const char *format, ...)
     __attribute__((format(printf, 1, 2), noreturn));

#endif

// Local Variables:
// c-basic-offset: 3
// End:
