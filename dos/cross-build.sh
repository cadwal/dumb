#!/bin/sh
# DUMB: cross-compile dosdumb.exe on Linux (or macOS) with DJGPP and
# Allegro 4.2, and package it with its WADs and CWSDPMI.
#
# Usage: dos/cross-build.sh [options]
#
#   -w DIR       work directory (default: build-dos in the source tree)
#   -j N         parallel make jobs (default: number of CPUs)
#   --iwad FILE  also copy FILE into the package as DOOM.WAD
#   --run        start the package in DOSBox-X (or DOSBox) afterwards
#   -h, --help   show this help
#
# Environment:
#   DJGPP_DIR    an existing DJGPP cross toolchain (the directory with
#                bin/i586-pc-msdosdjgpp-gcc); otherwise build-djgpp
#                v3.4 (GCC 12.2.0) is downloaded into the work directory
#   ALLEGRO_DIR  an existing Allegro 4.2 source tree; otherwise branch
#                4.2 is cloned into the work directory
#
# Needs: sh, make, a native C compiler, autoconf, automake, git, curl
# or wget, tar with bzip2.  The prebuilt DJGPP binutils need the flex
# runtime library (libfl.so.2; package libfl2 on Debian and Ubuntu).
#
# Steps, each skipped when its result already exists:
#   1. DJGPP toolchain        -> WORK/djgpp
#   2. Allegro 4.2 for DJGPP  -> WORK/allegro (lib/djgpp/liballeg.a)
#   3. native ptcomp/wadtool and the WADs -> WORK/native
#   4. dosdumb.exe            -> WORK/dos
#   5. package                -> WORK/dist: DOSDUMB.EXE, DOOM4DUM.WAD,
#                                HTIC4DUM.WAD, CWSDPMI.EXE, [DOOM.WAD]
# See docs/dos/MODERNIZE.md for the details.

set -eu

DJGPP_RELEASE=https://github.com/andrewwutw/build-djgpp/releases/download/v3.4
ALLEGRO_GIT=https://github.com/liballeg/allegro5.git
ALLEGRO_BRANCH=4.2
HOST=i586-pc-msdosdjgpp

here=$(cd "$(dirname "$0")" && pwd)
srcdir=$(cd "$here/.." && pwd)

work=$srcdir/build-dos
jobs=
iwad=
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

# ---- 4. dosdumb.exe ----------------------------------------------------

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

# ---- 5. package ----------------------------------------------------------

dist=$work/dist
mkdir -p "$dist"
cp "$dos/dumb/dosdumb.exe" "$dist/DOSDUMB.EXE"
cp "$native/doom/doom4dum.wad" "$dist/DOOM4DUM.WAD"
cp "$native/htic/htic4dum.wad" "$dist/HTIC4DUM.WAD"
if [ -n "$iwad" ]; then
   cp "$iwad" "$dist/DOOM.WAD"
fi
if [ ! -f "$dist/CWSDPMI.EXE" ]; then
   sh "$here/get-cwsdpmi.sh" "$dist" ||
      say "warning: no CWSDPMI.EXE; put one in $dist before running"
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
echo DUMB writes its log to DUMB.LOG
dosdumb.exe > DUMB.LOG
EOF

say "done: $dist"
ls -l "$dist"
[ -f "$dist/DOOM.WAD" ] ||
   say "note: put an IWAD in $dist as DOOM.WAD (e.g. Freedoom's freedoom1.wad)"

# ---- run -------------------------------------------------------------

if [ -n "$run" ]; then
   if command -v dosbox-x >/dev/null 2>&1; then
      dosbox="dosbox-x"
   elif command -v dosbox >/dev/null 2>&1; then
      dosbox=dosbox
   else
      die "--run needs dosbox-x or dosbox"
   fi
   [ -f "$dist/CWSDPMI.EXE" ] || die "--run needs $dist/CWSDPMI.EXE"
   [ -f "$dist/DOOM.WAD" ] || die "--run needs an IWAD; use --iwad"
   say "starting $dosbox"
   exec "$dosbox" -conf "$dist/dosbox.conf"
fi
