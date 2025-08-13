## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdumbutil.a

libdumbutil_a_SOURCES = confargs.c confeng.c conffile.c confhelp.c \
 fixed.c log.c safeio.c safem.c timer.c \
 confargs.h confdef.h confeng.h conffile.h confhelp.h \
 endiantypes.h fixed.h log.h safeio.h safem.h timer.h

INCLUDES = -I$(top_srcdir)

# mcount() in Debian GNU/Linux libc 5.4.33-3 seems to have a bug that
# messes up parameters when using both -pg and
# __attribute__((regparm)).  So I have a special set of CFLAGS for
# fixed.c.
fixed_CFLAGS = -O2
fixed.o: fixed.c
	$(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(fixed_CFLAGS) -c $<
