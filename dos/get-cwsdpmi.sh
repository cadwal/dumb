#!/bin/sh
# DUMB: download CWSDPMI, the DPMI host that DJGPP programs such as
# dosdumb.exe need under plain DOS and DOSBox.
#
# Usage: dos/get-cwsdpmi.sh [DESTDIR]
#
# Puts CWSDPMI.EXE into DESTDIR (default: the current directory).
# The mirrors are tried in order; set CWSDPMI_URL to try your own first.
# A csdpmi7b.zip that is already in DESTDIR is used as is.
#
# If no mirror can be reached, HDPMI32.EXE from the HX DOS extender
# (freeware, from its GitHub releases) is installed instead.  It
# works as well, but must be loaded first with "hdpmi32 -r"; the
# dosbox.conf that cross-build.sh writes does that.  Set NO_HX=1 to
# fail instead.

set -eu

dest=${1:-.}
zip=csdpmi7b.zip
hx_url=${HX_URL:-https://github.com/Baron-von-Riedesel/HX/releases/download/v2.23/HXRT223.zip}
mirrors="
${CWSDPMI_URL:-}
https://www.delorie.com/pub/djgpp/current/v2misc/$zip
http://sandmann.dotster.com/cwsdpmi/$zip
https://ftp.fu-berlin.de/pc/languages/djgpp/current/v2misc/$zip
https://ftp.gwdg.de/pub/micro/pc/simtelnet/gnu/djgpp/v2misc/$zip
"

die() {
   echo "get-cwsdpmi: $*" >&2
   exit 1
}

fetch() {
   if command -v curl >/dev/null 2>&1; then
      curl -fsSL --connect-timeout 20 -o "$2" "$1"
   elif command -v wget >/dev/null 2>&1; then
      wget -q -T 20 -O "$2" "$1"
   else
      die "need curl or wget"
   fi
}

unpack() {
   if command -v unzip >/dev/null 2>&1; then
      unzip -q -o "$1" -d "$2"
   elif command -v bsdtar >/dev/null 2>&1; then
      bsdtar -xf "$1" -C "$2"
   elif command -v python3 >/dev/null 2>&1; then
      python3 -I -m zipfile -e "$1" "$2"
   else
      die "need unzip, bsdtar or python3 to unpack $1"
   fi
}

mkdir -p "$dest"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM

if [ -f "$dest/$zip" ]; then
   cp "$dest/$zip" "$tmp/$zip"
else
   ok=
   for url in $mirrors; do
      echo "get-cwsdpmi: trying $url"
      if fetch "$url" "$tmp/$zip" 2>/dev/null && [ -s "$tmp/$zip" ]; then
	 ok=1
	 break
      fi
      rm -f "$tmp/$zip"
   done
   if [ -z "$ok" ]; then
      [ -z "${NO_HX:-}" ] ||
	 die "could not download $zip; fetch it yourself and put it in $dest"
      echo "get-cwsdpmi: no CWSDPMI mirror reachable; trying HDPMI32 from $hx_url"
      fetch "$hx_url" "$tmp/hx.zip" 2>/dev/null && [ -s "$tmp/hx.zip" ] ||
	 die "could not download $zip nor HX; fetch one yourself and put it in $dest"
      mkdir "$tmp/hx"
      unpack "$tmp/hx.zip" "$tmp/hx"
      exe=$(find "$tmp/hx" -type f -iname hdpmi32.exe | head -n 1)
      [ -n "$exe" ] || die "$hx_url has no HDPMI32.EXE"
      cp "$exe" "$dest/HDPMI32.EXE"
      echo "get-cwsdpmi: installed $dest/HDPMI32.EXE (load it with: hdpmi32 -r)"
      exit 0
   fi
fi

mkdir "$tmp/x"
unpack "$tmp/$zip" "$tmp/x"
exe=$(find "$tmp/x" -type f -iname cwsdpmi.exe | head -n 1)
[ -n "$exe" ] || die "$zip has no CWSDPMI.EXE"
cp "$exe" "$dest/CWSDPMI.EXE"
if command -v sha256sum >/dev/null 2>&1; then
   sha256sum "$tmp/$zip" | sed "s|$tmp/||"
fi
echo "get-cwsdpmi: installed $dest/CWSDPMI.EXE"
