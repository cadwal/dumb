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

bin_PROGRAMS = ptcomp wadtool mknulmap $(ppm_bin_programs)
noinst_PROGRAMS = fixed_bm mkpnames $(x_noinst_programs)

x_ldadd = $(X_LIBS) $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)

ppm_ldadd = -lppm -lpgm -lpbm
ppm_cflags = 

# Don't use srcdir or top_srcdir here!
LDADD = ../libdumb/libdumb.a ../libdumbwad/libdumbwad.a \
 ../libdumbutil/libdumbutil.a

mkdfnt_LDADD = $(LDADD) $(x_ldadd)
mkdfnt_CFLAGS = $(CFLAGS) $(X_CFLAGS)

ppmtodumb_LDADD = $(LDADD) $(ppm_ldadd)
ppmtodumb_CFLAGS = $(CFLAGS) $(ppm_cflags)

dark2trans_LDADD = $(LDADD) $(ppm_ldadd)
dark2trans_CFLAGS = $(CFLAGS) $(ppm_cflags)

# fixed_bm should not be optimized because that's cheating.
fixed_bm_CFLAGS = 

INCLUDES += -I$(top_srcdir)

# Special rules for building objects that need special CFLAGS
mkdfnt.o: mkdfnt.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(mkdfnt_CFLAGS) -c $<
ppmtodumb.o: ppmtodumb.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(ppmtodumb_CFLAGS) -c $<
dark2trans.o: dark2trans.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(dark2trans_CFLAGS) -c $<
fixed_bm.o: fixed_bm.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(fixed_bm_CFLAGS) -c $<
