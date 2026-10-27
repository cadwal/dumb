/* dos/config.h: config.h for building with DJGPP without configure,
   for dos/makefile.dj.  It is what
     configure --host=i586-pc-msdosdjgpp --disable-nls
   produces with DJGPP 2.05 and Allegro 4.2; regenerate it that way if
   configure.ac changes.  */

/* Allegro is used for video, input and sound; see libdumb/dosalleg.h.  */

#define ATTR_REGPARM

#define HAVE_FCNTL_H 1
#define HAVE_FORK 1
#define HAVE_GETPAGESIZE 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIBM 1
#define HAVE_LIMITS_H 1
#define HAVE_SETITIMER 1
#define HAVE_SNPRINTF 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRDUP 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRNLEN 1
#define HAVE_STRSTR 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WCHAR_H 1
/* No mmap, hstrerror, towlower, sockets or NLS in DJGPP 2.05.  */

#define PACKAGE "dumb"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME "dumb"
#define PACKAGE_STRING "dumb 0.13.11"
#define PACKAGE_TARNAME "dumb"
#define PACKAGE_URL ""
#define PACKAGE_VERSION "0.13.11"
#define VERSION "0.13.11"

#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define STDC_HEADERS 1
#define X_DISPLAY_MISSING 1
#define restrict __restrict__

/* For a parameter that a callback's signature needs but its
   function doesn't use.  */
#define ATTR_UNUSED __attribute__((unused))
