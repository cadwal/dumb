## Process this file with automake to produce Makefile.in

bin_PROGRAMS = ptcomp

ptcomp_SOURCES = ptcomp.c globals.h token.c token.h parm.c parm.h \
 animcomp.c animcomp.h gettcomp.c gettcomp.h licomp.c licomp.h \
 ltcomp.c ltcomp.h phasecomp.c phasecomp.h protocomp.c protocomp.h \
 soundcomp.c soundcomp.h

# DUMB libraries are in the build tree, so don't use $(srcdir).
LDADD = ../libdumb/libdumb.a ../libdumbwad/libdumbwad.a \
 ../libdumbutil/libdumbutil.a $(INTLLIBS)

INCLUDES = -I$(top_srcdir)
