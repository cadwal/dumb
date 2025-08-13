
### Choose your platform

# HPUX on codex.anu.edu.au
#CC=gcc
#RM=rm -f
#PLATFLAGS=-pipe -I/usr/include/X11R6 -DNO_MMAP -DBENDIAN
#BINARIES=xdumb XWad $(GAME).wad
#SOUND=dummy
#FBREREND=generic
#XLIBS=-lX11 -lXext
# required for -DBENDIAN to work!
#PLUSPLUSFLAGS=-x c++ -fenum-int-equiv

# DEC alpha / Digital UNIX 4.0
#CC=gcc -D_XOPEN_SOURCE_EXTENDED
#RM=rm -f
#PLATFLAGS=-DNO_1P_KLUDGE -DNO_MMAP -I/misc/hacks/include
#BINARIES=xdumb XWad $(GAME).wad
#SOUND=mme
#FBREREND=le64
#XLIBS=-lX11 -lXext
#PLATLFLAGS=-lmme

# linux-ppc
#CC=gcc
#RM=rm -f
#PLATFLAGS=-pipe -DBENDIAN
#PLATPRODFLAGS=-fomit-frame-pointer -mcpu=powerpc -mmultiple -mstring
#PLATXOPTFLAGS=
#BINARIES=ldumb xdumb XWad XProtoThing $(GAME).wad
#SOUND=linux
#NETWORK=unix
#FBREREND=generic
#XLIBS=-L/usr/X11/lib -lX11 -lXext
#PLUSPLUSFLAGS=-x c++ -fenum-int-equiv

# linux-ix86
CC=gcc
RM=rm -f
PLATFLAGS=-pipe -m486 # you might need -DSYSV, too
PLATPRODFLAGS=-fomit-frame-pointer
PLATXOPTFLAGS=
BINARIES=ldumb xdumb XWad XProtoThing $(GAME).wad
SOUND=linux
NETWORK=unix
FBREREND=le32
XLIBS=-L/usr/X11/lib -lX11 -lXext

# This for MS-DOS (haven't tried it in a while)
#CC=gcc
#RM=del
#PLATFLAGS=-m486 -DNO_MMAP 
#PLATPRODFLAGS=-fomit-frame-pointer
#SOUND=dummy
#FBREREND=le32
#BINARIES=dosdumb


### where is your doom.wad?  (if you want dumb to play it)
DOOMWAD=./doom.wad
HTICWAD=./heretic.wad


### what game do you want to build into a wad?  
### (by default, the one distributed with DUMB)
GAME=game


### platform independent configuration

# -WANT_1BPP, -DWANT_2BPP and -DWANT_4BPP control support of
# different pixel sizes in the renderer.  Leaving them out speeds compilation
# and shrinks the binary.  1BPP is fastest; the others may be
# needed if you don't want to reconfigure your X server just to play with DUMB

CFGFLAGS=-DWANT_2BPP -DWANT_1BPP -DWANT_4BPP

# a host other than the one you run make on to use for "make balanced"
# (we assume that you have DUMB in ~/dumb on this machine)
# you don't need this unless you explicitly invoke "make balanced"

OTHER_HOST=cmbsb77

### Choose your level of optimization

OPTFLAGS=-O2 -ffast-math
XOPTFLAGS=-O3 $(PLATXOPTFLAGS)


### Choose your debugging level

# Debugging
#DFLAGS=-g -DDEBUG
#LDFLAGS=-g

# Profiling (you can either profile & debug, or profile a "production" build)
#DFLAGS=-g -pg -DDEBUG
#DFLAGS=-g -fno-inline-functions $(PLATPRODFLAGS)
#LDFLAGS=-g -pg

# Production
DFLAGS=$(PLATPRODFLAGS)
LDFLAGS=-s


##### NetPBM stuff

# places to look for ppm.h
#PPMINCL=-I../pbmlibs

# stuff to link binaries using libppm with
#PPMLIBS=-L../pbmlibs -lppm -lpgm -lpbm
PPMLIBS=-lppm -lpgm -lpbm

# alternatively, you could link them with objects from your own 
# netpbm source tree
#PPMOBJS=


### No user servicable parts after this point

MOREFLAGS=-Wall -I.

CFLAGS=$(PLATFLAGS) $(OPTFLAGS) $(DFLAGS) $(MOREFLAGS) $(CFGFLAGS)\
       $(PPMINCL) $(PLUSPLUSFLAGS)
LFLAGS=$(PLATFLAGS) $(LDFLAGS) $(PLATLFLAGS)

# this division is for compilation load balancing between two processors
# try to keep the groups as mixed as possible 

SOUND_OBJ=plat/$(SOUND)_sound.o

NETWORK_OBJ=plat/$(NETWORK)_net.o

COREOBJS1=dumb/dumb.o lib/safem.o wad/wadio.o dumb/levdata.o lib/fixed.o \
 render/view.o dumb/thingm.o render/draw.o dumb/banner.o dumb/useitem.o \
 dumb/thinghit.o dumb/animtex.o dumb/bangbang.o render/render.o \
 dumb/linetype.o lib/timer.o dumb/levinfo.o lib/conf.o \
 plat/$(FBREREND)_fbrerend.o dumb/netplay.o dumb/netio.o $(SOUND_OBJ)

COREOBJS2=dumb/texture.o dumb/updmap.o dumb/updthing.o render/render16.o \
 render/render32.o lib/safeio.o dumb/things.o dumb/gettable.o \
 wad/loadlump.o dumb/levdyn.o dumb/prothing.o lib/log.o dumb/game.o \
 dumb/dsound.o dumb/termtype.o dumb/dyncode.o dumb/bogothing.o $(NETWORK_OBJ)

COREOBJS=$(COREOBJS1) $(COREOBJS2)

TOOL_OBJS=wad/wadio.o wad/loadlump.o wad/wadwr.o tool/wadtool.o lib/log.o \
 lib/safem.o lib/safeio.o

PPMTO_OBJS=tool/ppmtodumb.o $(PPMOBJS)

D2T_OBJS=tool/dark2trans.o $(PPMOBJS)

PTCOMP_OBJS=tool/ptcomp.o lib/log.o lib/safem.o

MKDFNT_OBJS=tool/mkdfnt.o

MKPNAMES_OBJS=tool/mkpnames.o

XWADL_OBJS=wad/wadio.o wad/loadlump.o \
 lib/log.o lib/safem.o lib/safeio.o lib/fixed.o \
 dumb/things.o dumb/texture.o
XWAD1_OBJS= xwad/xwadctls.o xwad/controls.o xwad/colour.o xwad/connect.o \
 xwad/choose.o wad/wadwr.o xwad/xtexture.o
XWAD2_OBJS= xwad/xwad.o xwad/drawmap.o xwad/mapcon.o xwad/savemap.o \
 xwad/disphash.o xwad/tchoose.o
XWAD_OBJS= $(XWADL_OBJS) $(XWAD1_OBJS) $(XWAD2_OBJS)

XPROTO_OBJS= $(XWADL_OBJS) xwad/controls.o xwad/colour.o  xwad/choose.o \
 xwad/disphash.o xwad/xproto.o xwad/xproctls.o dumb/prothing.o \
 xwad/xtexture.o dumb/dsound.o $(SOUND_OBJ)

GGIDUMB_OBJS=$(COREOBJS) plat/ggi_video.o

LDUMB_OBJS=$(COREOBJS) plat/linux_video.o plat/linux_input.o

XDUMB_OBJS=$(COREOBJS) plat/x11_video.o

DDUMB_OBJS=$(COREOBJS) plat/dummy_video.o plat/dummy_input.o

AADUMB_OBJS=$(COREOBJS) plat/aalib_video.o plat/aalib_input.o

DUMB_OBJS=$(COREOBJS) plat/dosvideo.o plat/dosinput.o


all: $(BINARIES)

depend:
	makedepend -Y -- $(CFLAGS) -- */*.c 2>/dev/null

cclean:
	$(RM) dumb/*.o lib/*.o wad/*o tool/*.o xwad/*.o plat/*.o render/*.o

clean: cclean
	$(RM) $(BINARIES)
	$(RM) $(GAME)/data/*.wad $(GAME)/data/*.lump 
	$(RM) $(GAME)/data/*.flat $(GAME)/data/*.patch 
	$(RM) data/*.ppm
	$(RM) $(GAME)/data/*.ppm
	$(RM) $(GAME)/gfx/*.flat $(GAME)/gfx/*.patch $(GAME)/gfx/*.?.ppm
	$(RM) doom/*.lump doom/*.wad
	$(RM) htic/*.lump htic/*.wad

spotless: clean
	$(RM) *~
	$(RM) docs/*~ dumb/*~ lib/*~ wad/*~ xwad/*~ plat/*~ render/*~ tool/*~
	$(RM) $(GAME)/gfx/*~ $(GAME)/misc/*~ 
	$(RM) doom/*~ htic/*~
	$(RM) $(GAME)/lvl/*~
	$(RM) */*.orig */*.rej */*/*.orig */*/*.rej

SRCDIST_STUFF=docs/* dumb/* lib/* wad/* xwad/* plat/* render/* tool/* data/* doom/* htic/* Makefile

zipdist: spotless
	zip -r9 dumbsrc.zip $(SRCDIST_STUFF)

tardist: spotless
	tar -czf dumbsrc.tar.gz $(SRCDIST_STUFF)

gamedist: spotless
	zip -r9 dumbgame.zip game/*

bindist: XWad ldumb xdumb doom4dum.wad htic4dum.wad $(GAME).wad docs/README.dumb XProtoThing
	zip -j9 dumbbin.zip $^

wadtool: $(TOOL_OBJS)
	$(CC) $(LFLAGS) -o $@ $^

ppmtodumb: $(PPMTO_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(PPMLIBS)

dark2trans: $(D2T_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(PPMLIBS)

ptcomp: $(PTCOMP_OBJS)
	$(CC) $(LFLAGS) -o $@ $^

mkpnames: $(MKPNAMES_OBJS)
	$(CC) $(LFLAGS) -o $@ $^

XWad: $(XWAD_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(XLIBS) -lm

XProtoThing: $(XPROTO_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(XLIBS) -lm

mkdfnt: $(MKDFNT_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(XLIBS) -lm

# I have not tested this! (josh)
ggidumb: $(GGIDUMB_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ -lggi -lm

ldumb: $(LDUMB_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ -lvga -lm

xdumb: $(XDUMB_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ $(XLIBS) -lm

aadumb: $(AADUMB_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ -laa -lncurses -lgpm $(XLIBS) -lm

ddumb: $(DDUMB_OBJS)
	$(CC) $(LFLAGS) -o $@ $^ -lm

# I haven't tested this in about a year, either. (josh)
dosdumb: $(DUMB_OBJS)
	$(CC) $(LFLAGS) -o dosdumb $(DUMB_OBJS) -lgrx20 -lm
	stubedit dosdumb.exe bufsize=64K argv0=dosdumb

wadtool.exe: wadtool

ddumb.exe: ddumb

dosdumb.exe: dosdumb

core1: $(COREOBJS1) $(XWAD1_OBJS) tool/ptcomp.o

core2: $(COREOBJS2) $(XWAD2_OBJS)

balanced: 
	rsh -n $(OTHER_HOST) "cd dumb ; make core2" & make core1 ; echo Waiting for $(OTHER_HOST) to finish compiling... ; wait


# fixbm stuff

fixbm: tool/fixed_bm.c lib/fixed.h
	$(CC) -I. -O0 -o fixbm tool/fixed_bm.c

benchmark: fixbm
	cat docs/README.fixbm
	./fixbm


# doom4dum stuff

D4D_LUMPS= doom/COLORM16.lump doom/COLORM32.lump \
 doom/PHASES.lump doom/PROTOS.lump doom/GETTABLE.lump doom/DUMBLOGO.lump

DOOM_PT= doom/doom.pt doom/gettables.pt doom/monsters.pt doom/missiles.pt \
 doom/generic.pt doom/furnishings.pt doom/linetypes.pt doom/animtex.pt \
 doom/weapons.pt

doom/PLAYPAL.lump: wadtool $(DOOMWAD)
	./wadtool -r $(DOOMWAD) -x PLAYPAL
	mv PLAYPAL.lump doom

doom/DUMBLOGO.lump: docs/dumblogo.ppm doom/PLAYPAL.lump ppmtodumb
	./ppmtodumb -p -M doom/PLAYPAL.lump <$< >$@

doom/PHASES.lump doom/PROTOS.lump doom/GETTABLE.lump: $(DOOM_PT) ptcomp
	cpp -lang-c++ -DDOOM -Idoom -I. $< |./ptcomp doom

doom/COLORM16.lump: ppmtodumb doom/PLAYPAL.lump
	./ppmtodumb -c -2 -M doom/PLAYPAL.lump >doom/COLORM16.lump

doom/COLORM32.lump: ppmtodumb doom/PLAYPAL.lump
	./ppmtodumb -c -4 -M doom/PLAYPAL.lump >doom/COLORM32.lump

doom4dum.wad: wadtool $(D4D_LUMPS)
	./wadtool -w $@ -f doom/WAFFLE doom/*.lump


# htic4dum stuff

H4D_LUMPS= htic/COLORM16.lump htic/COLORM32.lump \
 htic/PHASES.lump htic/PROTOS.lump htic/GETTABLE.lump htic/DUMBLOGO.lump

HTIC_PT= htic/htic.pt htic/keys.pt htic/animtex.pt doom/linetypes.pt \
         htic/furniture.pt htic/monsters.pt htic/missiles.pt htic/weapons.pt \
	 htic/ambient.pt

htic/PLAYPAL.lump: wadtool $(HTICWAD)
	./wadtool -r $(HTICWAD) -x PLAYPAL
	mv PLAYPAL.lump htic

htic/DUMBLOGO.lump: docs/dumblogo.ppm htic/PLAYPAL.lump ppmtodumb
	./ppmtodumb -p -M htic/PLAYPAL.lump <$< >$@

htic/PHASES.lump htic/PROTOS.lump htic/GETTABLE.lump: $(HTIC_PT) ptcomp
	cpp -lang-c++ -DHERETIC -Ihtic -I. $< |./ptcomp htic

htic/COLORM16.lump: ppmtodumb htic/PLAYPAL.lump
	./ppmtodumb -c -2 -M htic/PLAYPAL.lump >htic/COLORM16.lump

htic/COLORM32.lump: ppmtodumb htic/PLAYPAL.lump
	./ppmtodumb -c -4 -M htic/PLAYPAL.lump >htic/COLORM32.lump

htic4dum.wad: wadtool $(H4D_LUMPS)
	./wadtool -w $@ -f htic/*.lump


# wadfile building stuff

game_FLATS= game/gfx/woodflor.flat game/gfx/ice.flat \
game/gfx/fplnk.flat game/gfx/tiles1.flat game/gfx/fplnk.r.flat \
game/gfx/whmarb.flat game/gfx/brikflor.flat game/gfx/rockflor.flat \
game/gfx/woodflo2.flat

game_PATCHES= game/gfx/gargoyle.patch game/gfx/lplnk.patch \
game/gfx/lplnk.r.patch game/gfx/stg.patch \
game/gfx/stonwalb.patch game/gfx/stonwals.patch game/gfx/rufbrick.patch \
game/gfx/rockflor.patch

game_SPRITES= game/gfx/newsa0.patch \
game/gfx/fbala0.patch game/gfx/fbalb0.patch game/gfx/fbalc0.patch \
game/gfx/fbald0.patch game/gfx/fbale0.patch game/gfx/fbalf0.patch \
game/gfx/fbalg0.patch

%/data/pal-tail.ppm:
	ppmmake "#000001" 1 1 |pnmdepth 255 >$@

%/data/pal-nt.ppm: 
	pnmcat -tb $*/gfx/*.ppm | ppmquant 255 | ppmtomap | ppmchange "#000001" "#000000" | pnmdepth 255 >$@

%/data/palette.ppm: %/data/pal-nt.ppm %/data/pal-tail.ppm
	pnmcat -lr $^ >$@

%/data/PLAYPAL.lump: %/data/palette.ppm ppmtodumb
	./ppmtodumb -m <$< >$@

%/data/PNAMES.lump %/data/TEXTURE1.lump: %/misc/textures mkpnames
	./mkpnames $< $*/data/PNAMES.lump $*/data/TEXTURE1.lump

%/data/COLORMAP.lump: %/data/PLAYPAL.lump ppmtodumb 
	./ppmtodumb -c -M $< >$@

%/data/COLORM16.lump: %/data/PLAYPAL.lump ppmtodumb 
	./ppmtodumb -c -2 -M $< >$@

%/data/COLORM32.lump: %/data/PLAYPAL.lump ppmtodumb 
	./ppmtodumb -c -4 -M $< >$@

%/data/PHASES.lump %/data/PROTOS.lump %/data/GETTABLE.lump: %/misc/protos.pt ptcomp
	cpp -lang-c++ -I$*/misc $< |./ptcomp $*/data


%.r.ppm : %.ppm
	pnmflip -ccw $< >$@

data/tile-64.ppm :
	ppmmake "#d0d0d0" 60 60 |pnmmargin -black 2 |ppmrelief >$@

data/tile-128.ppm :
	ppmmake "#d0d0d0" 124 124 |pnmmargin -black 2 |ppmrelief >$@

%.t.ppm : %.ppm data/tile-64.ppm
	pnmarith -multiply data/tile-64.ppm $< |ppmbrighten -value +96 >$@

%.u.ppm : %.ppm data/tile-128.ppm
	pnmarith -multiply data/tile-128.ppm $< |ppmbrighten -value +96 >$@

%.flat : %.ppm ppmtodumb $(GAME)/data/PLAYPAL.lump $(GAME)/data/palette.ppm
	ppmquant -fs -map $(GAME)/data/palette.ppm $< | ./ppmtodumb -f -M $(GAME)/data/PLAYPAL.lump >$@

%.patch : %.ppm ppmtodumb $(GAME)/data/PLAYPAL.lump $(GAME)/data/palette.ppm
	ppmquant -fs -map $(GAME)/data/palette.ppm $< | ./ppmtodumb -p -M $(GAME)/data/PLAYPAL.lump >$@

MISCLUMPS=$(GAME)/data/WADINFO $(GAME)/data/PLAYPAL.lump \
$(GAME)/data/COLORM16.lump $(GAME)/data/COLORMAP.lump \
$(GAME)/data/COLORM32.lump \
$(GAME)/data/PNAMES.lump $(GAME)/data/TEXTURE1.lump \
$(GAME)/data/PHASES.lump $(GAME)/data/PROTOS.lump \
$(GAME)/data/GETTABLE.lump $(GAME)/data/LINETYPE.lump

FLATS=$($(GAME)_FLATS)
SPRITES=$($(GAME)_SPRITES)
PATCHES=$($(GAME)_PATCHES)

$(GAME).wad: wadtool $(MISCLUMPS) $(FLATS) $(PATCHES) $(SPRITES)
	./wadtool -W $@ \
	-f $(MISCLUMPS) \
	-L P_START -f $(PATCHES) -L P_END \
	-L F_START -f $(FLATS) -L F_END \
	-L S_START -f $(SPRITES) -L S_END \
	-c $(GAME)/lvl/*.wad

dumb/bangbang.o: dumb/bangbang.c
	$(CC) $(CFLAGS) $(XOPTFLAGS) -c dumb/bangbang.c -o dumb/bangbang.o

render/render.o: render/render.c render/flrender.c render/slice.c render/rendcore.c
	$(CC) $(CFLAGS) $(XOPTFLAGS) -c render/render.c -DBPP=1 -o render/render.o

render/render16.o: render/render.c render/flrender.c render/slice.c render/rendcore.c
	$(CC) $(CFLAGS) $(XOPTFLAGS) -c render/render.c -DBPP=2 -o render/render16.o

render/render32.o: render/render.c render/flrender.c render/slice.c render/rendcore.c
	$(CC) $(CFLAGS) $(XOPTFLAGS) -c render/render.c -DBPP=4 -o render/render32.o

# makedepend output follows

# DO NOT DELETE

dumb/animtex.o: lib/safem.h lib/log.h wad/wadio.h dumb/animtex.h
dumb/animtex.o: dumb/texture.h
dumb/bangbang.o: lib/log.h dumb/things.h render/view.h lib/fixed.h
dumb/bangbang.o: dumb/levdyn.h dumb/levdata.h wad/wadstruct.h lib/endian.h
dumb/bangbang.o: wad/wadio.h dumb/texture.h dumb/prothing.h
dumb/banner.o: lib/safem.h lib/log.h render/draw.h dumb/texture.h wad/wadio.h
dumb/banner.o: dumb/banner.h dumb/texture.h
dumb/bogothing.o: render/draw.h dumb/texture.h wad/wadio.h dumb/things.h
dumb/bogothing.o: render/view.h lib/fixed.h dumb/levdyn.h dumb/levdata.h
dumb/bogothing.o: wad/wadstruct.h lib/endian.h dumb/texture.h dumb/prothing.h
dumb/bogothing.o: dumb/bogothing.h
dumb/dsound.o: wad/wadstruct.h lib/endian.h wad/wadio.h lib/log.h lib/safem.h
dumb/dsound.o: plat/sound.h lib/fixed.h dumb/dsound.h render/view.h
dumb/dumb.o: lib/log.h lib/safem.h wad/wadio.h lib/timer.h lib/conf.h
dumb/dumb.o: dumb/texture.h dumb/gettable.h lib/fixed.h lib/endian.h
dumb/dumb.o: dumb/levdata.h wad/wadstruct.h plat/video.h render/render.h
dumb/dumb.o: render/view.h dumb/levdata.h plat/fbrerend.h render/draw.h
dumb/dumb.o: dumb/texture.h dumb/banner.h dumb/things.h render/view.h
dumb/dumb.o: dumb/levdyn.h dumb/prothing.h dumb/updmap.h dumb/dsound.h
dumb/dumb.o: dumb/linetype.h dumb/levinfo.h dumb/netplay.h plat/input.h
dumb/dumb.o: dumb/netio.h dumb/bogothing.h dumb/game.h plat/sound.h
dumb/dumb.o: plat/net.h dumb/netplay.h
dumb/dyncode.o: dumb/levdyn.h dumb/levdata.h lib/fixed.h wad/wadstruct.h
dumb/dyncode.o: lib/endian.h wad/wadio.h dumb/texture.h dumb/prothing.h
dumb/dyncode.o: dumb/things.h render/view.h dumb/dyncode.h
dumb/game.o: lib/log.h dumb/things.h render/view.h lib/fixed.h dumb/levdyn.h
dumb/game.o: dumb/levdata.h wad/wadstruct.h lib/endian.h wad/wadio.h
dumb/game.o: dumb/texture.h dumb/prothing.h dumb/gettable.h dumb/netplay.h
dumb/game.o: plat/input.h lib/conf.h dumb/netio.h dumb/game.h
dumb/gettable.o: wad/wadio.h lib/safem.h lib/log.h dumb/texture.h
dumb/gettable.o: render/draw.h dumb/texture.h dumb/banner.h dumb/levdata.h
dumb/gettable.o: lib/fixed.h wad/wadstruct.h lib/endian.h dumb/things.h
dumb/gettable.o: render/view.h dumb/levdyn.h dumb/prothing.h dumb/dsound.h
dumb/gettable.o: dumb/gettable.h dumb/game.h plat/input.h lib/conf.h
dumb/levdata.o: lib/log.h lib/safem.h wad/wadio.h dumb/levdata.h lib/fixed.h
dumb/levdata.o: wad/wadstruct.h lib/endian.h dumb/texture.h dumb/levdyn.h
dumb/levdata.o: dumb/prothing.h dumb/dyncode.h dumb/netplay.h plat/input.h
dumb/levdata.o: lib/conf.h dumb/netio.h
dumb/levdyn.o: lib/log.h lib/fixed.h dumb/things.h render/view.h
dumb/levdyn.o: dumb/levdyn.h dumb/levdata.h wad/wadstruct.h lib/endian.h
dumb/levdyn.o: wad/wadio.h dumb/texture.h dumb/prothing.h dumb/levinfo.h
dumb/levdyn.o: dumb/animtex.h
dumb/levinfo.o: lib/log.h wad/wadio.h dumb/levinfo.h lib/endian.h
dumb/levinfo.o: dumb/levdata.h lib/fixed.h wad/wadstruct.h dumb/texture.h
dumb/linetype.o: lib/log.h wad/wadio.h dumb/linetype.h lib/endian.h
dumb/linetype.o: lib/fixed.h dumb/levdata.h wad/wadstruct.h dumb/texture.h
dumb/netio.o: lib/log.h dumb/netio.h plat/net.h lib/conf.h dumb/netplay.h
dumb/netplay.o: lib/log.h lib/safem.h lib/timer.h dumb/game.h plat/input.h
dumb/netplay.o: lib/conf.h lib/endian.h dumb/levdata.h lib/fixed.h
dumb/netplay.o: wad/wadstruct.h wad/wadio.h dumb/texture.h dumb/netplay.h
dumb/netplay.o: dumb/netio.h
dumb/prothing.o: lib/log.h lib/safem.h wad/wadio.h dumb/prothing.h
dumb/prothing.o: lib/fixed.h lib/endian.h dumb/texture.h
dumb/termtype.o: lib/log.h dumb/levdyn.h dumb/levdata.h lib/fixed.h
dumb/termtype.o: wad/wadstruct.h lib/endian.h wad/wadio.h dumb/texture.h
dumb/termtype.o: dumb/prothing.h dumb/linetype.h
dumb/texture.o: lib/log.h lib/safem.h dumb/texture.h wad/wadio.h
dumb/texture.o: wad/wadstruct.h lib/endian.h dumb/pastepic.c
dumb/thinghit.o: lib/log.h dumb/levdyn.h dumb/levdata.h lib/fixed.h
dumb/thinghit.o: wad/wadstruct.h lib/endian.h wad/wadio.h dumb/texture.h
dumb/thinghit.o: dumb/prothing.h dumb/things.h render/view.h dumb/updmap.h
dumb/thinghit.o: dumb/game.h plat/input.h lib/conf.h dumb/gettable.h
dumb/thinghit.o: dumb/linetype.h
dumb/thingm.o: lib/log.h lib/fixed.h dumb/dsound.h lib/endian.h render/view.h
dumb/thingm.o: dumb/things.h dumb/levdyn.h dumb/levdata.h wad/wadstruct.h
dumb/thingm.o: wad/wadio.h dumb/texture.h dumb/prothing.h
dumb/things.o: lib/log.h dumb/things.h render/view.h lib/fixed.h
dumb/things.o: dumb/levdyn.h dumb/levdata.h wad/wadstruct.h lib/endian.h
dumb/things.o: wad/wadio.h dumb/texture.h dumb/prothing.h
dumb/updmap.o: lib/log.h dumb/levdyn.h dumb/levdata.h lib/fixed.h
dumb/updmap.o: wad/wadstruct.h lib/endian.h wad/wadio.h dumb/texture.h
dumb/updmap.o: dumb/prothing.h dumb/animtex.h dumb/dsound.h render/view.h
dumb/updmap.o: dumb/updmap.h dumb/things.h dumb/linetype.h dumb/game.h
dumb/updmap.o: plat/input.h lib/conf.h
dumb/updthing.o: lib/log.h lib/timer.h dumb/levdata.h lib/fixed.h
dumb/updthing.o: wad/wadstruct.h lib/endian.h wad/wadio.h dumb/texture.h
dumb/updthing.o: dumb/gettable.h dumb/things.h render/view.h dumb/levdyn.h
dumb/updthing.o: dumb/prothing.h
dumb/useitem.o: dumb/things.h render/view.h lib/fixed.h dumb/levdyn.h
dumb/useitem.o: dumb/levdata.h wad/wadstruct.h lib/endian.h wad/wadio.h
dumb/useitem.o: dumb/texture.h dumb/prothing.h dumb/gettable.h
lib/conf.o: lib/conf.h
lib/fixed.o: lib/fixed.h
lib/log.o: lib/log.h
lib/safeio.o: lib/log.h lib/safeio.h lib/safem.h
lib/safem.o: lib/log.h lib/safem.h
lib/timer.o: lib/timer.h
plat/aalib_input.o: lib/log.h plat/input.h lib/conf.h lib/endian.h
plat/aalib_video.o: lib/log.h plat/video.h lib/conf.h
plat/dosinput.o: plat/input.h lib/conf.h lib/endian.h
plat/dosvideo.o: lib/log.h plat/video.h lib/conf.h lib/safem.h
plat/dummy_input.o: plat/input.h lib/conf.h lib/endian.h
plat/dummy_net.o: lib/log.h plat/net.h lib/conf.h dumb/netplay.h
plat/dummy_sound.o: lib/log.h plat/sound.h lib/fixed.h
plat/dummy_video.o: lib/log.h plat/video.h lib/conf.h lib/safem.h
plat/generic_fbrerend.o: render/render.h render/view.h lib/fixed.h
plat/generic_fbrerend.o: dumb/levdata.h plat/fbrerend.h
plat/ggi_video.o: lib/safem.h lib/log.h plat/input.h lib/conf.h lib/endian.h
plat/ggi_video.o: plat/video.h
plat/le32_fbrerend.o: render/render.h render/view.h lib/fixed.h
plat/le32_fbrerend.o: dumb/levdata.h plat/fbrerend.h
plat/le64_fbrerend.o: render/render.h render/view.h lib/fixed.h
plat/le64_fbrerend.o: dumb/levdata.h plat/fbrerend.h
plat/linux_input.o: lib/log.h plat/input.h lib/conf.h lib/endian.h
plat/linux_sound.o: lib/log.h lib/safem.h plat/sound.h lib/fixed.h
plat/linux_video.o: lib/safem.h lib/log.h plat/video.h lib/conf.h
plat/mme_sound.o: lib/log.h lib/safem.h plat/sound.h lib/fixed.h
plat/unix_net.o: lib/log.h lib/safem.h plat/net.h lib/conf.h dumb/netplay.h
plat/vgagl_video.o: lib/safem.h lib/log.h plat/video.h lib/conf.h
plat/x11_video.o: lib/log.h lib/safem.h plat/input.h lib/conf.h lib/endian.h
plat/x11_video.o: plat/video.h
render/draw.o: render/draw.h dumb/texture.h wad/wadio.h
render/render.o: wad/wadio.h lib/log.h lib/fixed.h render/view.h
render/render.o: dumb/levdyn.h dumb/levdata.h wad/wadstruct.h lib/endian.h
render/render.o: dumb/texture.h dumb/prothing.h lib/safem.h render/render.h
render/render.o: dumb/levdata.h dumb/things.h render/view.h dumb/levdyn.h
render/view.o: lib/fixed.h lib/log.h render/view.h
tool/fixed_bm.o: lib/fixed.h
tool/mknulmap.o: lib/safem.h lib/log.h wad/wadwr.h wad/wadstruct.h
tool/mknulmap.o: lib/endian.h wad/wadstruct.h
tool/mkpnames.o: lib/endian.h wad/wadstruct.h
tool/ppmtodumb.o: wad/wadstruct.h lib/endian.h
tool/ptcomp.o: lib/timer.h lib/log.h lib/safem.h dumb/levdata.h
tool/ptcomp.o: dumb/prothing.h lib/fixed.h lib/endian.h dumb/texture.h
tool/ptcomp.o: wad/wadio.h dumb/gettable.h dumb/levdata.h wad/wadstruct.h
tool/ptcomp.o: dumb/linetype.h dumb/animtex.h dumb/dsound.h render/view.h
tool/ptcomp.o: dumb/levinfo.h
tool/wadtool.o: lib/log.h wad/wadio.h wad/wadwr.h wad/wadstruct.h
tool/wadtool.o: lib/endian.h
wad/loadlump.o: wad/wadstruct.h lib/endian.h wad/wadio.h lib/safeio.h
wad/loadlump.o: lib/safem.h lib/log.h
wad/wadio.o: wad/wadstruct.h lib/endian.h wad/wadio.h lib/safeio.h
wad/wadio.o: lib/safem.h lib/log.h
wad/wadwr.o: lib/log.h wad/wadwr.h wad/wadstruct.h lib/endian.h
xwad/choose.o: xwad/controls.h xwad/choose.h xwad/colour.h xwad/disphash.h
xwad/colour.o: xwad/xwad.h lib/fixed.h wad/wadstruct.h lib/endian.h
xwad/colour.o: wad/wadio.h xwad/controls.h xwad/choose.h xwad/colour.h
xwad/connect.o: lib/log.h lib/safem.h xwad/xwad.h lib/fixed.h wad/wadstruct.h
xwad/connect.o: lib/endian.h wad/wadio.h xwad/controls.h xwad/choose.h
xwad/controls.o: lib/log.h lib/safem.h xwad/controls.h xwad/colour.h
xwad/controls.o: xwad/disphash.h
xwad/disphash.o: xwad/disphash.h
xwad/drawmap.o: xwad/xwad.h lib/fixed.h wad/wadstruct.h lib/endian.h
xwad/drawmap.o: wad/wadio.h xwad/controls.h xwad/choose.h xwad/colour.h
xwad/drawmap.o: xwad/xwadmap.h
xwad/mapcon.o: lib/log.h xwad/xwad.h lib/fixed.h wad/wadstruct.h lib/endian.h
xwad/mapcon.o: wad/wadio.h xwad/controls.h xwad/choose.h xwad/xwadmap.h
xwad/savemap.o: lib/log.h lib/safem.h wad/wadwr.h wad/wadstruct.h
xwad/savemap.o: lib/endian.h wad/wadstruct.h xwad/xwad.h lib/fixed.h
xwad/savemap.o: wad/wadio.h xwad/controls.h xwad/choose.h
xwad/tchoose.o: xwad/xtexture.h dumb/texture.h wad/wadio.h xwad/controls.h
xwad/tchoose.o: xwad/choose.h xwad/colour.h xwad/disphash.h xwad/xwad.h
xwad/tchoose.o: lib/fixed.h wad/wadstruct.h lib/endian.h
xwad/xproctls.o: xwad/xproto.h lib/fixed.h wad/wadio.h dumb/prothing.h
xwad/xproctls.o: lib/endian.h dumb/texture.h xwad/controls.h xwad/choose.h
xwad/xproto.o: lib/log.h wad/wadio.h dumb/dsound.h lib/fixed.h lib/endian.h
xwad/xproto.o: render/view.h plat/sound.h xwad/xproto.h dumb/prothing.h
xwad/xproto.o: dumb/texture.h xwad/controls.h xwad/choose.h xwad/colour.h
xwad/xproto.o: xwad/disphash.h xwad/xtexture.h dumb/texture.h
xwad/xtexture.o: xwad/xtexture.h dumb/texture.h wad/wadio.h
xwad/xwad.o: lib/log.h lib/safem.h wad/wadio.h xwad/xwad.h lib/fixed.h
xwad/xwad.o: wad/wadstruct.h lib/endian.h xwad/controls.h xwad/choose.h
xwad/xwad.o: xwad/colour.h xwad/disphash.h
xwad/xwadctls.o: xwad/xwad.h lib/fixed.h wad/wadstruct.h lib/endian.h
xwad/xwadctls.o: wad/wadio.h xwad/controls.h xwad/choose.h
