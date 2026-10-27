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
- Compiler warnings still left: `-Wformat` warnings in `texture.c`
  and `netplay.c`, and `-Wformat-security` in `updthing.c`.  Look at
  those before trusting messages that contain `%`.

## Open items

- [ ] Run `DOSDUMB.EXE` in DOSBox-X with CWSDPMI.
- [ ] Try `dos/dosbox-x-build.ps1` on Windows.
- [ ] Run the kit on real Windows 11: play, quit with Esc, and allow
      network play through the firewall prompt.
- [ ] Listen to the sound on real hardware.
- [ ] Network play between two real machines.
- [ ] CI for the Linux, MinGW and DJGPP builds.
- [ ] Linux packaging (a `.desktop` file, an icon, maybe Flatpak).
- [ ] Fix the remaining format warnings listed under Gotchas.
