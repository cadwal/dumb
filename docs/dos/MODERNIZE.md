# Plan: building dosdumb with current DJGPP

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
- [~] Step 3: DUMB side done; Allegro build and the real cross-build
  **not yet run** (see below).  `configure --host=i586-pc-msdosdjgpp`
  now: defaults to `--disable-nls` on DJGPP, detects Allegro 4 (where
  `allegro_init()` is a macro for `_install_allegro_version_check`),
  skips building the WADs (ptcomp/wadtool can't run on the build
  machine; use the WADs from a native build), only replaces
  `hstrerror()` for the Unix network driver (it needs `<netdb.h>`), and
  gets real `sizeof` results so the le32 rerenderer is used.  Checked by
  configuring and building with `--host=i586-pc-msdosdjgpp` and the
  native gcc, which exercises the cross-compiling code paths.
- [~] Step 4: DOS sources patched; compile-checked, **not linked**.
  The new `libdumb/dosalleg.h` makes Allegro's inline functions
  `static` (the original objects each defined ~150 Allegro functions
  globally, so they could never link) and renames DUMB's `fixed`,
  `fixmul`, `fixdiv`, `fixsin`, `fixcos`, `fixtan`, which clash with
  Allegro's.  `make_sample()` zeroes the `SAMPLE` (new `stereo` field).
  `dos_video.c` includes `<dpmi.h>` and `<string.h>`.  All three files
  compile with `-Wall` and no warnings against the DJGPP 2.05 libc
  headers and Allegro 4.2 headers (host gcc `-m32` with DJGPP's
  predefined macros); the original `dos_sound.c` fails that check.
- [ ] Step 5

## Finishing steps 3-5

These need the DJGPP toolchain to run; in the session that made these
changes, running downloaded toolchain and Allegro code was not
permitted, so they are still to do.

```sh
# DJGPP cross-compiler, e.g. a build-djgpp release (GCC 12.2):
#   https://github.com/andrewwutw/build-djgpp/releases
export PATH=/path/to/djgpp/bin:$PATH
DJ=/path/to/djgpp/i586-pc-msdosdjgpp

# Allegro 4.2 (branch 4.2 of https://github.com/liballeg/allegro5)
cd allegro-4.2
sh fix.sh djgpp --quick
make CROSSCOMPILE=1 DJDIR=$DJ lib
make CROSSCOMPILE=1 DJDIR=$DJ install   # headers + liballeg.a into $DJ

# DUMB
cd dumb && autoreconf -fi
mkdir ../build-dos && cd ../build-dos
../dumb/configure --host=i586-pc-msdosdjgpp --disable-nls
make
# dumb/dosdumb.exe; take doom4dum.wad / htic4dum.wad from a native build
```

Things to watch for:

- Allegro 4.2 itself may need `-fgnu89-inline` (or similar) added to
  its CFLAGS with GCC 12.
- If `configure` reports "Allegro not found", check `config.log`: the
  test links `-lalleg`.
- Run `dosdumb.exe` in DOSBox (with a CWSDPMI next to it) and check
  the renderer at 8/16/32 bpp, page flipping, mouse and sound.
