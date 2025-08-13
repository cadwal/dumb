## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdumbutil.a

libdumbutil_a_SOURCES = align.h confargs.c confargs.h confdef.h \
 confeng.c confeng.h conffile.c conffile.h confhelp.c confhelp.h \
 dumb-nls.h endiantypes.h fixed.c fixed.h log.c log.h safeio.c \
 safeio.h safem.c safem.h timer.c timer.h

# These sources aren't currently used at all.
EXTRA_libdumbutil_a_SOURCES = dos_timer.c intset.c intset.h

INCLUDES = -I$(top_srcdir)
