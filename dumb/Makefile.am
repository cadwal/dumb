## Process this file with automake to produce Makefile.in

render_objs = $(DUMB_RENDER_SOURCES:.c=.o)
fbrerend_objs = $(DUMB_FBREREND_SOURCES:.c=.o)
network_objs = $(DUMB_NETWORK_SOURCES:.c=.o)

# These sources aren't currently used at all but I think it's better to
# keep them in the distribution.
EXTRA_DIST = vgagl_video.c

if DUMB_CONFIG_AALIB
aalib_bin_programs = aadumb
else
aalib_bin_programs =
endif

if DUMB_CONFIG_DUMMYVIDEO
dummy_bin_programs = dummydumb
else
dummy_bin_programs = 
endif

if DUMB_CONFIG_GGI
ggi_bin_programs = ggidumb
else
ggi_bin_programs = 
endif

if DUMB_CONFIG_SVGALIB
svgalib_bin_programs = ldumb
else
svgalib_bin_programs = 
endif

if DUMB_CONFIG_X
x_bin_programs = xdumb
else
x_bin_programs = 
endif

if DUMB_CONFIG_DOS
dos_bin_programs = dosdumb
else
dos_bin_programs =
endif

bin_PROGRAMS = $(aalib_bin_programs) $(dummy_bin_programs) \
 $(ggi_bin_programs) $(svgalib_bin_programs) $(x_bin_programs) \
 $(dos_bin_programs)

core_sources = dumb.c animtex.c animtex.h bangbang.c banner.c banner.h \
 bogothing.c bogothing.h dyncode.c dyncode.h game.c game.h gettable.c \
 gettable.h levdata.c levdata.h levdyn.c levdyn.h levinfo.c levinfo.h \
 linetype.c linetype.h netio.c netio.h netplay.c netplay.h termtype.c \
 thinghit.c thingm.c things.c things.h updmap.c updmap.h updthing.c useitem.c \
 ctlkey.c ctlkey.h keymap.c keymap.h keymapconf.c keymapconf.h \
 $(DUMB_FBREREND_SOURCES) fbrerend.h \
 $(DUMB_RENDER_SOURCES) render.h renderc.h rendcore.h rendflat.h rendsl.h \
 rendsl86.h rendcommon.c draw.c draw.h \
 $(DUMB_NETWORK_SOURCES) net.h \
 input.h video.h

keyname_sources = keyname.c keyname.h

extra_sources = render8.c render16.c render32.c \
 generic_fbrerend.c le32_fbrerend.c le64_fbrerend.c \
 dummy_net.c unix_net.c

dummy_ldadd = 
ggi_ldadd = -lggi
svgalib_ldadd = -lvga
x_ldadd = $(X_LIBS) $(X_PRE_LIBS) \
 $(DUMB_DGA_LDADD) $(DUMB_XSHM_LDADD) $(DUMB_XKB_LDADD) \
 -lX11 $(X_EXTRA_LIBS)
aalib_ldadd = -laa -lncurses $(x_ldadd)
dos_ldadd = -lalleg

aadumb_SOURCES = aalib_video.c aalib_input.c $(keyname_sources) \
 $(core_sources)
EXTRA_aadumb_SOURCES = $(extra_sources)

dummydumb_SOURCES = dummy_video.c dummy_input.c keymapdummy.c \
 $(core_sources)
EXTRA_dummydumb_SOURCES = $(extra_sources)

ggidumb_SOURCES = ggi_video.c $(keyname_sources) \
 $(core_sources)
EXTRA_ggidumb_SOURCES = $(extra_sources)

ldumb_SOURCES = linux_video.c linux_input.c keymapdummy.c \
 $(core_sources)
EXTRA_ldumb_SOURCES = $(extra_sources)

xdumb_SOURCES = x11_video.c $(keyname_sources) \
 $(core_sources)
EXTRA_xdumb_SOURCES = $(extra_sources)

dosdumb_SOURCES = dos_video.c dos_input.c keymapdummy.c \
 $(core_sources)
EXTRA_dosdumb_SOURCES = $(extra_sources)

# Objects and DUMB libraries are in the build tree, so don't use $(srcdir).
LDADD = $(render_objs) $(fbrerend_objs) $(network_objs) \
 ../libdumb/libdumb.a \
 ../libdumbwad/libdumbwad.a ../libdumbutil/libdumbutil.a \
 $(DUMB_CONFIG_SOUND_LDADD) $(DUMB_NETWORK_LDADD) $(INTLLIBS)

dependencies = $(render_objs) $(fbrerend_objs) $(network_objs) \
 ../libdumb/libdumb.a \
 ../libdumbwad/libdumbwad.a ../libdumbutil/libdumbutil.a

aadumb_LDADD = $(LDADD) $(aalib_ldadd)
aadumb_DEPENDENCIES = $(dependencies)

dummydumb_LDADD = $(LDADD) $(dummy_ldadd)
dummydumb_DEPENDENCIES = $(dependencies)

ggidumb_LDADD = $(LDADD) $(ggi_ldadd)
ggidumb_DEPENDENCIES = $(dependencies)

ldumb_LDADD = $(LDADD) $(svgalib_ldadd)
ldumb_DEPENDENCIES = $(dependencies)

xdumb_LDADD = $(LDADD) $(x_ldadd)
xdumb_DEPENDENCIES = $(dependencies)

dosdumb_LDADD = $(LDADD) $(dos_ldadd)
dosdumb_DEPENDENCIES = $(dependencies)


CC = $(CXX_HACK_CC)

INCLUDES = -I$(top_srcdir) -I../intl
