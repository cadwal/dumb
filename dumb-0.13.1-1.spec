# NB: this file was automatically generated from rpmspec.m4
# If you need to change it, please edit that file (see also 
# Makefile.am, which contains the m4 invocation to build it).

Summary: Doom-compatible 3D game engine
Name: dumb
Version: 0.13.1
Release: 1
Copyright: GPL
Group: Games
Source: samba.anu.edu.au:/pub/dumb/source/dumb-0.13.1.tar.gz

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
