## Process this file with automake to produce Makefile.in

EXTRA_DIST = $(pt_sources) .cvsignore

CLEANFILES = $(pt_wad) $(other_lumps) PLAYPAL.lump htic4dum.wad

if DUMB_CONFIG_HERETIC
pkgdata_DATA = htic4dum.wad
else
pkgdata_DATA = 
endif

pt_wad = pt.wad
other_lumps = COLORM16.lump COLORM32.lump DUMBLOGO.lump
playpal_src_wad = $(DUMB_CONFIG_HERETIC_WAD)

# Keep htic.pt first in this list so $< works right.
pt_sources = htic.pt ambient.pt animtex.pt ../doom/camera.pt		\
furniture.pt generic.pt gettables.pt keys.pt levels.pt			\
../doom/linetypes.pt missiles.pt monsters.pt ../doom/sectortypes.pt	\
weapons.pt

# The rest is identical with ../doom/, except for the last rule.

# Always use the latest versions.  Use full filenames so targets can
# depend on them and be rebuilt if the executables change.
WADTOOL = ../tool/wadtool
PPMTODUMB = ../tool/ppmtodumb
PTCOMP = ../ptcomp/ptcomp

pt_preprocess = $(CPP) -x c-header -I$(srcdir) -I$(top_srcdir)
# -x c-header is required because $(CPP) is typically gcc -E which
# assumes the .pt files are linker input and thus ignores them.  If
# $(CPP) is something else, -x c-header will cause it to fail.  In
# that case, try this definition:
#
# pt_preprocess = /lib/cpp -I$(srcdir) -I$(top_srcdir)
#
# If you need it, please report, so I can add a check in
# ../configure.in or devise a trickier scheme.  (One idea would be to
# create a temporary C file containing just #include <foo.pt> and
# preprocess that.)

PLAYPAL.lump: $(playpal_src_wad) $(WADTOOL)
	$(WADTOOL) -r $(playpal_src_wad) -x PLAYPAL

# ../docs/dumblogo.ppm is searched in VPATH and $< refers to it.
DUMBLOGO.lump: ../docs/dumblogo.ppm PLAYPAL.lump $(PPMTODUMB)
	$(PPMTODUMB) -p -M PLAYPAL.lump <$< >$@

$(pt_wad): $(pt_sources) $(PTCOMP)
	$(pt_preprocess) $< | $(PTCOMP) -o $@

COLORM16.lump: PLAYPAL.lump $(PPMTODUMB)
	$(PPMTODUMB) -c -2 -M $< >$@

COLORM32.lump: PLAYPAL.lump $(PPMTODUMB)
	$(PPMTODUMB) -c -4 -M $< >$@

htic4dum.wad: $(WADTOOL) $(pt_wad) $(other_lumps)
	$(WADTOOL) -w $@ \
		-c $(pt_wad) \
		-f $(other_lumps)
