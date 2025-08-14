## Process this file with automake to produce Makefile.in

EXTRA_DIST = FAQ Install-FAQ MMX.txt README.fixbm README.netplay	\
README.ptcomp discord.txt dumblogo.ppm dumb-pt.sgml dumb-pt-index.sgml	\
dumb-pt.fixed-index dumb-pt.index keymap.txt linux-libc5.txt		\
log-classes params.txt xrosella.xpm .cvsignore

SUBDIRS = dos maint patches . dumb-pt-html

info_TEXINFOS = dumb-pt.texinfo

if DUMB_CONFIG_JADE
all-local: dumb-pt-index.sgml
else
all-local:
endif

COLLATEINDEX = perl /usr/bin/collateindex.pl

# The ID is set to "genindex" because that will then become the output
# file name in the HTML version.  It would be more logical to use
# "index", but HTTP servers tend to think index.html is the title page.
$(srcdir)/dumb-pt-index.sgml: dumb-pt.fixed-index
	$(COLLATEINDEX) -i "genindex" -o $@ $<

# collateindex.pl 1.10 doesn't translate "<" to "&lt;", so we do that
# here.
$(srcdir)/dumb-pt.fixed-index: dumb-pt.index
	sed -e 's/</\&lt;/g' $< >$@

# TODO: Install some more documents.
#
# Files which should NOT be installed:
#
# File                  What it's about
# ----                  ---------------
# Install-FAQ           Installing DUMB
# configure.txt         Configuration before compilation
# linux-libc5.txt       Compiling for Linux libc.so.5
