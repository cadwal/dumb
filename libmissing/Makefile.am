## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libmissing.a

# getopt.c and getopt1.c contain logic to elide code if using the
# GNU C library.
libmissing_a_SOURCES = getopt.c getopt.h getopt1.c

INCLUDES = -I../intl
