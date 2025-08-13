## Process this file with automake to produce Makefile.in

if DUMB_CONFIG_X
x_bin_programs = XWad XProtoThing
else
x_bin_programs = 
endif

bin_PROGRAMS = $(x_bin_programs)

XWad_SOURCES = xwad.c choose.c colour.c connect.c controls.c		\
disphash.c drawmap.c mapcon.c savemap.c tchoose.c xtexture.c		\
xwadctls.c choose.h colour.h controls.h disphash.h xtexture.h xwad.h	\
xwadmap.h

XProtoThing_SOURCES = xproto.c choose.c colour.c controls.c disphash.c	\
xproctls.c xtexture.c xproto.h choose.h colour.h controls.h disphash.h	\
xtexture.h

XProtoThing_LDADD = $(LDADD) $(DUMB_CONFIG_SOUND_LDADD)

# DUMB libraries are in the build tree, so don't use $(srcdir).
LDADD = ../libdumb/libdumb.a ../libdumbwad/libdumbwad.a			\
../libdumbutil/libdumbutil.a ../libmissing/libmissing.a $(X_PRE_LIBS)	\
-lX11 $(X_EXTRA_LIBS) $(INTLLIBS)

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libmissing -I../intl
