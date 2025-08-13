## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdumbworldb.a

EXTRA_DIST = PLAN

libdumbworldb_a_SOURCES = dumbworldb.h private.h alloc.c del.c \
 detach.c difficulty.c dummyvtbl.c findsec.c initfini.c isok.c \
 lineprop.c loaddoom.c observer.c prop.c savedoom.c thingsec.c \
 doomwad.h hexenwad.h

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I../intl
