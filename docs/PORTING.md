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

### Decision: SDL3 or SDL2

- **SDL3 (recommended):** current API; prefers Wayland by default;
  packaged in current Fedora, Debian 13, Arch and MSYS2.  Not in
  Ubuntu 24.04, where it has to be built from source (or vendored).
- **SDL2:** packaged everywhere, including Ubuntu 24.04, but on
  Wayland it usually runs through XWayland unless
  `SDL_VIDEODRIVER=wayland` is set.

The code is small enough that supporting both behind a few macros is
possible, but the plan assumes SDL3 only unless decided otherwise.

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

- [ ] Decide SDL3 vs SDL2
- [ ] Step 1: build system
- [ ] Step 2: timer
- [ ] Step 3: video
- [ ] Step 4: input
- [ ] Step 5: sound
- [ ] Step 6: Windows
- [ ] Step 7: packaging
- [ ] Step 8: testing
