#!/bin/sh
# XWad: open a map, enlarge the window (there may be no window manager
# to do it), run mouse steps, and take screenshots before and after.
#
# Usage: test/manual/xwad.sh PROGRAM MAP [STEP]...
#
#   PROGRAM is xwad/XWad from a build tree.  Each STEP is "X Y BUTTON"
#   (a click at screen position X,Y) or "sleep N".  The window is put
#   at 0,0 and made 1000x800, so for the layout of this version:
#     "792 781 1"  Zoom In        "848 781 1"  Zoom Out
#     "152 21 1"   VerMode        "300 400 2"  create an entity there
#
# Environment:
#   GAME_DIR  directory with the WADs (default: current directory)
#   WAD       WAD to load (default doom.wad)
#   OUT       directory for the output (default: ./manual-out/xwad)
#   DISPLAY   X display (default :7)
#
# Example: zoom in to 1:2, then create and select a vertex:
#   xwad.sh xwad/XWad E1M1 "792 781 1" "792 781 1" "792 781 1" \
#      "152 21 1" "300 400 2" "300 400 1"

set -u

prog=${1:?usage: xwad.sh PROGRAM MAP [STEP]...}
map=${2:?usage: xwad.sh PROGRAM MAP [STEP]...}
shift 2
case $prog in /*) ;; *) prog=$(pwd)/$prog ;; esac
out=${OUT:-$(pwd)/manual-out/xwad}
DISPLAY=${DISPLAY:-:7}
export DISPLAY
export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1

rm -rf "$out"
mkdir -p "$out"
cd "${GAME_DIR:-.}" || exit 1
"$prog" -v -w "${WAD:-doom.wad}" -m "$map" > "$out/out.txt" 2>&1 &
pid=$!
sleep 8
win=$(xdotool search --name "$map" 2>/dev/null | head -n 1)
if [ -z "$win" ]; then
   echo "no XWad window; output:"
   tail -n 5 "$out/out.txt"
   kill -9 "$pid" 2>/dev/null
   exit 1
fi
xdotool windowmove "$win" 0 0 windowsize "$win" 1000 800
sleep 2
import -window root "$out/before.png"
for step in "$@"; do
   case $step in
      sleep*) sleep "${step#sleep }" ;;
      *)
	 # shellcheck disable=SC2086 # split "X Y BUTTON"
	 set -- $step
	 xdotool mousemove "$1" "$2" click "$3"
	 sleep 1
	 ;;
   esac
done
sleep 1
import -window root "$out/after.png"
if kill -0 "$pid" 2>/dev/null; then echo "XWad still running"; else echo "XWad exited"; fi
kill "$pid" 2>/dev/null
sleep 1
kill -9 "$pid" 2>/dev/null
echo "UBSan: $(grep -c 'runtime error' "$out/out.txt") reports"
grep -m 3 'runtime error\|AddressSanitizer' "$out/out.txt"
echo "screenshots: $out/before.png $out/after.png"
