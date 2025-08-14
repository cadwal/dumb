## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore

bin_PROGRAMS = ptcomp

ptcomp_SOURCES = ptcomp.c globals.h token.c token.h parm.c parm.h	\
animcomp.c animcomp.h fontcomp.c fontcomp.h fontmapcomp.c		\
fontmapcomp.h gettcomp.c gettcomp.h licomp.c licomp.h ltcomp.c		\
ltcomp.h msgdomcomp.c msgdomcomp.h phasecomp.c phasecomp.h protocomp.c	\
protocomp.h soundcomp.c soundcomp.h

# DUMB libraries are in the build tree, so don't use $(srcdir).
LDADD = ../libdumb/libdumb.a ../libdumbwad/libdumbwad.a			\
../libdumbutil/libdumbutil.a ../libmissing/libmissing.a $(INTLLIBS)

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libmissing -I../intl
