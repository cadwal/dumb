## Process this file with automake to produce Makefile.in

EXTRA_DIST = PLAN .cvsignore

noinst_LIBRARIES = libdumbworldb.a

libdumbworldb_a_SOURCES = dumbworldb.h dumbworldb.hh private.h alloc.c	\
del.c detach.c difficulty.c dummyvtbl.c findsec.c initfini.c isok.c	\
lineprop.c loaddoom.c observer.c prop.c savedoom.c thingsec.c		\
doomwad.h hexenwad.h

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I../intl
