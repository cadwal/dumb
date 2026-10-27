/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/log.c: Logging.
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef _WIN32
# include <windows.h>
#endif

#include "libdumbutil/dumb-nls.h"

#include "log.h"
#include "safem.h"

/* Even C++ allows using the same name for the typedef and the structure,
 * if they refer to the same thing.  */
typedef struct LogFile {
   struct LogFile *next;
   enum log_level lvl;
   char *classes;
   FILE *f;
   unsigned flags;
} LogFile;

/* LogFile::flags */
#define LOGF_CLOSE 0x0001

static LogFile *logs = NULL;
static int errors = 0;

#ifdef _WIN32
/* A program linked as a GUI program has nowhere to print to, so a
   fatal error is also shown in a message box, unless there is a
   console.  */
static int have_console;
static char fatal_msg[1024];
static char *last_log_file;
#endif

static void add_logfile(enum log_level, const char *classes,
			FILE *stream, unsigned flags);

void
log_close_all(void)
{
   while (logs != NULL) {
      LogFile *del = logs;
      logs = logs->next;
      if (del->flags & LOGF_CLOSE)
	 fclose(del->f);
      safe_free(del->classes);
      safe_free(del);
   }
}

static void
add_logfile(enum log_level lvl, const char *classes,
	    FILE *stream, unsigned flags)
{
   LogFile *log = (LogFile *) safe_malloc(sizeof(LogFile));
   log->lvl = lvl;
   if (classes == NULL)
      classes = "*";
   log->classes = safe_strdup(classes);
   log->f = stream;
   log->flags = flags;
   log->next = logs;
   logs = log;
}

void
log_stream(FILE *f, enum log_level lvl, const char *cl)
{
   add_logfile(lvl, cl, f, 0);
}

void
log_file(const char *fname, enum log_level lvl, const char *cl)
{
   FILE *f = fopen(fname, "a");
   if (f == NULL) {
      logprintf(LOG_ERROR, 'L', _("%s: opening log file: %s"), fname,
		strerror(errno));
      return;
   }
   add_logfile(lvl, cl, f, LOGF_CLOSE);
#ifdef _WIN32
   free(last_log_file);	/* from _fullpath(), may be NULL */
   last_log_file = _fullpath(NULL, fname, 0);
#endif
}

#ifdef _WIN32
int
log_win32_console(void)
{
   HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
   if (h != NULL && h != INVALID_HANDLE_VALUE)
      have_console = 1;		/* redirected, or a console program */
   else if (AttachConsole(ATTACH_PARENT_PROCESS)) {
      /* Started from cmd.exe or PowerShell: print there.  */
      freopen("CONOUT$", "w", stdout);
      freopen("CONOUT$", "w", stderr);
      have_console = 1;
   }
   return have_console;
}

static void
show_fatal_box(void)
{
   char text[sizeof(fatal_msg) + 512];
   if (have_console)
      return;
   if (last_log_file)
      snprintf(text, sizeof(text), _("%s\n\nThe log is in %s"),
	       fatal_msg, last_log_file);
   else
      snprintf(text, sizeof(text), "%s", fatal_msg);
   MessageBoxA(NULL, text, "DUMB",
	       MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}
#endif /* _WIN32 */

void
log_exit(void)
{
   logprintf(LOG_INFO, 'L', _("Normal exit."));
   log_close_all();
   exit(0);
}

void
log_chkerror(int maxerrs)
{
   if (errors > maxerrs)
      logprintf(LOG_FATAL, 'L', _("%d errors occured."), errors);
}

static int
log_ok(const LogFile *log, int l, char c)
{
   if (l > (int) log->lvl)
      return 0;
   else if (*log->classes == '*')
      return 1;
   else if (*log->classes == '!')
      return strchr(log->classes, c) == NULL;
   else
      return strchr(log->classes, c) != NULL;
}

void
do_logfatal(void)
{
   logprintf(LOG_INFO, 'L', _("A fatal error occured."));
   /* strerror(errno) should always be called as part of the log
    * message, and then the following line could be removed.  */
   perror(_("System error message (may be unrelated)"));
   log_close_all();
#ifdef _WIN32
   show_fatal_box();
#endif
   exit(EXIT_FAILURE);
}

void
logvprintf(int lvl, char cl, const char *fmt, va_list argl)
{
   LogFile *log;
   if (lvl < 0)
      return;
   for (log = logs; log != NULL; log = log->next) {
      if (log_ok(log, lvl, cl)) {
	 /* A va_list can be used only once; on x86-64 and other ABIs
	    that pass arguments in registers, a second vfprintf() with
	    the same one prints garbage.  */
	 va_list ap;
	 va_copy(ap, argl);
	 vfprintf(log->f, fmt, ap);
	 va_end(ap);
	 fputc('\n', log->f);
	 /* Flush so the log survives a crash.  */
	 fflush(log->f);
      }
   }
#ifdef _WIN32
   if (lvl == LOG_FATAL) {
      va_list ap;
      va_copy(ap, argl);
      vsnprintf(fatal_msg, sizeof(fatal_msg), fmt, ap);
      va_end(ap);
   }
#endif
   if (lvl == LOG_FATAL)
      do_logfatal();
   if (lvl == LOG_ERROR)
      errors++;
}

void
logprintf(int lvl, char cl, const char *fmt,...)
{
   va_list ap;
   va_start(ap, fmt);
   logvprintf(lvl, cl, fmt, ap);
   va_end(ap);
}

void
logfatal(char cl, const char *fmt,...)
{
   va_list ap;
   va_start(ap, fmt);
   logvprintf(LOG_FATAL, cl, fmt, ap);
   va_end(ap);
   exit(EXIT_FAILURE);		/* should never get here */
}

// Local Variables:
// c-basic-offset: 3
// End:
