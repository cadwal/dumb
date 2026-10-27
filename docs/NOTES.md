# Working notes

A running log of the modernisation work: what was done, what was
decided and why, how things were tested, and what is still open.
[MIGRATION.md](MIGRATION.md) is the tidy summary; this file keeps the
details that would otherwise be lost.  Add to the end of the log as
work goes on.

## Log

### 1. DJGPP (branch `djgpp-modernize`)

- Plan written first: [dos/MODERNIZE.md](dos/MODERNIZE.md).
- Autotools moved to autoconf 2.71 and automake 1.16; `intl/` was
  dropped for a small `DUMB_NLS` macro.
- Modern GCC fixes, the most important being the inline asm
  constraints (checked in a 32-bit harness, with mutation testing).
- Allegro 4.2 cross-built for DJGPP.  `libdumb/dosalleg.h` handles
  Allegro's `extern __inline__`, and `fixsin`/`fixcos`/`fixtan` were
  renamed `fix_*`, since liballeg exports those names.
- `DOSDUMB.EXE` links.  It has not been run: CWSDPMI could not be
  downloaded here.
- Scripts: `dos/cross-build.sh` (Linux), `dos/get-cwsdpmi.sh`, and
  `dos/dosbox-x-build.ps1`/`.cmd` (Windows; not tried on Windows).

### 2. The DUMB game (cadwal/dumb-game, branch `djgpp-modernize`)

- The `.pt` files were ported to the current ptcomp, and
  `ptcomp -o pt.wad` fixed the missing `FONTMAP` lump.
- `prebuilt/game.wad` is committed for builds without netpbm.
- The DUMB game is the default data and Freedoom is optional (the
  user's choice).

### 3. SDL3 (branch `sdl-port`)

- Plan: [PORTING.md](PORTING.md).  The user chose SDL3 over SDL2 and
  native drivers.
- `sdldumb` covers Wayland, X11 and Windows.  The timer is portable.
  `le64_fbrerend.c` uses `uint64_t` for Win64, where `long` is 32
  bits.
- Running the game exposed four bugs in the original code: the WAD
  directory seek without mmap, `lseek` getting the length as
  `whence`, an uninitialised `ProtoThing.sprites`, and `reset_misc`
  aborting.  Those fixes were ported to `djgpp-modernize` too.

### 4. One branch (`modernize`)

- `modernize` = `sdl-port` + `djgpp-modernize`, which merged without
  conflicts.  The older branches and `main` stay as they are; the
  branches are meant to live side by side, with no PRs.
- [MIGRATION.md](MIGRATION.md) was written.

### 5. Windows fixes and the kit

- Console when there is one (`AttachConsole`); otherwise
  `%APPDATA%\DUMB\dumb.log` and a message box on fatal errors.
- WADs are also looked for next to `sdldumb.exe`.
- Winsock driver: `dumb/win_net.c`.
- Bugs fixed on the way: a `va_list` reused in `logvprintf()`, and
  the netplay join going to the wrong address.
- `win/make-kit.sh` makes `dumb-VERSION-win64.zip`.

### 6. Compiler warnings

- A clean build of Linux, Windows and DOS now has no warnings at the
  default flags (`-g -O2`).
- Format strings: messages from WADs (pickup messages in `updthing.c`)
  and ptcomp's error messages were passed as the format, so a `%` in
  them would have been read as a conversion.  They are now passed as
  `"%s"` arguments, and `size_t` values are printed with `%lu` and a
  cast.
- Unchecked `fread`/`fscanf`: `wadtool -c`, `todumb`'s palette and
  `mkpnames` now report short or bad input instead of using garbage.
- Bug found next to a warning: `--dumb-play` opened the
  `--dumb-record` file (a NULL name when not recording).  Demo
  playback works again.  Tested by recording about 5 seconds and
  playing them back; the playback quit by itself at the recorded
  Escape.  Demos store input per frame, not per tick, so a playback
  is not frame-exact, as in the original.
- `wadtool -c` was checked by copying `game.wad`: all 57 lumps are
  identical.

### 7. `-Wall`

- configure now puts `-Wall` first in CFLAGS and CXXFLAGS for GCC, so
  a `-Wno-...` option given by the user still wins.  Linux (with xwad
  and the tests), Windows and DOS all build with 0 warnings.
- There were 36 warnings on Linux and 1 more for MinGW:
  - `-Wstringop-truncation` (11): `strncpy()` into 8-byte lump-name
    fields, which are meant to be unterminated when full.  The new
    `set_lumpname()` in `libdumbwad/wadstruct.h` does the same copy
    without the warning.
  - Unused variables, and an `if (!rs->name)` test on an array, which
    can never be true.
  - `test/fixed_bm.c`: the benchmark loops threw their results away,
    so the compiler could drop the work being timed.  They now store
    into `volatile` sinks.
  - xwad: `XGetGeometry()` wants `unsigned int *`, and three macros
    had a misleading `if`.
  - getopt.c: an `else` that only looked ambiguous; braces added.
- A real bug, found while fixing the misleading-`if` warnings: in
  xwad's side-ID input, the line after `if (LINEVALID)` was not
  guarded, so with no line selected it read
  `line[-1]`.
- Checked afterwards: the game under Xvfb (59 fps, walking and
  shooting), network play (3 runs out of 3), and `fixed_bm`.

### 8. `-Wextra`

- configure now adds `-Wall -Wextra` for GCC.  Builds with 0 warnings:
  Linux (default, and `--without-sdl` with OSS sound), Windows, DOS,
  and the native tools built by the DOS and Windows scripts.
- About 90 warnings across the targets:
  - Unused parameters (most of them): callbacks whose signature is
    fixed.  `ATTR_UNUSED` (`__attribute__((unused))`, from config.h
    and `dos/config.h`) marks them.  It means "may be unused", so it
    also suits parameters used only under `#ifdef`.
  - Signed/unsigned comparisons: casts where the value is known to be
    in range (`dyncode.c`, `texture.c`, `dark2trans.c`, xwad frame
    sizes, an SDL keycode that is `long` on Linux but 32 bits on
    Windows).
  - `INT_TO_FIXED()` shifted negative values left, which is
    undefined; it now multiplies (same code from GCC).  The same goes
    for `-1 << 10` in `thinghit.c`.
  - `wadtool`'s deliberate fall-through is now marked with the comment
    GCC understands.  `dos_video.c` initialises the RGB `filler`.
  - OSS sound: `write()` is checked, so a dead device logs an error
    instead of spinning.
- Bugs in xwad (the map editor):
  - `map_width` and `map_height` were unsigned, so `MAPX()` and
    `MAPY()`, which turn mouse positions into map coordinates, did
    unsigned arithmetic.  When zoomed out (a right shift), clicks in
    the left or top half of the map gave numbers around 2^31 instead
    of small negatives, and the grid there was wrong (`abs()` of an
    unsigned value).  They are `int` now.  Checked by computing the
    old and new values; xwad itself was not run.
  - The chooser skipped an item partly scrolled off the top, and did
    not clamp scrolling in lists shorter than the window.
- Checked afterwards: the game, network play (3 out of 3), demo
  playback, and the rebuilt Windows kit under Wine.

## Decisions

| Question | Decision |
|---|---|
| Backend for Linux and Windows | SDL3, version 3.4.8 |
| Default game data | The DUMB game; Freedoom optional |
| Where the game port lives | A `djgpp-modernize` branch in cadwal/dumb-game |
| Windows users without build tools | Prebuilt `game.wad` in dumb-game |
| Branches | Side by side with `main`; no PRs.  `modernize` has everything |
| Windows network driver | A separate `win_net.c` rather than `#ifdef`s in `unix_net.c` |
| Windows log without console | `dumb.log` next to `dumb.cfg`, overwritten each run, level INFO |

## Test recipes

Paths below are examples; S is any scratch directory.

**Builds** (from a clean tree, after `autoreconf -fi`):

```sh
# Linux, with a local SDL3
mkdir linux && cd linux
PKG_CONFIG_PATH=$S/sdl3/lib/pkgconfig ../configure && make -j8
# Windows
mkdir win && cd win
../configure --host=x86_64-w64-mingw32 --disable-nls \
   PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=$S/sdl3-win64/lib/pkgconfig
make -j8
# DOS
sh dos/cross-build.sh -w $S/dos
# Windows zip
sh win/make-kit.sh -w $S/kit
```

**X11 without a screen:** `Xvfb :7 -screen 0 1280x1024x24 &`, then
`DISPLAY=:7 SDL_AUDIO_DRIVER=dummy ./dumb/sdldumb -w game.wad`.  Use
`xdotool` for keys and `import -window root` for screenshots.

**Wayland without a screen:**

```sh
export XDG_RUNTIME_DIR=$S/xdg
weston --backend=headless --renderer=pixman --socket=wl-dumb --debug &
WAYLAND_DISPLAY=wl-dumb SDL_VIDEO_DRIVER=wayland ./dumb/sdldumb -w game.wad
```

Screenshots are taken with `weston-screenshooter`, which needs
`--debug`.  Pixman renders on the CPU, so the frame rate (about 39
fps) says nothing about a real desktop.

**Windows under Wine:** run `wine sdldumb.exe -w game.wad`, with
`SDL3.dll` next to the exe, and set `WINEDEBUG=-all` to quiet Wine.
Wine always passes standard handles through, so to test the "started
from Explorer" path (log file and message box), use a launcher that
calls `CreateProcessA` with `STARTF_USESTDHANDLES`, NULL handles and
`DETACHED_PROCESS`.

**Network play on one machine:**

```sh
B=./dumb/sdldumb   # or a wrapper that runs sdldumb.exe under Wine
$B -w game.wad -l master.log --dumb-master localhost:7778 &
sleep 1
$B -w game.wad -l slave.log --net-udp-port 7778 --dumb-slave localhost:7777 &
sleep 8; grep 'starting game' master.log slave.log
```

Both logs should say "starting game".  Kill the games by PID: they
don't stop on SIGTERM while waiting for the network.

**Memory bugs:** `MALLOC_PERTURB_=165 ./dumb/sdldumb ...` fills
`malloc`'d memory with garbage.  That is how the `ProtoThing.sprites`
bug became reproducible.

**WAD reading without mmap on Linux:** configure with
`ac_cv_func_mmap_fixed_mapped=no`.

## Gotchas

- Option names carry their module's prefix: `--vid-fullscreen`,
  `--input-no-mouse`, `--net-udp-port`, `--dumb-master`.  Only a few
  have short forms (`-w`, `-l`, `-d`, `-q`).
- `pkill -f sdldumb` can match the shell that runs it.  Kill by PID,
  or use `wineserver -k` for Wine.
- Under Wine on a bare Xvfb (no window manager), `xdotool` keys may
  not reach the game window.
- In shell heredocs, a line that is exactly `EOF` ends the heredoc;
  use another delimiter, or a script file.
- Allegro's cross build calls plain `gcc` and `ar`, so the DJGPP tools
  must come first in `PATH` (see `cross-build.sh`).  The prebuilt
  DJGPP `ar` needs `libfl.so.2`.
- The CWSDPMI and DJGPP mirrors may be blocked by a restrictive
  network.  The scripts say so rather than failing silently.
- `safe_free(NULL)` logs an error.  Use `free()` for memory that
  doesn't come from `safe_malloc`, such as `_fullpath()`'s result.
- Xvfb can disappear between test runs; if `xdotool` says "Can't
  open display", start it again.

## Open items

- [ ] Run `DOSDUMB.EXE` in DOSBox-X with CWSDPMI.
- [ ] Try `dos/dosbox-x-build.ps1` on Windows.
- [ ] Run the kit on real Windows 11: play, quit with Esc, and allow
      network play through the firewall prompt.
- [ ] Listen to the sound on real hardware.
- [ ] Network play between two real machines.
- [ ] CI for the Linux, MinGW and DJGPP builds.
- [ ] Linux packaging (a `.desktop` file, an icon, maybe Flatpak).
- [x] Fix the compiler warnings (0 left at `-g -O2`).
- [x] `-Wall` on by default, with 0 warnings.
- [x] `-Wextra` on by default, with 0 warnings.
- [ ] Run xwad and check the map editor by hand after the signedness
      fixes.
