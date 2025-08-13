## Process this file with automake to produce Makefile.in

if DUMB_CONFIG_X
x_noinst_programs = mkdfnt
else
x_noinst_programs = 
endif

if DUMB_CONFIG_LIBPPM
ppm_bin_programs = dark2trans ppmtodumb
else
ppm_bin_programs = 
endif

bin_PROGRAMS = wadtool mknulmap mkpnames $(ppm_bin_programs)
noinst_PROGRAMS = fixed_bm $(x_noinst_programs)

x_ldadd = $(X_LIBS) $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)

ppm_ldadd = -lppm -lpgm -lpbm

# Don't use srcdir or top_srcdir here!
LDADD = ../libdumb/libdumb.a ../libdumbwad/libdumbwad.a \
 ../libdumbutil/libdumbutil.a

mkdfnt_LDADD = $(LDADD) $(x_ldadd)

ppmtodumb_LDADD = $(LDADD) $(ppm_ldadd)

dark2trans_LDADD = $(LDADD) $(ppm_ldadd)

INCLUDES = -I$(top_srcdir)

# fixed_bm should not be optimized because that's cheating.
fixed_bm_CFLAGS = 
fixed_bm.o: fixed_bm.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(fixed_bm_CFLAGS) -c $<
