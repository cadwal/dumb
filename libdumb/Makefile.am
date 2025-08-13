## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdumb.a

libdumb_a_SOURCES = dsound.c prothing.c texture.c view.c \
 $(DUMB_CONFIG_SOUND)_sound.c sound_mix.c sound_mix.h \
 dsound.h pastepic.h prothing.h texture.h sound.h view.h \
 animtexstruct.h gettablestruct.h levinfostruct.h linetypestruct.h \
 prothingstruct.h levdatanums.h

EXTRA_libdumb_a_SOURCES = dummy_sound.c dos_sound.c linux_sound.c mme_sound.c

INCLUDES = -I$(top_srcdir)
