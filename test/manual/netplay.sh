#!/bin/sh
# DUMB: network play on one machine, a master and N slaves, each on
# its own UDP port.  Reports which players got as far as the game.
#
# Usage: test/manual/netplay.sh PROGRAM NSLAVES [DUMB OPTIONS]...
#
#   PROGRAM is sdldumb, or a wrapper that runs sdldumb.exe under Wine
#   (exec wine /path/sdldumb.exe "$@").  NSLAVES is 1 to 3.
#
# Environment:
#   GAME_DIR  directory with the WADs (default: current directory)
#   SECONDS_  how long to let the games run (default 15)
#   OUT       directory for the logs (default: ./manual-out/netplay)
#   DISPLAY   X display (default :7)
#
# Example, with Freedoom's freedoom2.wad as doom2.wad and doom4dum.wad
# in GAME_DIR:  netplay.sh dumb/sdldumb 3 -m MAP01
# The map needs a start for every player.  The games are killed at
# the end: they don't stop on SIGTERM while waiting for the network.

set -u

prog=${1:?usage: netplay.sh PROGRAM NSLAVES [DUMB OPTIONS]...}
n=${2:?usage: netplay.sh PROGRAM NSLAVES [DUMB OPTIONS]...}
shift 2
case $prog in /*) ;; *) prog=$(pwd)/$prog ;; esac
out=${OUT:-$(pwd)/manual-out/netplay}
DISPLAY=${DISPLAY:-:7}
export DISPLAY SDL_AUDIO_DRIVER="${SDL_AUDIO_DRIVER:-dummy}"
export UBSAN_OPTIONS=print_stacktrace=1

rm -rf "$out"
mkdir -p "$out"
cd "${GAME_DIR:-.}" || exit 1

masters=
i=1
while [ "$i" -le "$n" ]; do
   masters="$masters --dumb-master localhost:$((7777 + i))"
   i=$((i + 1))
done
# shellcheck disable=SC2086 # masters holds several words
ASAN_OPTIONS=detect_leaks=0:log_path=$out/asan-master \
   "$prog" "$@" --input-no-mouse -l "$out/master.log" $masters \
   > "$out/master.out" 2>&1 &
pids=$!
sleep 1
i=1
while [ "$i" -le "$n" ]; do
   ASAN_OPTIONS=detect_leaks=0:log_path=$out/asan-slave$i \
      "$prog" "$@" --input-no-mouse -l "$out/slave$i.log" \
      --net-udp-port $((7777 + i)) --dumb-slave localhost:7777 \
      > "$out/slave$i.out" 2>&1 &
   pids="$pids $!"
   i=$((i + 1))
done
sleep "${SECONDS_:-15}"
# shellcheck disable=SC2086 # pids holds several words
kill -9 $pids 2>/dev/null

for who in master $(seq 1 "$n" | sed 's/^/slave/'); do
   log=$out/$who.log
   printf '%-8s started: %s  fatal: %s  UBSan: %s\n' "$who" \
      "$(grep -c 'starting game' "$log" 2>/dev/null)" \
      "$(grep -c 'fatal' "$log" 2>/dev/null)" \
      "$(grep -c 'runtime error' "$out/$who.out")"
   grep -h -m 1 'has no start\|fatal' "$log" 2>/dev/null
done
if find "$out" -name 'asan*' | grep -q .; then
   echo "ASan reports: $(find "$out" -name 'asan*' | tr '\n' ' ')"
else
   echo "no ASan reports"
fi
