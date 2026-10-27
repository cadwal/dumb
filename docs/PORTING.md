# Plan: DUMB on Linux (Wayland and X11) and Windows 11

This work lives on the `sdl-port` branch.  It starts from
`djgpp-modernize`, whose build-system and modern-GCC fixes it needs,
and lives side by side with it and with `main` (the original 0.13.11
sources, unchanged).  The DOS build from `djgpp-modernize` should keep
working here.

Goal: a `sdldumb` program that runs natively on Linux under Wayland
and X11, and on Windows 11 (64-bit), using one portable backend for
video, input, sound and timing.  The existing drivers stay; `sdldumb`
is added next to them.

## Where things stand

DUMB's platform layer is small and well separated, which makes this
practical:

| Interface | Functions | Existing drivers |
|---|---|---|
| `dumb/video.h` | `video_preinit`, `init_video`, `reset_video`, `video_setpal`, `video_newframe`, `video_updateframe`, `video_winstuff` | X11 (`x11_video.c`), DOS/Allegro, SVGAlib, GGI, AA-lib, Linux fbdev, dummy |
| `dumb/input.h` | `init_input`, `get_input`, `reset_input` | X11 (in `x11_video.c`), DOS, SVGAlib, GGI, AA-lib, dummy |
| `libdumb/sound.h` | `init_sound`, `play_sound`, `poll_sound`, ... | OSS `/dev/dsp`, MME, Allegro, dummy |
| `dumb/net.h` | `NetDriver` table | Unix sockets (`unix_net.c`), dummy |
| `libdumbutil/timer.h` | `init_timer`, `reset_timer`, `read_timer` | `setitimer` (`timer.c`), DOS |

What blocks the goal today:

- **Wayland:** the X11 driver uses Xlib, XShm, DGA and XKB, so on a
  Wayland desktop it only runs through XWayland.  SVGAlib, GGI and
  fbdev are obsolete.
- **Sound:** OSS `/dev/dsp` is gone on current distributions (it
  needs `osspd` or `padsp`).
- **Windows:** the timer uses `setitimer`/`SIGALRM`, networking uses
  BSD sockets, the config file is `$HOME/.dumbrc`, file names are
  checked for `/` only, and configure insists on `sys/wait.h`.  `mmap`
  is already optional (done for DJGPP).
- **64-bit Windows** is LLP64 (`long` is 32 bits).  configure picks
  the 64-bit rerenderer by `sizeof(long)`, so Win64 would quietly get
  the slower 32-bit one; other `long`/pointer assumptions are not yet
  audited.
- Already fine: the native x86_64 Linux build works (the x86 asm is
  32-bit only; other CPUs use the C renderer) and plays E1M1 with the
  DUMB game or Freedoom.

## Approach

One new backend on **SDL** (video, input, audio, timer), built as a
new program `sdldumb`.  SDL gives native Wayland, X11 and Windows
from the same code, plus fullscreen, scaling, HiDPI and gamepads.

### Decision: SDL3

Decided: **SDL3** only.  It is the current API, prefers Wayland by
default, and is packaged in current Fedora, Debian 13, Arch and MSYS2.
Ubuntu 24.04 doesn't package it, so there it has to be built from
source.  (SDL2 was the alternative: packaged everywhere, but on
Wayland it usually runs through XWayland.)

## Steps

1. **Build system.**  `PKG_CHECK_MODULES([SDL], [sdl3])` in
   `configure.ac`; an `AM_CONDITIONAL` and a `sdldumb` program in
   `dumb/Makefile.am`.  Base the rerenderer choice on pointer size,
   not `sizeof(long)`.
2. **Timer** (`libdumbutil/timer.c`): replace `setitimer` with
   `SDL_GetTicks()` for `sdldumb`, and with `clock_gettime(CLOCK_MONOTONIC)`
   for the other Unix programs.  Keep the 10 ms tick.
3. **Video** (`dumb/sdl_video.c`): window plus renderer plus a
   streaming texture.  DUMB renders into a plain buffer at 8, 16 or 32
   bpp.  Render at 32 bpp when the colormaps allow it, otherwise at
   8 bpp and convert through the palette in `video_updateframe`.  Use
   the SDL renderer's logical size for scaling: integer scaling, 4:3
   aspect correction for 320x200, a fullscreen toggle, and vsync.
   `video_winstuff` sets the window title.
4. **Input** (`dumb/sdl_input.c`): keyboard state by scancode,
   mapped through DUMB's keymap (`keymap.c`), with SDL key names for
   configuration; mouse in relative mode; window close and focus
   events.  Gamepads optional.
5. **Sound** (`libdumb/sdl_sound.c`): an SDL audio stream fed by the
   existing software mixer (`sound_mix.c`), as `linux_sound.c` does
   for OSS.
6. **Windows specifics:**
   - Build with MinGW-w64 (MSYS2 UCRT64): autotools works there.
     Make `sys/wait.h` and `fork` optional in configure.
   - Config file in `%APPDATA%\DUMB\dumb.cfg`.
   - In `safeio.c`, treat `\` and drive letters as path separators,
     as is already done for `__MSDOS__`.
   - Use `SDL_main`, and log to a file, since there is no console.
   - Networking: Winsock in `unix_net.c` (`WSAStartup`,
     `closesocket`), or the dummy driver at first.
   - NLS off by default.
7. **Packaging:**
   - Linux: `make install` as usual.
   - Windows: a zip with `sdldumb.exe`, `SDL3.dll`, `doom4dum.wad`,
     `htic4dum.wad` and the DUMB game's `game.wad`.
   - A script in the spirit of `dos/cross-build.sh` that builds the
     Windows zip from Linux with MinGW-w64.
8. **Testing:**
   - Linux X11 under Xvfb, and Wayland under a headless compositor
     (e.g. `weston --backend=headless`).
   - Windows: MinGW-w64 cross build run under Wine, then a real
     Windows 11 check.
   - Compare frames with `xdumb`'s output for the same level.
   - Optional GitHub Actions: Ubuntu, plus Windows with MSYS2.

Not in scope for now: porting the XWad editor (X11 only; it still
runs through XWayland), and removing the old drivers.

## Status

- [x] Decided: SDL3.
- [x] Step 1: build system.  `configure` finds SDL3 with pkg-config
  (`--without-sdl` turns it off; it is off by default for DJGPP, where
  pkg-config would find the build machine's SDL) and builds `sdldumb`.
  SDL3 is also the preferred sound driver for all programs
  (`--with-sdl-sound`).  The rerenderer is chosen by pointer size,
  and `le64_fbrerend.c` uses `uint64_t`, so 64-bit Windows gets it too.
- [x] Step 2: timer.  `libdumbutil/timer.c` uses
  `clock_gettime(CLOCK_MONOTONIC)`, or `QueryPerformanceCounter` on
  Windows, instead of `setitimer`.  It keeps the 10 ms tick and the old
  cap of 99 ticks, and no longer loses the part of a tick left over at
  each reset (tested: 50 x 7 ms gives 35 ticks).
- [x] Steps 3-5: `dumb/sdl_video.c`, `dumb/sdl_input.c`,
  `libdumb/sdl_sound.c`, tested with SDL 3.4.8 (built from source):
  - Linux X11 (Xvfb) and native Wayland (headless Weston): same frames
    as `xdumb`; 8, 16 and 32 bpp; ~60 fps with vsync.
  - Video: resizable window, starting at the largest whole multiple
    that fits; 320x200 shown at 4:3; `--vid-fullscreen`,
    `--vid-scale`, `--vid-no-aspect`, `--vid-integer-scale`,
    `--vid-no-vsync`; F11 or Alt+Enter toggles fullscreen.  SDL is
    started in `init_video()`, so `--help` works without a display.
  - Input: SDL keycodes with key names compatible with the X11
    driver's, so existing key bindings work; keys are released when
    the window loses focus; a key pressed and released within one frame
    still counts; mouse in relative mode (x turns, y looks, or walks
    with `--input-mouse-move`; left button shoots, right activates,
    middle walks); `--input-no-mouse`, `--input-mouse-sens`.
    Keyboard (turn, walk, Escape) and mouse turning tested with xdotool.
  - Sound: an SDL audio stream topped up from `sound_mix.c` in
    `poll_sound()`; `--sound-mono`, `--sound-latency`.  Only tested
    with SDL's dummy audio driver (no sound device here).
- [x] Step 6 (mostly): `sdldumb.exe` cross-built with MinGW-w64 and an
  SDL3 built the same way; it needs only `SDL3.dll` besides Windows'
  own DLLs, and runs under Wine with SDL's Windows video driver and the
  Direct3D renderer, drawing the same frames as on Linux.
  - configure no longer requires `sys/wait.h`; `test/fixed_bm` is
    only built where `setitimer()` exists.
  - Config file: `dumb.cfg`, in the current directory or
    `%APPDATA%\DUMB\`.  `\` and drive letters count as paths.
  - Logging: started from cmd.exe or PowerShell, or with stdout
    redirected, DUMB prints to the console (`AttachConsole`).
    Otherwise it logs to `%APPDATA%\DUMB\dumb.log` (or `dumb.log` in
    the current directory, if one is there), and a fatal error is also
    shown in a message box that names the log.  `-l FILE` works as
    before.  Tested under Wine with a launcher that starts DUMB without
    standard handles, as Explorer does.
  - WADs are also looked for in the directory of `sdldumb.exe`, so it
    can be started from anywhere.
  - Networking: `dumb/win_net.c`, UDP over Winsock 2 (`WSAStartup`,
    `getaddrinfo`, `ioctlsocket`, `select`), chosen by configure for
    MinGW and linked with `-lws2_32`.  Tested under Wine: two Windows
    instances, and a Linux master with a Windows slave.
  - Still to do: a real Windows 11 check.
- Bugs found in the original code while running it, fixed here and on
  `djgpp-modernize` (DOS takes the same no-mmap paths):
  - WAD reading without mmap: the directory was read from the wrong
    file position, and `load_lump()` passed the lump length to
    `lseek()` as `whence`.
  - Protos were `malloc`'d and their `sprites` pointer never set, so
    sprites crashed at random (reproducible with `MALLOC_PERTURB_`).
  - `reset_misc()` aborted on exit if no misc texture was ever loaded.
  - `logvprintf()` used one `va_list` for every log file; on x86-64
    the second file got garbage.  It now uses `va_copy()`.
  - Network play: when the master got a slave's join request, it
    looked the slave up again by the host name the slave sent, with
    the default port.  With a port given in `--dumb-master`, or two
    players on one machine, the master then sent to itself forever
    unless its first packet happened to arrive first.  The master now
    keeps the address the request came from.
- [x] Step 7: packaging.  `win/make-kit.sh` builds SDL3 for MinGW-w64
  (or uses `SDL3_DIR`), the WADs, the DUMB game and `sdldumb.exe`, and
  makes `dumb-VERSION-win64.zip` with `sdldumb.exe`, `SDL3.dll`,
  `game.wad`, `doom4dum.wad`, `htic4dum.wad`, a `DUMB.cmd` launcher, a
  `README.txt` and the licences.  It checks that the program needs no
  DLLs but SDL3's and Windows' own.  The unpacked kit was tested
  under Wine.  Linux `make install` works as before.
- [~] Step 8: testing as above; no CI yet.

The existing programs still build and run on this branch: `xdumb`
plays the DUMB game as before, with the new timer, and the DOS cross
build is unaffected.
