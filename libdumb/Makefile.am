## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdumb.a

libdumb_a_SOURCES = dsound.c prothing.c texture.c view.c \
 $(DUMB_CONFIG_SOUND)_sound.c \
 dsound.h pastepic.h prothing.h texture.h sound.h view.h \
 animtexstruct.h gettablestruct.h levinfostruct.h linetypestruct.h \
 prothingstruct.h levdatanums.h

EXTRA_libdumb_a_SOURCES = dummy_sound.c linux_sound.c mme_sound.c

# Now here's a hack...
# I use -MM instead of -M in MKDEP.  This way, I get the dependencies
# built even on machines that don't have all the header files used on
# other machines.  The resulting dependency lists won't include system
# headers but their locations wouldn't be portable anyway.
MKDEP = gcc -MM $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

INCLUDES += -I$(top_srcdir)
