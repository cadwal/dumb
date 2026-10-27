#!/bin/sh
# DUMB: cross-compile sdldumb.exe for 64-bit Windows on Linux with
# MinGW-w64 and SDL3, and package it as a zip that runs on Windows 10
# and 11 with nothing else installed.
#
# Usage: win/make-kit.sh [options]
#
#   -w DIR       work directory (default: build-win in the source tree)
#   -j N         parallel make jobs (default: number of CPUs)
#   -h, --help   show this help
#
# Environment:
#   SDL3_DIR     an existing SDL3 built for x86_64-w64-mingw32 (the
#                prefix with bin/SDL3.dll and lib/pkgconfig/sdl3.pc);
#                otherwise SDL release 3.4.8 is cloned and built
#   DUMB_GAME_DIR  an existing checkout of the DUMB game (cadwal/dumb-game);
#                otherwise its modernize branch is cloned
#
# Needs: sh, make, a native C compiler, autoconf, automake,
# pkg-config, git, the MinGW-w64 toolchain (x86_64-w64-mingw32-gcc;
# package gcc-mingw-w64-x86-64 on Debian and Ubuntu), cmake and ninja
# to build SDL3, and zip or python3.  Building game.wad also needs
# netpbm and a C++ preprocessor; without them the game's
# prebuilt/game.wad is used.
#
# Steps, each skipped when its result already exists:
#   1. SDL3 for Windows       -> WORK/sdl3 (bin/SDL3.dll)
#   2. native ptcomp/wadtool and the WADs -> WORK/native
#   3. the DUMB game          -> WORK/game/game.wad
#   4. sdldumb.exe            -> WORK/win
#   5. kit                    -> WORK/dist/dumb-VERSION-win64/ and
#                                WORK/dist/dumb-VERSION-win64.zip
# See docs/PORTING.md for the details.

set -eu

SDL_GIT=https://github.com/libsdl-org/SDL.git
SDL_TAG=release-3.4.8
DUMB_GAME_GIT=https://github.com/cadwal/dumb-game.git
DUMB_GAME_BRANCH=modernize
HOST=x86_64-w64-mingw32

here=$(cd "$(dirname "$0")" && pwd)
srcdir=$(cd "$here/.." && pwd)

work=$srcdir/build-win
jobs=

say() {
   echo "make-kit: $*"
}

die() {
   echo "make-kit: error: $*" >&2
   exit 1
}

usage() {
   sed -n '2,/^$/s/^# \{0,1\}//p' "$0"
   exit "${1:-0}"
}

while [ $# -gt 0 ]; do
   case $1 in
      -w) [ $# -ge 2 ] || usage 1; work=$2; shift 2 ;;
      -j) [ $# -ge 2 ] || usage 1; jobs=$2; shift 2 ;;
      -h|--help) usage 0 ;;
      *) echo "make-kit: unknown option $1" >&2; usage 1 ;;
   esac
done

if [ -z "$jobs" ]; then
   jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
fi

need() {
   for c in "$@"; do
      command -v "$c" >/dev/null 2>&1 || die "$c is needed but not installed"
   done
}

need make cc autoreconf pkg-config "$HOST-gcc" "$HOST-strip" "$HOST-objdump"
mkdir -p "$work"
work=$(cd "$work" && pwd)

if [ -f "$srcdir/config.status" ]; then
   die "$srcdir has been configured in place; run 'make distclean' there first"
fi

# ---- 1. SDL3 -----------------------------------------------------------

if [ -n "${SDL3_DIR:-}" ]; then
   sdl=$(cd "$SDL3_DIR" && pwd)
else
   sdl=$work/sdl3
   if [ ! -f "$sdl/bin/SDL3.dll" ]; then
      need git cmake ninja
      sdlsrc=$work/sdl3-src
      if [ ! -f "$sdlsrc/CMakeLists.txt" ]; then
	 say "cloning SDL $SDL_TAG"
	 rm -rf "$sdlsrc"
	 git -c advice.detachedHead=false clone -q --depth 1 --branch "$SDL_TAG" "$SDL_GIT" "$sdlsrc"
      fi
      say "building SDL3 for Windows (log: $work/sdl3.log)"
      (
	 cmake -S "$sdlsrc" -B "$work/sdl3-build" -G Ninja \
	    -DCMAKE_SYSTEM_NAME=Windows \
	    -DCMAKE_C_COMPILER="$HOST-gcc" \
	    -DCMAKE_CXX_COMPILER="$HOST-g++" \
	    -DCMAKE_RC_COMPILER="$HOST-windres" \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_INSTALL_PREFIX="$sdl" \
	    -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF \
	    -DSDL_TEST_LIBRARY=OFF -DSDL_EXAMPLES=OFF
	 ninja -C "$work/sdl3-build" -j"$jobs" install
      ) > "$work/sdl3.log" 2>&1 || die "SDL3 build failed; see $work/sdl3.log"
   fi
fi
[ -f "$sdl/bin/SDL3.dll" ] || die "no bin/SDL3.dll in $sdl"
[ -f "$sdl/lib/pkgconfig/sdl3.pc" ] || die "no lib/pkgconfig/sdl3.pc in $sdl"

# ---- 2. native tools and WADs -----------------------------------------

if [ ! -x "$srcdir/configure" ] ||
   [ -n "$(find "$srcdir/configure.ac" -newer "$srcdir/configure")" ]; then
   say "running autoreconf"
   (cd "$srcdir" && autoreconf -fi) || die "autoreconf failed"
fi

native=$work/native
if [ ! -f "$native/doom/doom4dum.wad" ] || [ ! -f "$native/htic/htic4dum.wad" ]; then
   say "building the WADs natively (log: $work/native.log)"
   mkdir -p "$native"
   (
      cd "$native"
      [ -f config.status ] ||
	 "$srcdir/configure" --disable-nls --without-x --without-sdl
      for d in libmissing libdumbutil libdumbwad libdumb ptcomp tool doom htic; do
	 make -j"$jobs" -C "$d"
      done
   ) > "$work/native.log" 2>&1 || die "native build failed; see $work/native.log"
fi

# ---- 3. the DUMB game ---------------------------------------------------

if [ -n "${DUMB_GAME_DIR:-}" ]; then
   gamesrc=$(cd "$DUMB_GAME_DIR" && pwd)
else
   gamesrc=$work/dumb-game
   if [ ! -f "$gamesrc/pt/game.pt" ]; then
      need git
      say "cloning the DUMB game ($DUMB_GAME_BRANCH)"
      rm -rf "$gamesrc"
      git clone -q --depth 1 --branch "$DUMB_GAME_BRANCH" "$DUMB_GAME_GIT" "$gamesrc"
   fi
fi

game=$work/game
gamewad=$game/game.wad
if [ ! -f "$gamewad" ]; then
   can_build=1
   for c in pnmcat pnmdepth pnmflip ppmforge ppmchange ppmmake ppmquant ppmtomap c++; do
      command -v "$c" >/dev/null 2>&1 || can_build=
   done
   [ -x "$native/tool/ppmtodumb" ] || can_build=
   if [ -n "$can_build" ]; then
      say "building the DUMB game (log: $work/game.log)"
      mkdir -p "$game"
      (
	 PATH=$native/ptcomp:$native/tool:$PATH
	 export PATH
	 if [ ! -x "$gamesrc/configure" ]; then
	    (cd "$gamesrc" && autoreconf -fi)
	 fi
	 cd "$game"
	 [ -f config.status ] || "$gamesrc/configure"
	 make
      ) > "$work/game.log" 2>&1 || die "DUMB game build failed; see $work/game.log"
   elif [ -f "$gamesrc/prebuilt/game.wad" ]; then
      say "no netpbm or ppmtodumb; using the game's prebuilt/game.wad"
      mkdir -p "$game"
      cp "$gamesrc/prebuilt/game.wad" "$gamewad"
   else
      die "can't build game.wad (needs netpbm) and $gamesrc has no prebuilt/game.wad"
   fi
fi

# ---- 4. sdldumb.exe ----------------------------------------------------

win=$work/win
say "building sdldumb.exe (log: $work/win.log)"
mkdir -p "$win"
(
   cd "$win"
   # PKG_CONFIG_LIBDIR, not PKG_CONFIG_PATH, so that the build
   # machine's own libraries are never picked up.
   [ -f config.status ] ||
      "$srcdir/configure" --host=$HOST --disable-nls \
	 PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR="$sdl/lib/pkgconfig"
   make -j"$jobs"
) > "$work/win.log" 2>&1 || die "Windows build failed; see $work/win.log"
[ -f "$win/dumb/sdldumb.exe" ] || die "sdldumb.exe was not built; see $work/win.log"

# Everything but SDL3.dll must come with Windows itself.
extra_dlls=$("$HOST-objdump" -p "$win/dumb/sdldumb.exe" |
   sed -n 's/^[[:space:]]*DLL Name: //p' |
   grep -viE '^(SDL3|kernel32|user32|msvcrt|ws2_32|advapi32|shell32|gdi32)\.dll$' || true)
[ -z "$extra_dlls" ] || die "sdldumb.exe needs DLLs the kit doesn't have: $extra_dlls"

# ---- 5. kit -------------------------------------------------------------

version=$(sed -n 's/^PACKAGE_VERSION = //p' "$win/Makefile")
name=dumb-$version-win64
dist=$work/dist
kit=$dist/$name
rm -rf "$kit" "$dist/$name.zip"
mkdir -p "$kit"

cp "$win/dumb/sdldumb.exe" "$kit/sdldumb.exe"
"$HOST-strip" "$kit/sdldumb.exe"
cp "$sdl/bin/SDL3.dll" "$kit/"
"$HOST-strip" --strip-unneeded "$kit/SDL3.dll"
cp "$gamewad" "$kit/game.wad"
cp "$native/doom/doom4dum.wad" "$native/htic/htic4dum.wad" "$kit/"

# Windows text files: CRLF line ends and a .txt name.
crlf() {
   sed 's/$/\r/' "$1" > "$2"
}
crlf "$srcdir/COPYING" "$kit/COPYING.txt"
if [ -f "$sdl/share/licenses/SDL3/LICENSE.txt" ]; then
   crlf "$sdl/share/licenses/SDL3/LICENSE.txt" "$kit/SDL3-LICENSE.txt"
fi

cat > "$kit/DUMB.cmd.tmp" <<'EOF'
@echo off
rem Play the DUMB game.  Add DUMB options after %*, or run
rem sdldumb.exe --help in a console for the full list.
start "" "%~dp0sdldumb.exe" -w game.wad %*
EOF
crlf "$kit/DUMB.cmd.tmp" "$kit/DUMB.cmd"
rm -f "$kit/DUMB.cmd.tmp"

cat > "$kit/README.tmp" <<EOF
DUMB $version for 64-bit Windows
================================

DUMB is a Doom-like 3D game engine from 1999, here built with SDL3
for Windows 10 and 11.

Playing
-------

Double-click DUMB.cmd to play the DUMB game (game.wad).

To play Doom instead, copy doom.wad (Doom, The Ultimate Doom or
Freedoom's freedoom1.wad renamed to doom.wad) into this folder and
run sdldumb.exe without options.  doom4dum.wad, which DUMB needs
for Doom data, is already here; so is htic4dum.wad for Heretic.

Keys: the arrows move and turn, Alt with the arrows or , and .
sidestep, Shift runs, Ctrl shoots, Space opens doors, Tab or q and w
change weapons, Page Up and Page Down look up and down, F11 or
Alt+Enter toggles full screen, and Esc quits.  The mouse turns and
looks; the left button shoots and the right button opens doors.
"sdldumb.exe --help keymap" lists the options that change the keys.

Settings and logs
-----------------

Settings are read from %APPDATA%\\DUMB\\dumb.cfg.  When DUMB is not
started from a console, it writes its log to %APPDATA%\\DUMB\\dumb.log,
and a fatal error is also shown in a message box.

Started from cmd.exe or PowerShell, DUMB prints to that console
instead.  Some useful options:

  sdldumb.exe --help all          every option
  sdldumb.exe -w game.wad --vid-fullscreen
  sdldumb.exe -w game.wad -l mylog.txt

Network play
------------

One player is the master, the others are slaves; all must use the
same WADs.  UDP port 7777 must be open in the Windows firewall
(Windows asks the first time).

  master:  sdldumb.exe -w game.wad --dumb-master otherpc
  slave:   sdldumb.exe -w game.wad --dumb-slave masterpc

Licence
-------

DUMB is free software under the GNU General Public License,
version 2 or later; see COPYING.txt.  Its source code is at
https://github.com/cadwal/dumb (branch modernize).  SDL3 is under
the zlib licence; see SDL3-LICENSE.txt.
EOF
crlf "$kit/README.tmp" "$kit/README.txt"
rm -f "$kit/README.tmp"

say "zipping $name.zip"
if command -v zip >/dev/null 2>&1; then
   (cd "$dist" && zip -qr9 "$name.zip" "$name")
elif command -v python3 >/dev/null 2>&1; then
   (cd "$dist" && python3 -I -c 'import os, sys, zipfile
with zipfile.ZipFile(sys.argv[1], "w", zipfile.ZIP_DEFLATED) as z:
    for root, dirs, files in os.walk(sys.argv[2]):
        for f in sorted(files):
            z.write(os.path.join(root, f))' "$name.zip" "$name")
else
   die "need zip or python3 to make the zip"
fi

say "done: $dist/$name.zip"
ls -l "$kit" "$dist/$name.zip"
