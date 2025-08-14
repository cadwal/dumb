divert(-1)

# This file is used to generate a .spec file for rpm packaging.
# The basic reason for the m4 preprocessing is so that we can
# steal the version number from automake.

# The most notable thing about this file is that it assumes 
# you want a stripped, optimized version of DUMB: it uses 
# -fomit-frame-pointer, which basically excludes debugging on
# many platforms (incl. ix86), and installs with install-strip

# I'm presuming that people who want to really hack on DUMB
# will work from the original sources, rather than an SRPM.

# Comments following this one are for the .spec, ignore them.

divert(0)dnl
# NB: this file was automatically generated from aux/rpmspec.m4
# If you need to change it, please edit that file (see also 
# Makefile.am, which contains the m4 invocation to build it).

Summary: Doom-compatible 3D game engine
Name: RPMPACKAGE
Version: RPMVERSION
Release: RPMRELEASE
Copyright: GPL
Group: Games
Source: samba.anu.edu.au:/pub/dumb/source/RPMPACKAGE-RPMVERSION.tar.gz

%description
This program allows you to build and play 3D games rather after the
style of Id Software's "Doom".  You can also play Doom, Doom II, or
Heretic with it, but you will need the ".WAD" files that come with
those games.

%prep
%setup

%build
export CFLAGS="-pipe -O2 -fomit-frame-pointer $RPM_OPT_FLAGS"
export CXXFLAGS="-pipe -O2 -fomit-frame-pointer $RPM_OPT_FLAGS"
./configure --prefix=/usr
make

%install
make install-strip

%files
/usr/bin/xdumb
%ifarch i386
/usr/bin/ldumb
%endif
/usr/bin/XWad
/usr/bin/XProtoThing
/usr/bin/ppmtodumb
/usr/bin/ptcomp
/usr/bin/mkpnames
/usr/share/dumb/
