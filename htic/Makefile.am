## Process this file with automake to produce Makefile.in

EXTRA_DIST = $(pt_sources) .cvsignore

CLEANFILES = htic4dum.wad

pkgdata_DATA = htic4dum.wad

# Keep htic.pt first in this list so $< works right.
pt_sources = htic.pt ambient.pt ammo.pt animtex.pt ../doom/camera.pt	\
font.pt furniture.pt generic.pt gettables.pt keys.pt levels.pt		\
../doom/linetypes.pt missiles.pt monsters.pt ../doom/sectortypes.pt	\
test.pt weapons.pt

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

htic4dum.wad: $(pt_sources) $(PTCOMP)
	$(pt_preprocess) $< | $(PTCOMP) -o $@
