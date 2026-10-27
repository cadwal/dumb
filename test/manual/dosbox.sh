#!/bin/sh
# DUMB: run DOSDUMB.EXE in DOSBox (or DOSBox-X) on an X display, walk a
# little, quit with Escape, and show DUMB.LOG.
#
# Usage: test/manual/dosbox.sh DIST [SECONDS] [DOSDUMB OPTIONS]...
#
#   DIST is the dist directory made by dos/cross-build.sh, with
#   DOSDUMB.EXE, the WADs and CWSDPMI.EXE or HDPMI32.EXE.  It is
#   copied, so DIST isn't changed.  SECONDS (default 30) is how long
#   the game runs before Escape.  Options default to "-w game.wad".
#
# Environment:
#   OUT       working directory (default: ./manual-out/dosbox)
#   DISPLAY   X display (default :7)
#
# Keys are held down rather than tapped: the DOS input driver reads
# key states once a frame, so a very short tap can be missed.

set -u

dist=${1:?usage: dosbox.sh DIST [SECONDS] [DOSDUMB OPTIONS]...}
t=${2:-30}
if [ $# -ge 2 ]; then shift 2; else shift; fi
args=${*:--w game.wad}
out=${OUT:-$(pwd)/manual-out/dosbox}
DISPLAY=${DISPLAY:-:7}
export DISPLAY SDL_AUDIODRIVER=dummy

if command -v dosbox-x >/dev/null 2>&1; then
   dosbox="dosbox-x"
else
   dosbox=dosbox
fi
rm -rf "$out"
mkdir -p "$out"
cp "$dist"/*.EXE "$dist"/*.WAD "$out"/
dpmi=
if [ ! -f "$out/CWSDPMI.EXE" ] && [ -f "$out/HDPMI32.EXE" ]; then
   dpmi="hdpmi32 -r"
fi
cat > "$out/dosbox.conf" <<EOF
[sdl]
output=surface
[dosbox]
memsize=64
[cpu]
core=dynamic
cputype=pentium_slow
cycles=max
[mixer]
nosound=true
[autoexec]
mount c "$out"
c:
$dpmi
dosdumb.exe $args > DUMB.LOG
exit
EOF

"$dosbox" -conf "$out/dosbox.conf" > "$out/dosbox.out" 2>&1 &
pid=$!
sleep $((t / 2))
import -window root "$out/screen-1.png" 2>/dev/null
xdotool keydown Up; sleep 2; xdotool keyup Up
xdotool keydown Left; sleep 1; xdotool keyup Left
sleep $((t / 2))
import -window root "$out/screen-2.png" 2>/dev/null
xdotool keydown Escape; sleep 0.5; xdotool keyup Escape
sleep 5
if kill -0 "$pid" 2>/dev/null; then
   echo "$dosbox is still running; killing it"
   kill -9 "$pid"
else
   echo "$dosbox exited"
fi
echo "--- DUMB.LOG (end)"
tail -n 5 "$out/DUMB.LOG" 2>/dev/null
echo "screenshots: $out/screen-1.png $out/screen-2.png"
