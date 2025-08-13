## Process this file with automake to produce Makefile.in

if DUMB_CONFIG_DOOM
pkgdata_DATA = doom4dum.wad
else
pkgdata_DATA = 
endif

CLEANFILES = $(d4d_lumps) PLAYPAL.lump doom4dum.wad

d4d_pt_lumps = ANIMTEX.lump GETTABLE.lump LEVINFO.lump LINETYPE.lump \
 PHASES.lump PROTOS.lump SECTTYPE.lump SOUNDS.lump
d4d_lumps = $(d4d_pt_lumps) COLORM16.lump COLORM32.lump DUMBLOGO.lump
# Keep doom.pt first in this list so $< works right.
d4d_sources = doom.pt animtex.pt furnishings.pt generic.pt gettables.pt \
 levels.pt linetypes.pt missiles.pt monsters.pt sectortypes.pt weapons.pt

wadtool = ../tool/wadtool
ppmtodumb = ../tool/ppmtodumb
ptcomp = ../tool/ptcomp
pt_preprocess = cpp -lang-c++ -I$(srcdir)
# or: pt_preprocess = $(CPP) -x c-header -I$(srcdir)

PLAYPAL.lump: $(DUMB_CONFIG_DOOM_WAD) $(wadtool)
	$(wadtool) -r $< -x PLAYPAL

# ../docs/dumblogo.ppm is searched in VPATH and $< refers to it.
DUMBLOGO.lump: ../docs/dumblogo.ppm PLAYPAL.lump $(ppmtodumb)
	$(ppmtodumb) -p -M PLAYPAL.lump <$< >$@

$(d4d_pt_lumps): $(d4d_sources) $(ptcomp)
	$(pt_preprocess) -DDOOM $< | $(ptcomp) .

COLORM16.lump: PLAYPAL.lump $(ppmtodumb)
	$(ppmtodumb) -c -2 -M $< >$@

COLORM32.lump: PLAYPAL.lump $(ppmtodumb)
	$(ppmtodumb) -c -4 -M $< >$@

doom4dum.wad: $(wadtool) $(d4d_lumps)
	$(wadtool) -w $@ -f $(srcdir)/WAFFLE $(d4d_lumps)

EXTRA_DIST = WAFFLE $(d4d_sources)
