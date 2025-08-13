## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdumbutil.a

common_sources = align.h confargs.c confargs.h confdef.h \
 confeng.c confeng.h conffile.c conffile.h confhelp.c confhelp.h \
 dumb-nls.h endiantypes.h fixed.c fixed.h log.c log.h safeio.c \
 safeio.h safem.c safem.h strgrow.c strgrow.h timer.h

if DUMB_CONFIG_DOS
libdumbutil_a_SOURCES = $(common_sources) dos_timer.c
else
libdumbutil_a_SOURCES = $(common_sources) timer.c
endif

# Files that are compiled sometimes but not always, must be listed
# here.  Automake 1.3 is not smart enough to combine the different
# versions of libdumbutil_a_SOURCES automatically.
EXTRA_libdumbutil_a_SOURCES = dos_timer.c timer.c

# These sources aren't currently used at all.
EXTRA_DIST = intset.c intset.h

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I../intl
