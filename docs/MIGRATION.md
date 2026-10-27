# Modernising DUMB 0.13.11: migration notes and findings

DUMB 0.13.11 dates from 1999.  This document covers the work that
makes it build with current toolchains and run on current systems,
along with what turned up along the way.  Two documents go into more
detail:

- [docs/dos/MODERNIZE.md](dos/MODERNIZE.md) covers DOS: DJGPP 2.05,
  Allegro 4.2 and the build scripts.
- [docs/PORTING.md](PORTING.md) covers the SDL3 port to Linux
  (Wayland and X11) and Windows.
- [docs/NOTES.md](NOTES.md) is the running work log: decisions, test
  recipes, gotchas and open items.

## Branches

All of these branches live side by side.  Nothing is merged into
`main`.

| Branch | Repository | Contents |
|---|---|---|
| `main` | cadwal/dumb | The original sources, untouched |
| `djgpp-modernize` | cadwal/dumb | Autotools, GCC fixes, dosdumb with DJGPP and Allegro, scripts |
| `sdl-port` | cadwal/dumb | `djgpp-modernize` plus the SDL3 driver, the portable timer and Windows |
| `modernize` | cadwal/dumb | Everything: `sdl-port` merged with the latest `djgpp-modernize` |
| `djgpp-modernize` | cadwal/dumb-game | The game data ported to the current ptcomp, plus a prebuilt `game.wad` |

`modernize` is the branch to use.  The other two branches are kept so
that each step can be read on its own.

## What one branch builds

| Target | Program | Toolchain | Status |
|---|---|---|---|
| Linux, Wayland or X11 | `sdldumb` | GCC with SDL 3.4 | Builds; runs on Xvfb and headless Weston |
| Linux, X11 only | `xdumb` | GCC with Xlib | Builds |
| Windows 10/11 x64 | `sdldumb.exe` | MinGW-w64 with SDL 3.4 | Builds, and runs under Wine; `win/make-kit.sh` makes a zip |
| DOS | `DOSDUMB.EXE` | DJGPP 2.05 (GCC 12.2) with Allegro 4.2 | Builds and links; not yet run (see "Remaining work") |
| Any | `dummydumb` | GCC | Builds; headless test driver |

## Build system

- `configure.in` became `configure.ac`.  It needs autoconf 2.71 and
  automake 1.16.
- `acinclude.m4` was rewritten.  Obsolete macros were replaced, and a
  small `DUMB_NLS` macro replaces the bundled 1990s `intl/` directory,
  which has been removed together with `acconfig.h` and the generated
  `aclocal.m4`.
- `INCLUDES` became `AM_CPPFLAGS`.  The checks for `sys/wait.h` and
  `RETSIGTYPE` were dropped.
- netpbm headers are now found in `<netpbm/ppm.h>`, where current
  distributions install them.
- The UTF-8 message catalog is built with `iconv` rather than
  `recode`.  `po/fi.po-utf8` contained a stray Latin-1 byte, which has
  been fixed.
- New options and conditionals:
  - `PKG_CHECK_MODULES([SDL], [sdl3])` with `--without-sdl` and
    `--with-sdl-sound`.
  - `DUMB_HAVE_SETITIMER`.
  - `AC_CHECK_SIZEOF([void *])`, which picks the 32- or 64-bit frame
    buffer rerenderer.
  - The WADs are not built when cross-compiling, because the tools
    that make them must run on the build machine.
- `libmissing/` gained `towlower` and related functions, which DJGPP
  lacks.  `hstrerror` is now replaced only where the Unix network
  driver needs it.

## Fixes for modern compilers

GCC 12 and later reject or miscompile a good deal of 1999 code.  The
fixes are:

- **Inline asm.**  Several `asm` statements had wrong input and output
  constraints, or registers that were modified without being declared.
  Old GCC happened to produce working code; new GCC uses the registers
  it was promised were untouched.  The constraints were corrected and
  memory clobbers added.  `SLICE_STRING` became a macro so that its
  shift count can be an `"i"` (immediate) operand.  The fixed-point
  routines were checked in a freestanding 32-bit test harness against
  C reference versions, and the harness was itself checked by mutation
  testing: deliberately broken asm had to fail it.
- **`ATTR_REGPARM`.**  The attribute is placed consistently on
  declarations and definitions in `fixed.c`.
- **C++ (xwad).**  `wadchain.hh` and the `CTLATTR` macro were fixed
  for current g++.
- **libpng.**  `png->jmpbuf` became `png_jmpbuf(png)`.
- **Missing `#include`s and implicit declarations**, which are now
  errors.
- **Warnings.**  The tree now builds with no warnings at `-Wall
  -Wextra`, which configure turns on for GCC.  See [NOTES.md](NOTES.md) for
  what was fixed.
- **ptcomp.**  `Chance` values were parsed with `atol` into a type
  that is 64 bits wide on 64-bit Linux, so negative values broke.  They
  are now signed and parsed with `strtol`.

## DOS: DJGPP and Allegro 4.2

- The toolchain is build-djgpp v3.4: DJGPP 2.05, GCC 12.2 and
  binutils.  Allegro 4.2 is cross-compiled with `CROSSCOMPILE=1`, with
  symlinks that put the DJGPP tools first on `PATH`.  `asmcapa.h` and
  `asmdef.inc` must be generated first.  The prebuilt `ar` needs
  `libfl.so.2`.
- Allegro 4.2 declares its inline functions `extern __inline__`.
  Under C99 and GCC 5 or later this means the opposite of what GNU89
  meant, so every file that includes Allegro defines those functions
  again, and the link fails.  `libdumb/dosalleg.h` wraps
  `<allegro.h>`: it makes `AL_INLINE` static, and it renames Allegro's
  `fixed`, `fixmul` and `fixdiv`, which clash with DUMB's own.
- liballeg also exports `fixsin`, `fixcos` and `fixtan`, so DUMB's
  functions of those names were renamed `fix_sin`, `fix_cos` and
  `fix_tan`.
- The DOS drivers (`dos_video.c`, `dos_input.c` and `dos_sound.c`)
  were updated for the Allegro 4.2 API.  The sound `SAMPLE` is now
  allocated with `calloc` and initialised fully.
- `dos/makefile.dj` is a flat makefile for DJGPP itself, with `-MMD`
  dependencies.  It builds both natively in DOSBox-X and when
  cross-compiling.

### Scripts in `dos/`

| Script | Runs on | Does |
|---|---|---|
| `cross-build.sh` | Linux, POSIX sh | Toolchain, Allegro, WADs, the DUMB game and `DOSDUMB.EXE`, then a `dist/` directory with `dosbox.conf`.  `--run` starts DOSBox-X; `--freedoom` and `--iwad` choose other game data |
| `get-cwsdpmi.sh` | Linux | Downloads the CWSDPMI DPMI host |
| `dosbox-x-build.ps1` / `.cmd` | Windows | Sets up DJGPP and Allegro inside DOSBox-X, compiles there and runs dosdumb |

The default game is the DUMB game from cadwal/dumb-game.  Freedoom is
optional.

## The DUMB game (cadwal/dumb-game)

- The `.pt` files were ported to the current ptcomp syntax: names are
  quoted, and `FireballThrower` is declared gettable.
- `pt/Makefile.am` uses `ptcomp -o pt.wad`.  The old output lacked the
  `FONTMAP` lump, and DUMB crashed without it.
- `configure.ac` was modernised in the same way as the engine's.
- `prebuilt/game.wad` is committed so that Windows users don't need
  netpbm and the build tools.

## SDL3: Linux (Wayland and X11) and Windows

SDL3 was chosen because one driver then covers Wayland, X11 and
Windows, and SDL picks the right backend at run time.  The new driver
is `sdldumb`; the old X11, SVGAlib and DOS drivers remain.

- **Video** (`dumb/sdl_video.c`):
  - The game draws into a streaming texture, shown with logical
    presentation and nearest-neighbour scaling.  Sizes in a 16:10
    ratio are shown at 4:3, as the original was meant to look.
  - 8-bit frames are converted through the palette to XRGB8888.  16-
    and 32-bit depths are supported directly.
  - Options: `--vid-fullscreen`, `--vid-scale`, `--vid-no-aspect`,
    `--vid-integer-scale` and `--vid-no-vsync`.  F11 or Alt+Enter
    toggles fullscreen.
  - SDL is initialised only when the video starts, so `--help` works
    without a display.
- **Input** (`dumb/sdl_input.c`):
  - SDL keycodes are mapped to the X11 driver's key names, so the
    existing keymaps work unchanged.
  - The mouse uses relative mode.  The left button shoots, the right
    button activates and the middle button walks forward.
  - Keys that are held down are released when the window loses focus.
  - A key pressed and released within one frame has its release held
    back by one frame, because the game reads key states only once a
    frame and would otherwise miss quick taps.
- **Sound** (`libdumb/sdl_sound.c`):
  - DUMB's own mixer (`sound_mix.c`) feeds an SDL audio stream with
    signed 16-bit samples.
  - Options: `--sound-mono` and `--sound-latency` (60 ms by default).
- **Timer** (`libdumbutil/timer.c`):
  - The timer is now portable.  It uses `clock_gettime` or
    `QueryPerformanceCounter` instead of `setitimer` and `SIGALRM`.
  - A reset keeps the part of a tick that has already elapsed.  Long
    stalls are capped at 99 ticks so that the game doesn't spin.
- **Windows** (MinGW-w64, x86-64):
  - Windows is LLP64, so `long` is 32 bits there.  The 64-bit frame
    buffer rerenderer used `long`; it now uses `uint64_t`, and the
    rerenderer is chosen by `sizeof(void *)`.
  - The config file is `%APPDATA%\DUMB\dumb.cfg`.  Paths may contain
    `\` and drive letters, and WADs are also found next to
    `sdldumb.exe`.
  - Only `SDL3.dll` needs to be shipped.
  - The program is a GUI application, so it has no console of its own.
    Started from cmd.exe or PowerShell, it attaches to that console.
    Otherwise it logs to `%APPDATA%\DUMB\dumb.log`, and shows fatal
    errors in a message box.
  - Network play uses Winsock 2 (`dumb/win_net.c`).  Windows and Linux
    players can play together.
  - `win/make-kit.sh` builds `dumb-VERSION-win64.zip`, which runs with
    nothing else installed.

## Bugs found in the original code

Some bugs were latent: they never showed on Linux with glibc and
`mmap`, but appeared on other platforms or with other allocators.

1. **WAD directory read from the wrong place without `mmap`**
   (`libdumbwad/wadio.c`).  The code that reads files without `mmap`
   never seeked to the directory offset, so it read the header area as
   the directory ("lump 0 is too big for wad").  This showed up on
   Windows.  The fix adds a `safe_lseek` to `diroffset`.
   `safe_munmap` is now also guarded by `HAVE_MMAP`.
2. **`lseek` given the length as `whence`** (`libdumbwad/loadlump.c`).
   `load_lump()` passed the lump's length where `SEEK_SET` belonged.
   This showed as "lseek failed: Invalid argument" right after fixing
   bug 1.  Both fixes were also verified on Linux by forcing the code
   without `mmap` (`ac_cv_func_mmap_fixed_mapped=no`).
3. **Uninitialised `ProtoThing.sprites`** (`libdumb/protoinwad.c`).
   The array was allocated with `malloc`, but `get_sprite_table()`
   expected the field to start out `NULL`.  Random crashes appeared
   when running under `stdbuf`; `MALLOC_PERTURB_` made them
   reproducible.  The array is now allocated with `calloc`.
4. **`reset_misc` with nothing to reset** (`libdumb/texture.c`).  It
   aborted when called before the misc textures were loaded.  It now
   returns early, and the alignment check uses `uintptr_t` instead of
   casting a pointer to `int`.
5. **ptcomp's 64-bit `Chance`**, described in "Fixes for modern
   compilers".
6. **Inline asm constraints**, described in "Fixes for modern
   compilers".  This code was wrong all along, but only miscompiles
   with a modern register allocator.
7. **Allegro's `extern __inline__`**, described under DOS.  The
   meaning of `extern inline` changed with C99.
8. **One `va_list` used twice** (`libdumbutil/log.c`).
   `logvprintf()` passed the same `va_list` to `vfprintf()` for every
   log file.  That happens to work on i386, but on x86-64 the second
   file (for example `-l FILE` next to the console) got garbage.  It
   now uses `va_copy()`.
9. **Network play joined the wrong address** (`dumb/netplay.c`).  When
   the master got a slave's join request, it looked the slave up again
   by the host name in the request, with the default port.  With a
   port given in `--dumb-master`, or with two players on one machine,
   the master then sent to itself forever, unless its first packet
   happened to reach the slave before the request came.  On one
   machine it worked in 1 run out of 5.  The master now keeps the
   address the request came from; 5 runs out of 5 then worked.
10. **Demo playback opened the wrong file** (`dumb/dumb.c`).
    `--dumb-play` opened the file named by `--dumb-record`, which is
    NULL when not recording.  Found while fixing a compiler warning
    next to it.
11. **Messages used as format strings** (`dumb/updthing.c`,
    `ptcomp/parm.c`).  Pickup messages come from the WAD, so a `%` in
    one would have been read as a conversion.
12. **xwad read `line[-1]`** (`xwad/xwadctls.c`).  A macro's second
    statement after `if (LINEVALID)` was not guarded, so with no line
    selected it indexed the line array with -1.  Found while fixing
    `-Wmisleading-indentation` warnings in the macros next to it.
13. **xwad mapped mouse clicks with unsigned arithmetic**
    (`xwad/xwad.h`, `xwad/xwadmap.h`).  `map_width` and `map_height`
    were unsigned, so `MAPX()` and `MAPY()` worked in unsigned
    arithmetic.  When zoomed in, the result was off by exactly 2^31,
    which the 16-bit map coordinates (and the grid's modulo) happen to
    discard, so it never showed: an XWad with the old code places a
    vertex in the same spot.  Not a visible bug, but the code now says
    what it means.  Found by `-Wextra`'s `-Wsign-compare`.

These were found by running the game and XWad under AddressSanitizer
and UndefinedBehaviorSanitizer:

14. **Renderer read and wrote outside its buffers at odd view sizes**
    (`dumb/renderc.h`, `dumb/rendcore.h`).  With an odd width, the
    left edge of the view became column -1; with an odd height,
    sprites were clipped to row -1.
15. **Network play on a map with too few player starts** used thing
    -1 as a player (`dumb/levdata.c`).  The DUMB game's map has only
    a player 1 start, so every two-player game with it ran on
    garbage.  DUMB now stops with a clear message.
16. **More than one slave on one machine never got going**
    (`dumb/unix_net.c`, `dumb/win_net.c`).  The master told stations
    apart by IP address only.  It now compares the port too.
17. **Signed overflow on big maps** (`dumb/renderc.h`,
    `dumb/levdyn.c`): wall occlusion could get the wrong sign, and a
    sector's centre was a sum that overflowed.  Both use 64-bit
    arithmetic now.  Texture stepping in `dumb/rendflat.h` wrapped by
    design and is now unsigned.
18. **Doom textures read misaligned** (`libdumbwad/doomstruct.h`).
    Harmless on x86 but fatal on strict-alignment CPUs; the struct is
    now packed.
19. **Renderer limits** (`dumb/renderc.h`): 64 wall starts per column
    was too few for Freedoom's E3M3, and the active-sprite list had a
    fixed 128 entries with no check in normal builds.
20. **XWad never started.**  It aborted in `setwmstuff()` because the
    texture browser had no title for "no type chosen yet"
    (`xwad/tchoose.c`).  It also crashed when the old Adobe X fonts
    are missing, as they are on most current systems; it now falls
    back to the `fixed` font (`xwad/controls.c`).

## How things were tested

| What | How |
|---|---|
| X11 | `sdldumb` and `xdumb` on Xvfb, with `xdotool` for keys and the mouse, and screenshots compared with ImageMagick `compare`.  About 58–60 fps with vsync |
| Wayland | Weston with `--backend=headless --renderer=pixman`, with its own socket and `XDG_RUNTIME_DIR`.  Screenshots were taken with `weston-screenshooter`, which needs `--debug`.  About 39 fps; see below |
| Windows | `sdldumb.exe` under Wine, with SDL's `windows` video driver and the `direct3d` renderer.  A small launcher started it without standard handles, as Explorer does, to test the log file and the message box |
| Network play | Master and slave on one machine with different UDP ports: Linux with Linux, Windows with Windows, and a Linux master with a Windows slave |
| Memory errors | `MALLOC_PERTURB_` to fill new memory with garbage, which exposed bug 3 |
| Inline asm | A freestanding 32-bit harness compared asm with C reference versions, and was checked with mutation testing |
| DOS | Built and linked with the cross toolchain.  Not run yet |

### A note on Weston's renderer

The Wayland test ran Weston with the headless backend and the
**pixman** renderer, which composites on the CPU.  That is why it
reached about 39 fps against 60 on Xvfb: the compositor itself was the
bottleneck, not DUMB.  A desktop session (Weston with its GL renderer,
GNOME's Mutter or KDE's KWin) composites on the GPU, so this number
says nothing about real performance.  Its value is that it proves
SDL's native Wayland backend works with DUMB: window creation, the
`xdg-shell` surface, presentation and input, without XWayland.

To try it:

```sh
export XDG_RUNTIME_DIR=$(mktemp -d)
weston --backend=headless --renderer=pixman --socket=wl-dumb &
WAYLAND_DISPLAY=wl-dumb SDL_VIDEO_DRIVER=wayland ./dumb/sdldumb
```

## Remaining work

- **DOS.**  `DOSDUMB.EXE` has run in DOSBox (0.74) with HDPMI32 as
  the DPMI host, because the CWSDPMI mirrors were blocked here.  It
  found the Sound Blaster, set the VGA mode, and played the DUMB game
  with keyboard input.  Still to do: a run with CWSDPMI, a run in
  DOSBox-X, sound by ear, and the Windows `dosbox-x-build.ps1`
  script, which hasn't been run on Windows.
- **Windows 11 hardware.**  Everything Windows-specific has been
  tested under Wine only: the game, the console and message box
  handling, Winsock, and the kit from `win/make-kit.sh`.  A run on
  real Windows 11 is still needed, including the firewall prompt for
  network play.
- **Packaging** for Linux (a `.desktop` file and an icon) and
  Flatpak.
- **CI** for the Linux, MinGW and DJGPP builds.
- **Sound on real hardware.**  The sound drivers run without errors,
  but nobody has listened to them yet.
