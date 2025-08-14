/* DUMB: A Doom-like 3D game engine.
 *
 * test/fixed_bm.c: Benchmark for fixed-point arithmetic.
 * Copyright (C) 1998, 1999 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/exitcode.h"
#include "libdumbutil/fixed.h"

static volatile loop;
static const char *argv0;

static int parse_int(const char *str);
static void exit_invalid_args(void) __attribute__((noreturn));

static RETSIGTYPE
alrm(int dummy)
{
   loop = 0;
}

static void
rtimer(void)
{
   struct itimerval it;
   loop = 1;
   it.it_value.tv_usec = 0;
   it.it_value.tv_sec = 2;
   it.it_interval.tv_usec = 0;
   it.it_interval.tv_sec = 0;
   setitimer(ITIMER_REAL, &it, NULL);
   signal(SIGALRM, alrm);
}

static int
parse_int(const char *str)
{
   char *tail;
   long value;
   int strtol_errno;
   errno = 0;
   value = strtol(str, &tail, 0);
   strtol_errno = errno;
   if (tail == str || *tail != '\0') {
      /* strtol() couldn't parse it at all, or there is some garbage
       * after it.  Note that str may be empty, in which case
       * *tail=='\0' but tail==str.  */
      fprintf(stderr, _("%s: invalid integer `%s'\n"), argv0, str);
      exit_invalid_args();
   }
   if (strtol_errno != 0
       || value < INT_MIN || value > INT_MAX) {
      fprintf(stderr, _("%s: integer `%s' is too large\n"), argv0, str);
      exit_invalid_args();
   }
   return (int) value;		/* will fit */
}

static void
exit_invalid_args(void)
{
   fprintf(stderr, _("Try `%s --help' for more information.\n"), argv0);
   exit(DUMB_EXIT_INVALID_ARGS);
}

/* these reproduce the losses due to procedure calls with fixmul etc.
   it is impossible to allow inlining with GCC without also allowing
   optimization, which can cheat some of the tests */
#ifdef FIXMUL_AS_MACRO

#define fltmul(a,b) (FIXED_TO_FLOAT(a)*FIXED_TO_FLOAT(b))
#define fltdiv(a,b) (FIXED_TO_FLOAT(a)/FIXED_TO_FLOAT(b))

#else

static inline float
fltmul(fixed a, fixed b)
{
   return FIXED_TO_FLOAT(a) * FIXED_TO_FLOAT(b);
}

static inline float
fltdiv(fixed a, fixed b)
{
   return FIXED_TO_FLOAT(a) / FIXED_TO_FLOAT(b);
}

#endif

int
main(int argc, char **argv)
{
   unsigned int i;
   static const struct option long_options[] =
   {
      { "seed", required_argument, NULL, 's' },
      { "help", no_argument, NULL, 'h' }, /* no -h */
      { "version", no_argument, NULL, 'V' }, /* no -V */
      { NULL, 0, NULL, '\0' }
   };
   argv0 = argv[0];
	 
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */

   for (;;) {
      int c = getopt_long(argc, argv, "s:", long_options, NULL);
      if (c == -1)
	 break;			/* end of options */
      switch (c) {
      case 's':			/* -s, --seed=INTEGER */
	 srand(parse_int(optarg));
	 break;
      case 'h':			/*     --help */
	 printf(_("Usage: %s [--seed=INTEGER]\n"
		  "Test how fast your machine can add, multiply and divide random numbers in\n"
		  "fixed and floating point.\n"
		  "\n"), argv0);
	 fputs(_("  -s, --seed=INTEGER  seed for random number generator\n"
		 "      --help          display this help and exit\n"
		 "      --version       output version information and exit\n"
		 "\n"), stdout);
	 print_bugaddr_message(stdout);
	 exit(EXIT_SUCCESS);
      case 'V':			/*     --version */
	 {
	    static const struct copyright copyrights[] = {
	       { "1998", "Josh Parsons" },
	       { "1994", "Chris Laurel" },
	       COPYRIGHT_END
	    };
	    printf("fixed_bm (DUMB) " VERSION "\n");
	    print_copyrights(copyrights);
	 }
	 printf(_("This program is free software; you may redistribute it under the terms of\n"
		  "the GNU General Public License.  This program has absolutely no warranty.\n"));
	 exit(EXIT_SUCCESS);
      case '?':			/* invalid option */
	 exit_invalid_args();
      }	/* switch */
   } /* for ever */
   
   if (argc > optind) {
      fprintf(stderr, _("%s: Non-option arguments are not allowed\n"), argv0);
      exit_invalid_args();
   }

   setvbuf(stdout, NULL, _IONBF, 0);
   /*
      printf("sizeof(long long)=%d sizeof(long)=%d, sizeof(int)=%d\n",
      sizeof(long long),sizeof(long),sizeof(int));
    */

   fputs(_("\nAdding random numbers..."), stdout);
   i = 0;
   rtimer();
   while (loop) {
      int n = rand() + rand();
      i++;
   }
   printf(_("  Fixed: %d"), i);
   i = 0;
   rtimer();
   while (loop) {
      float f = FIXED_TO_FLOAT(rand()) + FIXED_TO_FLOAT(rand());
      i++;
   }
   printf(_("  Float: %d"), i);

   fputs(_("\nMultiplying random numbers..."), stdout);
   i = 0;
   rtimer();
   while (loop) {
      int n = fixmul(rand(), rand());
      i++;
   }
   printf(_("  Fixed: %d"), i);
   i = 0;
   rtimer();
   while (loop) {
      float f = fltmul(rand(), rand());
      i++;
   }
   printf(_("  Float: %d"), i);

   fputs(_("\nDividing random numbers..."), stdout);
   i = 0;
   rtimer();
   while (loop) {
      fixed a = rand() & 0xffffff; /* FIXME: if RAND_MAX<0xFFFFFF */
      int n = fixdiv(a, a + FIXED_ONE);
      i++;
   }
   printf(_("  Fixed: %d"), i);
   i = 0;
   rtimer();
   while (loop) {
      fixed a = rand() & 0xffffff;
      float f = fltdiv(a, a + FIXED_ONE);
      i++;
   }
   printf(_("  Float: %d"), i);

   putchar('\n');
   return 0;
}

// Local Variables:
// c-basic-offset: 3
// End:
