## Process this file with automake to produce Makefile.in

EXTRA_DIST = WADTOOL.PLAN .cvsignore

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
noinst_PROGRAMS = $(x_noinst_programs)

x_ldadd = $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)

ppm_ldadd = @DUMB_PPM_LDADD@

# DUMB libraries are in the build tree, so don't use $(srcdir).
LDADD = ../libdumb/libdumb.a ../libdumbwad/libdumbwad.a			\
../libdumbutil/libdumbutil.a ../libmissing/libmissing.a $(INTLLIBS)

mkdfnt_LDADD = $(LDADD) $(x_ldadd)

ppmtodumb_LDADD = $(LDADD) $(ppm_ldadd)

dark2trans_LDADD = $(LDADD) $(ppm_ldadd)

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libmissing -I../intl
