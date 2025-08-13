## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = fixed_bm ldltest

fixed_bm_LDADD = ../libdumbutil/libdumbutil.a $(INTLLIBS)

# DUMB libraries are in the build tree, so don't use $(srcdir).
# libdumb.a and libdumbworldb.a currently refer to each other.  This
# will be fixed later.
ldltest_LDADD = ../libdumb/libdumb.a ../libdumbworldb/libdumbworldb.a \
 ../libdumb/libdumb.a \
 ../libdumbwad/libdumbwad.a ../libdumbutil/libdumbutil.a \
 ../libmissing/libmissing.a $(INTLLIBS)

# libintl.h is generated in the build tree.
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libmissing -I../intl

# fixed_bm should not be optimized because that's cheating.
fixed_bm_CFLAGS = 
fixed_bm.o: fixed_bm.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(fixed_bm_CFLAGS) -c $<
