## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore

noinst_LIBRARIES = libmissing.a

# getopt.c and getopt1.c contain logic to elide code if using the
# GNU C library.
libmissing_a_SOURCES = libmissing.h getopt.c getopt.h getopt1.c
libmissing_a_LIBADD = @LIBOBJS@

INCLUDES = -I../intl
