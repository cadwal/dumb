## Process this file with automake to produce Makefile.in

EXTRA_DIST = .cvsignore

noinst_LIBRARIES = libdumb.a

libdumb_a_SOURCES = animtexstruct.h dsound.c dsound.h font.c font.h	\
fontinwad.c fontinwad.h fontmap.c fontmap.h fontmapinwad.c		\
fontmapinwad.h gettableinwad.c gettableinwad.h gettablestruct.h		\
levdatanums.h levinfostruct.h linetypestruct.h lumpver.c lumpver.h	\
pastepic.h prothing.c prothing.h prothingstruct.h protoinwad.c		\
protoinwad.h sound.h sound_mix.c sound_mix.h texture.c texture.h	\
view.c view.h

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
