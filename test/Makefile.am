## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore

if DUMB_CONFIG_LDWB
ldwb_programs = dwdbtest
else
ldwb_programs = 
endif

noinst_PROGRAMS = fixed_bm $(ldwb_programs)

LDADD = ../libdumbutil/libdumbutil.a ../libmissing/libmissing.a	\
$(INTLLIBS)

fixed_bm_LDADD = $(LDADD)

# DUMB libraries are in the build tree, so don't use $(srcdir).
# FIXME: libdumb.a and libdumbworldb.a currently refer to each other.
dwdbtest_LDADD = ../libdumb/libdumb.a ../libdumbworldb/libdumbworldb.a	\
../libdumb/libdumb.a ../libdumbwad/libdumbwad.a $(LDADD)

# libintl.h is generated in the build tree.
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libmissing -I../intl

CC = $(CXX_HACK_CC)

# fixed_bm should not be optimized because that's cheating.
fixed_bm_CFLAGS = 
fixed_bm.o: fixed_bm.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(fixed_bm_CFLAGS) -c $<
