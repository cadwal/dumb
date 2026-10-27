# Plan: building dosdumb with current DJGPP

This work lives on the `djgpp-modernize` branch, side by side with
`main`, which keeps the original DUMB 0.13.11 sources unchanged.  The
DUMB game (cadwal/dumb-game) is set up the same way.

Goal: build `dosdumb.exe` with a current DJGPP toolchain (GCC 12+,
cross-compiled from Linux as `i586-pc-msdosdjgpp`) against Allegro 4.2,
the last Allegro line with working DOS support.

The DOS-specific code needs few changes.  Most of the work is the
1999-era autotools setup and a handful of C / inline-asm constructs
that modern GCC rejects.  Those affect every port, not only DOS.

## 1. Build system

- `acinclude.m4`: `DUMB_ARG_HELP` uses `AC_DIVERT_PUSH(AC_DIVERSION_NOTICE)`,
  which modern autoconf no longer has; `aclocal` aborts in m4.  Replace
  it with `AS_HELP_STRING` / `m4_divert_text([HELP_ENABLE], ...)`.
- Quote macro names in `AC_DEFUN`, move to `configure.ac`, use
  `AC_INIT([dumb], [0.13.11])` + `AM_INIT_AUTOMAKE([foreign])`,
  `AC_CONFIG_HEADERS`, `AH_TEMPLATE` (or `AC_DEFINE` with a description)
  instead of `acconfig.h`, and `AM_CPPFLAGS` instead of `INCLUDES`.
- gettext: the bundled `intl/` (gettext 0.10.35) and old
  `AM_GNU_GETTEXT` do not work with current tools.  Use
  `AM_GNU_GETTEXT([external])`.  DOS keeps `--disable-nls`, because
  DJGPP's conio `gettext()` still clashes with libintl.
- Cross-compiling: support `./configure --host=i586-pc-msdosdjgpp
  --disable-nls`.  `AC_TRY_RUN` checks need cross-compile defaults.
  Host-only tools (`ptcomp`, `ppmtodumb`, WAD building) must be built
  for the build machine or skipped when cross-compiling.
- Drop `-fenum-int-equiv` from the optional C++ build hack (removed
  from g++ long ago).

## 2. Core C code rejected by modern GCC

| Where | Problem |
|---|---|
| `libdumbutil/fixed.h` (`fixdiv`) | asm input in `edx` also listed as clobbered ("impossible constraints") |
| `dumb/rendsl86.h` (`SLICE_STRING`) | same with `edi`; `shrl %5` with `"g"` must be `"c"`/immediate |
| `libdumbutil/intset.c` | `readonly++` through a `const struct intset *` (debug builds) |
| `libdumb/gettableinwad.h`, `.c`, `protoinwad.c` | array parameters of incomplete struct type; use pointers |
| `libdumbutil/fixed.c` vs `fixed.h` | declarations have `ATTR_REGPARM`, definitions don't |
| various | missing `<string.h>`, `<stdlib.h>`, `<assert.h>` includes (errors on GCC 14+) |

Validate with a native Linux build (`dummydumb`, `xdumb`) before
touching DOS.

## 3. dosdumb (Allegro 3 -> Allegro 4.2)

Nearly all of the Allegro API used is unchanged in 4.2 (`GFX_VESA2L`,
`allegro_error`, `is_linear_bitmap`, `screen->seg`, `scroll_screen`,
`get_mouse_mickeys`, `key[]`, `mouse_b`, `KEY_*`).  Changes needed:

- `libdumb/dos_sound.c` `make_sample()` fills a `SAMPLE` by hand;
  Allegro 4 added `stereo`, left as `malloc` garbage.  Zero the struct
  (or use `create_sample()`).
- `fixed` typedef clash: Allegro 4.2 has `typedef int32_t fixed`
  (`long` in DJGPP), DUMB has `typedef int fixed`.  Guard every Allegro
  user the way `dos_sound.c` already does.
- Add `#include <dpmi.h>` for `__dpmi_get_segment_base_address`.
- Allegro 4.2 headers with modern GCC usually need `-fgnu89-inline`.
- `docs/dos/` netpbm files are obsolete; only needed to rebuild WADs.

## Steps

1. Modernise autotools until `autoreconf -fi` succeeds.
2. Fix section 2; check with a native Linux build.
3. Cross-compile Allegro 4.2 with DJGPP; configure DUMB with
   `--host=i586-pc-msdosdjgpp --disable-nls`.
4. Patch the DOS files (section 3) and link `dosdumb.exe`.
5. Run in DOSBox to catch runtime regressions (asm, sound).

## Status

- [x] Step 1: `autoreconf -fi` and `configure` work with autoconf 2.71 / automake 1.16.  The bundled `intl/` is gone; NLS uses the C library or an installed libintl (`DUMB_NLS` in `acinclude.m4`).  The custom `po/Makefile.in.in` (with its `-utf8` catalogs) is kept.
- [x] Step 2: native build (xdumb, dummydumb, ptcomp, tools, WADs, manual) is clean with GCC 13, with and without NLS, also under GCC 14's stricter defaults (`-Werror=implicit-function-declaration` etc.).  All C files also compile as 32-bit x86 at -O0 and -O2.  The rewritten x86 asm (wall/trans slices, flat loop, memmove, fixdiv) was checked against the C fallbacks and a 64-bit reference in a freestanding 32-bit test harness; mutation testing confirmed the harness catches broken asm.  Extra fixes found on the way: `Chance` in SoundType is signed (failed on 64-bit hosts), `wadtool`'s `basename` clashed with libc, libpng's `jmpbuf` field is private now, `wadchain.hh` access errors in modern g++, `safe_munmap` called without mmap (DJGPP), and code calling `gettext()`/`dgettext()` directly with NLS disabled.
- [x] Step 3: Allegro 4.2 (branch `4.2`, 4.2.4 SVN) cross-built with
  DJGPP 2.05 / GCC 12.2 (build-djgpp v3.4).  `configure
  --host=i586-pc-msdosdjgpp` defaults to `--disable-nls` on DJGPP,
  detects Allegro 4 (`allegro_init()` is a macro for
  `_install_allegro_version_check`), skips building the WADs (ptcomp and
  wadtool can't run on the build machine; use the WADs from a native
  build), only replaces `hstrerror()` for the Unix network driver, and
  uses real `sizeof` results (le32 rerenderer).  DJGPP has no
  `<wctype.h>` functions, so libmissing provides ASCII/Latin-1 ones.
- [x] Step 4: `dosdumb.exe` (plus `ptcomp.exe`, `wadtool.exe`, ...)
  builds and links with no warnings.  `libdumb/dosalleg.h` makes
  Allegro's inline functions `static` (the original objects each
  defined ~150 Allegro functions globally) and renames DUMB's inline
  `fixed`, `fixmul`, `fixdiv`.  DUMB's `fixsin`/`fixcos`/`fixtan` are
  now `fix_sin`/`fix_cos`/`fix_tan`, because liballeg.a exports the old
  names.  `make_sample()` zeroes the `SAMPLE` (new `stereo` field).
- [x] Bugs in the original code found while porting (on `sdl-port`)
  and fixed here too, because DOS takes the affected code paths:
  without mmap (DJGPP has none), the WAD directory was read from the
  wrong file position and `load_lump()` passed the lump length to
  `lseek()` as `whence`, so `dosdumb.exe` could not have loaded any
  WAD; protos were `malloc`'d with an uninitialised `sprites` pointer
  (random crashes); and `reset_misc()` aborted on exit when no misc
  texture had been loaded (the DUMB game).
- [ ] Step 5: started.  Under DOSBox 0.74 the executable loads and the
  DJGPP stub stops with `Load error: no DPMI - Get csdpmi*b.zip`, as
  expected without a DPMI host.  CWSDPMI could not be downloaded in the
  session that did this, so the game itself has not been run yet.

## Scripts

`dos/` automates all of this; see `dos/README`:

- `dos/cross-build.sh`: Linux/macOS cross build (toolchain, Allegro,
  WADs, the DUMB game, dosdumb.exe, package in `build-dos/dist`,
  optional `--run`).  Tested end to end from an empty work directory.
- `dos/get-cwsdpmi.sh`: downloads CWSDPMI.EXE.
- `dos/dosbox-x-build.ps1` (+ `.cmd`): Windows; sets up DJGPP 2.05,
  Allegro and DUMB on a DOSBox-X C: drive, builds natively in DOS with
  `dos/makefile.dj` and runs dosdumb.  **Not tested yet**: it needs
  Windows, and the DJGPP mirror was not reachable from where it was
  written.  `dos/makefile.dj` itself was tested by cross-compiling
  with it (its WADs are byte-identical to the configure build's).

The default game data is the DUMB game, cadwal/dumb-game, branch
`djgpp-modernize`: its `.pt` files were ported to the current ptcomp
syntax and it has a `prebuilt/game.wad`.  `xdumb -w game.wad` and
`dummydumb -w game.wad` play its E1M1 with nothing else loaded.  On
DUMB's `modernize` branch, the scripts use dumb-game's `modernize`
branch instead, which also has starts for 4 players.
Freedoom or Doom remain available with `--iwad`/`--freedoom`
(`-Iwad`/`-Freedoom` on Windows).

## Reproducing the DOS build by hand

```sh
# DJGPP cross-compiler: build-djgpp v3.4 (GCC 12.2.0)
#   https://github.com/andrewwutw/build-djgpp/releases
DJ=/path/to/djgpp
export PATH=$DJ/bin:$PATH

# Allegro 4.2 (branch 4.2 of https://github.com/liballeg/allegro5).
# Its cross build expects unprefixed gcc/ar/... on PATH.
mkdir shim
for t in gcc g++ cpp; do ln -s $DJ/bin/i586-pc-msdosdjgpp-$t shim/$t; done
cd allegro-4.2
sh fix.sh djgpp --quick
M="make CROSSCOMPILE=1 DJDIR=$DJ/i586-pc-msdosdjgpp"
PATH=$PWD/../shim:$DJ/i586-pc-msdosdjgpp/bin:$PATH $M obj/djgpp/asmcapa.h
PATH=$PWD/../shim:$DJ/i586-pc-msdosdjgpp/bin:$PATH $M obj/djgpp/asmdef.inc
PATH=$PWD/../shim:$DJ/i586-pc-msdosdjgpp/bin:$PATH $M -j8 lib
# Stage it: include/allegro.h, include/allegro/ -> $ALLEG/include,
#           lib/djgpp/liballeg.a              -> $ALLEG/lib

# DUMB
cd dumb && autoreconf -fi
mkdir ../build-dos && cd ../build-dos
../dumb/configure --host=i586-pc-msdosdjgpp \
  CPPFLAGS=-I$ALLEG/include LDFLAGS=-L$ALLEG/lib
make
# dumb/dosdumb.exe; take doom4dum.wad / htic4dum.wad from a native build
```

Notes:

- `asmcapa.h` and `asmdef.inc` must be made first; `fix.sh --quick`
  skips `make depend`, so `make lib` doesn't know it needs them.
- The prebuilt build-djgpp `ar` needs the flex runtime (`libfl.so.2`,
  package `libfl2` on Debian/Ubuntu).
- To run it: put `CWSDPMI.EXE` (csdpmi7b.zip) next to `dosdumb.exe`,
  together with an IWAD and `doom4dum.wad`, and start it in DOSBox.
  Check the renderer at 8/16/32 bpp, page flipping, mouse and sound.
