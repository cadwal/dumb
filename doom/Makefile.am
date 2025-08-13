## Process this file with automake to produce Makefile.in

## This should be automatically added by Automake, but 1.2 has a bug.
CXXCPP = @CXXCPP@

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
ptcomp = ../ptcomp/ptcomp

pt_preprocess = $(CXXCPP) -x c-header -I$(srcdir)
# -x c-header is required because $(CXXCPP) is typically g++ -E which
# assumes the .pt files are linker input and thus ignores them.  If
# $(CXXCPP) is something else, -x c-header will cause it to fail.  In
# that case, try this definition (which uses -lang-c++ because of //
# comments and so needs GNU cpp):
#
# pt_preprocess = /lib/cpp -lang-c++ -I$(srcdir)
#
# If you need it, please report, so I can add a check in
# ../configure.in or devise a trickier scheme.  (One idea would be to
# create a temporary C++ file containing just #include <foo.pt> and
# preprocess that.)

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
