#!/bin/sh
# DUMB: cross-compile dosdumb.exe on Linux (or macOS) with DJGPP and
# Allegro 4.2, and package it with the DUMB game, its WADs and CWSDPMI.
#
# Usage: dos/cross-build.sh [options]
#
#   -w DIR       work directory (default: build-dos in the source tree)
#   -j N         parallel make jobs (default: number of CPUs)
#   --iwad FILE  play Doom data instead of the DUMB game: copy FILE
#                into the package as DOOM.WAD (with DOOM4DUM.WAD)
#   --freedoom   like --iwad, with Freedoom's freedoom1.wad (downloaded)
#   --run        start the package in DOSBox-X (or DOSBox) afterwards
#   -h, --help   show this help
#
# Environment:
#   DJGPP_DIR    an existing DJGPP cross toolchain (the directory with
#                bin/i586-pc-msdosdjgpp-gcc); otherwise build-djgpp
#                v3.4 (GCC 12.2.0) is downloaded into the work directory
#   ALLEGRO_DIR  an existing Allegro 4.2 source tree; otherwise branch
#                4.2 is cloned into the work directory
#   DUMB_GAME_DIR  an existing checkout of the DUMB game (cadwal/dumb-game);
#                otherwise its djgpp-modernize branch is cloned
#
# Needs: sh, make, a native C compiler, autoconf, automake, git, curl
# or wget, tar with bzip2.  The prebuilt DJGPP binutils need the flex
# runtime library (libfl.so.2; package libfl2 on Debian and Ubuntu).
# Building game.wad also needs netpbm (its programs, and libnetpbm for
# ppmtodumb) and a C++ preprocessor; without them the game's
# prebuilt/game.wad is used.
#
# Steps, each skipped when its result already exists:
#   1. DJGPP toolchain        -> WORK/djgpp
#   2. Allegro 4.2 for DJGPP  -> WORK/allegro (lib/djgpp/liballeg.a)
#   3. native ptcomp/wadtool and the WADs -> WORK/native
#   4. the DUMB game          -> WORK/game/game.wad
#   5. dosdumb.exe            -> WORK/dos
#   6. package                -> WORK/dist: DOSDUMB.EXE, GAME.WAD,
#                                DOOM4DUM.WAD, HTIC4DUM.WAD, CWSDPMI.EXE
#                                (or HDPMI32.EXE if no CWSDPMI mirror
#                                can be reached),
#                                [DOOM.WAD], dosbox.conf
# See docs/dos/MODERNIZE.md for the details.

set -eu

DJGPP_RELEASE=https://github.com/andrewwutw/build-djgpp/releases/download/v3.4
ALLEGRO_GIT=https://github.com/liballeg/allegro5.git
ALLEGRO_BRANCH=4.2
DUMB_GAME_GIT=https://github.com/cadwal/dumb-game.git
DUMB_GAME_BRANCH=djgpp-modernize
FREEDOOM_ZIP=https://github.com/freedoom/freedoom/releases/download/v0.13.0/freedoom-0.13.0.zip
HOST=i586-pc-msdosdjgpp

here=$(cd "$(dirname "$0")" && pwd)
srcdir=$(cd "$here/.." && pwd)

work=$srcdir/build-dos
jobs=
iwad=
freedoom=
run=

say() {
   echo "cross-build: $*"
}

die() {
   echo "cross-build: error: $*" >&2
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
      --iwad) [ $# -ge 2 ] || usage 1; iwad=$2; shift 2 ;;
      --freedoom) freedoom=1; shift ;;
      --run) run=1; shift ;;
      -h|--help) usage 0 ;;
      *) echo "cross-build: unknown option $1" >&2; usage 1 ;;
   esac
done

if [ -z "$jobs" ]; then
   jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
fi
if [ -n "$iwad" ]; then
   [ -f "$iwad" ] || die "no such IWAD: $iwad"
   iwad=$(cd "$(dirname "$iwad")" && pwd)/$(basename "$iwad")
fi

need() {
   for c in "$@"; do
      command -v "$c" >/dev/null 2>&1 || die "$c is needed but not installed"
   done
}

fetch() {
   if command -v curl >/dev/null 2>&1; then
      curl -fL --progress-bar --connect-timeout 20 -o "$2" "$1"
   elif command -v wget >/dev/null 2>&1; then
      wget -O "$2" "$1"
   else
      die "need curl or wget"
   fi
}

need make cc autoreconf
mkdir -p "$work"
work=$(cd "$work" && pwd)
mkdir -p "$work/dl"

if [ -f "$srcdir/config.status" ]; then
   die "$srcdir has been configured in place; run 'make distclean' there first"
fi

# ---- 1. DJGPP toolchain ------------------------------------------------

if [ -n "${DJGPP_DIR:-}" ]; then
   dj=$(cd "$DJGPP_DIR" && pwd)
else
   dj=$work/djgpp
   if [ ! -x "$dj/bin/$HOST-gcc" ]; then
      case $(uname -s)-$(uname -m) in
	 Linux-x86_64) tarball=djgpp-linux64-gcc1220.tar.bz2 ;;
	 Darwin-*) tarball=djgpp-osx-gcc1220.tar.bz2 ;;
	 *) die "no prebuilt DJGPP for $(uname -s) $(uname -m); set DJGPP_DIR" ;;
      esac
      need tar bzip2
      if [ ! -f "$work/dl/$tarball" ]; then
	 say "downloading $tarball"
	 fetch "$DJGPP_RELEASE/$tarball" "$work/dl/$tarball.part"
	 mv "$work/dl/$tarball.part" "$work/dl/$tarball"
      fi
      say "unpacking the DJGPP toolchain"
      rm -rf "$dj"
      tar -xjf "$work/dl/$tarball" -C "$work"
   fi
fi
[ -x "$dj/bin/$HOST-gcc" ] || die "no $HOST-gcc in $dj/bin"
PATH=$dj/bin:$PATH
export PATH
if ! "$HOST-ar" --version >/dev/null 2>&1; then
   "$HOST-ar" --version || true
   die "$HOST-ar does not run; it may need the flex runtime (libfl.so.2)"
fi
say "using $("$HOST-gcc" --version | head -n 1)"

# ---- 2. Allegro 4.2 ----------------------------------------------------

if [ -n "${ALLEGRO_DIR:-}" ]; then
   alleg=$(cd "$ALLEGRO_DIR" && pwd)
else
   alleg=$work/allegro
   if [ ! -f "$alleg/makefile.dj" ]; then
      need git
      say "cloning Allegro $ALLEGRO_BRANCH"
      rm -rf "$alleg"
      git clone -q --depth 1 --branch "$ALLEGRO_BRANCH" "$ALLEGRO_GIT" "$alleg"
   fi
fi

if [ ! -f "$alleg/lib/djgpp/liballeg.a" ]; then
   say "building Allegro (log: $work/allegro.log)"
   # Allegro's cross build runs plain gcc, ar etc., so put DJGPP's
   # unprefixed tools first in PATH while building it.
   shim=$work/shim
   mkdir -p "$shim"
   for t in gcc g++ cpp; do
      ln -sf "$dj/bin/$HOST-$t" "$shim/$t"
   done
   (
      cd "$alleg"
      # shellcheck disable=SC2030 # PATH is meant to be local here
      PATH=$shim:$dj/$HOST/bin:$PATH
      export PATH
      m="make CROSSCOMPILE=1 DJDIR=$dj/$HOST"
      sh fix.sh djgpp --quick
      # fix.sh --quick skips "make depend", so these generated files
      # must be made first.
      $m obj/djgpp/asmcapa.h
      $m obj/djgpp/asmdef.inc
      $m -j"$jobs" lib
   ) > "$work/allegro.log" 2>&1 || die "Allegro build failed; see $work/allegro.log"
fi
[ -f "$alleg/lib/djgpp/liballeg.a" ] || die "no liballeg.a in $alleg/lib/djgpp"

# ---- 3. native tools and WADs -----------------------------------------

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
      [ -f config.status ] || "$srcdir/configure" --disable-nls --without-x
      for d in libmissing libdumbutil libdumbwad libdumb ptcomp tool doom htic; do
	 make -j"$jobs" -C "$d"
      done
   ) > "$work/native.log" 2>&1 || die "native build failed; see $work/native.log"
fi

# ---- 4. the DUMB game ---------------------------------------------------

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
	 # shellcheck disable=SC2031
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

# ---- 5. dosdumb.exe ----------------------------------------------------

dos=$work/dos
say "building dosdumb.exe (log: $work/dos.log)"
mkdir -p "$dos"
(
   cd "$dos"
   [ -f config.status ] ||
      "$srcdir/configure" --host=$HOST \
	 CPPFLAGS="-I$alleg/include" LDFLAGS="-L$alleg/lib/djgpp"
   make -j"$jobs"
) > "$work/dos.log" 2>&1 || die "DOS build failed; see $work/dos.log"
[ -f "$dos/dumb/dosdumb.exe" ] || die "dosdumb.exe was not built; see $work/dos.log"

# ---- 6. package ----------------------------------------------------------

unzip_one() {
   # unzip_one ZIP MEMBER DEST
   if command -v unzip >/dev/null 2>&1; then
      unzip -p "$1" "$2" > "$3"
   elif command -v python3 >/dev/null 2>&1; then
      python3 -I -c 'import sys, zipfile
sys.stdout.buffer.write(zipfile.ZipFile(sys.argv[1]).read(sys.argv[2]))' "$1" "$2" > "$3"
   else
      die "need unzip or python3 to unpack $1"
   fi
}

if [ -n "$freedoom" ] && [ -z "$iwad" ]; then
   fzip=$work/dl/$(basename "$FREEDOOM_ZIP")
   if [ ! -f "$fzip" ]; then
      say "downloading Freedoom"
      fetch "$FREEDOOM_ZIP" "$fzip.part"
      mv "$fzip.part" "$fzip"
   fi
   iwad=$work/dl/freedoom1.wad
   unzip_one "$fzip" freedoom-0.13.0/freedoom1.wad "$iwad.part"
   mv "$iwad.part" "$iwad"
fi

dist=$work/dist
mkdir -p "$dist"
cp "$dos/dumb/dosdumb.exe" "$dist/DOSDUMB.EXE"
cp "$gamewad" "$dist/GAME.WAD"
cp "$native/doom/doom4dum.wad" "$dist/DOOM4DUM.WAD"
cp "$native/htic/htic4dum.wad" "$dist/HTIC4DUM.WAD"
if [ -n "$iwad" ]; then
   cp "$iwad" "$dist/DOOM.WAD"
   # Without -w, DUMB loads doom.wad and doom4dum.wad.
   dumbargs=
else
   rm -f "$dist/DOOM.WAD"
   dumbargs="-w game.wad"
fi
if [ ! -f "$dist/CWSDPMI.EXE" ] && [ ! -f "$dist/HDPMI32.EXE" ]; then
   sh "$here/get-cwsdpmi.sh" "$dist" ||
      say "warning: no CWSDPMI.EXE; put one in $dist before running"
fi
# CWSDPMI is loaded by dosdumb.exe itself; HDPMI32, the fallback, has
# to be made resident first.
dpmi_load=
if [ ! -f "$dist/CWSDPMI.EXE" ] && [ -f "$dist/HDPMI32.EXE" ]; then
   dpmi_load="hdpmi32 -r"
fi

cat > "$dist/dosbox.conf" <<EOF
[dosbox]
memsize=64

[cpu]
core=dynamic
cputype=pentium
cycles=max

[autoexec]
mount c "$dist"
c:
$dpmi_load
echo DUMB writes its log to DUMB.LOG
dosdumb.exe $dumbargs > DUMB.LOG
EOF

say "done: $dist"
ls -l "$dist"
if [ -n "$dumbargs" ]; then
   say "plays the DUMB game: dosdumb.exe $dumbargs"
else
   say "plays DOOM.WAD: dosdumb.exe"
fi

# ---- run -------------------------------------------------------------

if [ -n "$run" ]; then
   if command -v dosbox-x >/dev/null 2>&1; then
      dosbox="dosbox-x"
   elif command -v dosbox >/dev/null 2>&1; then
      dosbox=dosbox
   else
      die "--run needs dosbox-x or dosbox"
   fi
   [ -f "$dist/CWSDPMI.EXE" ] || [ -f "$dist/HDPMI32.EXE" ] ||
      die "--run needs $dist/CWSDPMI.EXE (or HDPMI32.EXE)"
   say "starting $dosbox"
   exec "$dosbox" -conf "$dist/dosbox.conf"
fi
