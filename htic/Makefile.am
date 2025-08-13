## Process this file with automake to produce Makefile.in

if DUMB_CONFIG_HERETIC
pkgdata_DATA = htic4dum.wad
else
pkgdata_DATA = 
endif

CLEANFILES = $(h4d_lumps) PLAYPAL.lump

h4d_pt_lumps = ANIMTEX.lump GETTABLE.lump LEVINFO.lump LINETYPE.lump \
 PHASES.lump PROTOS.lump SECTTYPE.lump SOUNDS.lump
h4d_lumps = $(h4d_pt_lumps) COLORM16.lump COLORM32.lump DUMBLOGO.lump

# Keep htic.pt first in this list so $< works right.
h4d_sources = htic.pt ambient.pt animtex.pt furniture.pt generic.pt \
 gettables.pt keys.pt levels.pt missiles.pt monsters.pt weapons.pt

wadtool = ../tool/wadtool
ppmtodumb = ../tool/ppmtodumb
ptcomp = ../tool/ptcomp
pt_preprocess = cpp -lang-c++ -I$(srcdir)
# or: pt_preprocess = $(CPP) -x c-header -I$(srcdir)

PLAYPAL.lump: $(DUMB_CONFIG_HERETIC_WAD) $(wadtool)
	$(wadtool) -r $< -x PLAYPAL

# ../docs/dumblogo.ppm is searched in VPATH and $< refers to it.
DUMBLOGO.lump: ../docs/dumblogo.ppm PLAYPAL.lump $(ppmtodumb)
	$(ppmtodumb) -p -M PLAYPAL.lump <$< >$@

$(h4d_pt_lumps): $(h4d_sources) $(ptcomp)
	$(pt_preprocess) -DHERETIC $< | $(ptcomp) .

COLORM16.lump: PLAYPAL.lump $(ppmtodumb)
	$(ppmtodumb) -c -2 -M $< >$@

COLORM32.lump: PLAYPAL.lump $(ppmtodumb)
	$(ppmtodumb) -c -4 -M $< >$@

htic4dum.wad: $(wadtool) $(h4d_lumps)
	$(wadtool) -w $@ -f $(h4d_lumps)

EXTRA_DIST = $(h4d_sources)
