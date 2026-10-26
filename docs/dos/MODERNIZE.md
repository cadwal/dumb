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

- [ ] Step 1
- [ ] Step 2
- [ ] Step 3
- [ ] Step 4
- [ ] Step 5
