#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>

void begin_file(FILE *, const char *name);
void end_file(void);

const char *next_token(void);
void unget_token(void);

void print_error_prefix(void);
void synerr(const char *detail) __attribute__((noreturn));
void err(const char *detail) __attribute__((noreturn));

#endif
