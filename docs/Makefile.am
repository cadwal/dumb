## Process this file with automake to produce Makefile.in

EXTRA_DIST = FAQ Install-FAQ MMX.txt README.fixbm README.netplay	\
README.ptcomp discord.txt dumblogo.ppm dumb-pt.sgml keymap.txt		\
linux-libc5.txt log-classes params.txt xrosella.xpm .cvsignore

SUBDIRS = dos maint patches

info_TEXINFOS = dumb-pt.texinfo

# FIXME: Create the HTML files in a subdirectory.  But how to handle $<?
# Perhaps the subdirectory should have a Makefile of its own.
book1.html: dumb-pt.sgml
	jade -t sgml \
	     -d /usr/lib/sgml/stylesheet/dsssl/docbook/nwalsh/html/docbook.dsl \
	     $<

# TODO: Install some more documents.
#
# Files which should NOT be installed:
#
# File                  What it's about
# ----                  ---------------
# Install-FAQ           Installing DUMB
# configure.txt         Configuration before compilation
# linux-libc5.txt       Compiling for Linux libc.so.5
