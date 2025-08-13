## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore

noinst_LIBRARIES = libdumb.a

libdumb_a_SOURCES = dsound.c prothing.c texture.c view.c sound_mix.c	\
sound_mix.h dsound.h pastepic.h prothing.h texture.h sound.h view.h	\
animtexstruct.h gettablestruct.h levinfostruct.h linetypestruct.h	\
prothingstruct.h protoinwad.c protoinwad.h levdatanums.h

# If I put $(DUMB_CONFIG_SOUND)_sound.c directly in libdumb_a_SOURCES,
# Automake would add .deps/$(DUMB_CONFIG_SOUND)_sound.o in DEP_FILES
# which already has .deps/linux_sound.o etc. from
# EXTRA_libdumb_a_SOURCES.  Then a bug in make-3.77 would cause
# compilation to fail because make doesn't know how to make
# .deps/linux_sound.o.  (It should ignore the missing file, since
# -include is used.  But for some reason, -including the same file
# twice messes things up.)
libdumb_a_DEPENDENCIES = $(DUMB_CONFIG_SOUND)_sound.o
libdumb_a_LIBADD = $(DUMB_CONFIG_SOUND)_sound.o

EXTRA_libdumb_a_SOURCES = dummy_sound.c dos_sound.c linux_sound.c mme_sound.c

CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I../intl
