## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore

noinst_LIBRARIES = libdumbwad.a

libdumbwad_a_SOURCES = loadlump.c wadio.c wadwr.c \
 wadio.h wadstruct.h wadwr.h

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I../intl
