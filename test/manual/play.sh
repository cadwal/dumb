#!/bin/sh
# DUMB: play a level with scripted keyboard input on an X display, and
# report how it ended and any sanitizer reports.
#
# Usage: test/manual/play.sh PROGRAM [DUMB OPTIONS]...
#
#   PROGRAM is sdldumb or xdumb from a build tree.  The options are
#   passed on, e.g. "-m E1M3", "-d 4", "-x 321 -y 203".
#
# Environment:
#   GAME_DIR  directory with the WADs (default: current directory)
#   WADS      WAD options (default "-w game.wad"; set it empty to load
#             doom.wad and doom4dum.wad, e.g. for Freedoom)
#   OUT       directory for the output (default: ./manual-out/play)
#   DISPLAY   X display (default :7; Xvfb is started there if needed)
#
# For a sanitizer build, configure with
#   CFLAGS="-g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined
#           -fno-sanitize=shift-base" LDFLAGS=-fsanitize=address,undefined
# (shift-base is off because the fixed-point code shifts negative
# numbers left, which GCC defines).  Needs xdotool, and Xvfb if no X
# server is running.  See docs/NOTES.md.

set -u

prog=${1:?usage: play.sh PROGRAM [DUMB OPTIONS]...}
shift
case $prog in /*) ;; *) prog=$(pwd)/$prog ;; esac
out=${OUT:-$(pwd)/manual-out/play}
DISPLAY=${DISPLAY:-:7}
export DISPLAY SDL_AUDIO_DRIVER="${SDL_AUDIO_DRIVER:-dummy}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}:log_path=$out/asan"
export UBSAN_OPTIONS=print_stacktrace=1

if ! xdpyinfo >/dev/null 2>&1 && ! xdotool getmouselocation >/dev/null 2>&1; then
   Xvfb "$DISPLAY" -screen 0 1280x1024x24 >/dev/null 2>&1 &
   sleep 1
fi

rm -rf "$out"
mkdir -p "$out"
cd "${GAME_DIR:-.}" || exit 1
# shellcheck disable=SC2086 # WADS holds several words
"$prog" ${WADS--w game.wad} --input-no-mouse "$@" > "$out/out.txt" 2>&1 &
pid=$!
sleep 5

hold() {
   xdotool keydown "$1"
   sleep "$2"
   xdotool keyup "$1"
}
# Walk, turn, shoot, open doors, change weapons, look, jump, duck.
hold Up 1.5; hold Left 0.7; hold Up 1; xdotool key space; hold Control_L 1
hold Right 1.4; hold Up 2; xdotool key Tab; hold Control_L 0.8; xdotool key 2
hold Down 1; hold Prior 0.5; hold Next 0.5; hold Shift_L 0.2; hold a 0.4; hold z 0.4
hold Up 3; xdotool key space; hold Left 2; hold Up 2; hold Control_L 1.5
import -window root "$out/screen.png" 2>/dev/null
hold Escape 0.3
sleep 3
kill -9 "$pid" 2>/dev/null

ended=$(grep -E 'Normal exit|fatal error' "$out/out.txt" | head -n 1)
echo "ended:     ${ended:-killed (no normal exit)}"
echo "frames:    $(grep 'frames,' "$out/out.txt" | tail -n 1)"
echo "UBSan:     $(grep -c 'runtime error' "$out/out.txt") reports"
grep 'runtime error' "$out/out.txt" | sort -u | head -n 5
asan=$(find "$out" -name 'asan*' | head -n 1)
if [ -n "$asan" ]; then
   echo "ASan:      report in $asan"
   grep -m 3 -E 'ERROR|SUMMARY' "$asan"
else
   echo "ASan:      no reports"
fi
echo "output:    $out"
