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

# Always use the latest versions.  Use full filenames so targets can
# depend on them and be rebuilt if the executables change.
wadtool = ../tool/wadtool
ppmtodumb = ../tool/ppmtodumb
ptcomp = ../ptcomp/ptcomp

# srcdir/.. is needed so we can get at doom's linetypes, on which
# heretic's are based

pt_preprocess = $(CXXCPP) -x c-header -I$(srcdir) -I$(top_srcdir)
# -x c-header is required because $(CXXCPP) is typically g++ -E which
# assumes the .pt files are linker input and thus ignores them.  If
# $(CXXCPP) is something else, -x c-header will cause it to fail.  In
# that case, try this definition (which uses -lang-c++ because of //
# comments and so needs GNU cpp):
#
# pt_preprocess = /lib/cpp -lang-c++ -I$(srcdir) -I$(top_srcdir)
#
# If you need it, please report, so I can add a check in
# ../configure.in or devise a trickier scheme.  (One idea would be to
# create a temporary C++ file containing just #include <foo.pt> and
# preprocess that.)

PLAYPAL.lump: $(DUMB_CONFIG_HERETIC_WAD) $(wadtool)
	$(wadtool) -r $(DUMB_CONFIG_HERETIC_WAD) -x PLAYPAL

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
