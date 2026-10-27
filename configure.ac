dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([dumb], [0.13.11])
AC_CONFIG_SRCDIR([libdumb/dsound.h])
AC_CONFIG_AUX_DIR([scripts])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([config.h dircfg.h])

# $datadir is usually '${datarootdir}', which is '${prefix}/share'.
# Expand it fully.  This does not handle spaces in $datadir.
dumb_save_prefix="$prefix"
test "x$prefix" = xNONE && prefix="$ac_default_prefix"
dumb_expanded_datadir="$datadir"
while :; do
  case "$dumb_expanded_datadir" in
    *'$'*) eval "dumb_expanded_datadir=\"$dumb_expanded_datadir\"";;
    *) break;;
  esac
done
prefix="$dumb_save_prefix"
DUMB_DEFINE_UNQUOTED(DUMB_CONFIG_DUMBDATADIR, "${dumb_expanded_datadir}/dumb")

dnl --- Optional parts.
AC_MSG_CHECKING([whether libdumbworldb is requested])
AC_ARG_ENABLE([libdumbworldb],
  [AS_HELP_STRING([--enable-libdumbworldb], [enable the worldbase (doesn't work yet)])],
  [:],
  [enable_libdumbworldb="no"])
dnl This is of course not cached.
if test "$enable_libdumbworldb" = "no"; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([yes])
  AC_DEFINE([DUMB_CONFIG_LDWB], [1], [Define to enable libdumbworldb])
  AC_MSG_WARN([libdumbworldb is rather incomplete; you may have problems])
fi
AM_CONDITIONAL(DUMB_CONFIG_LDWB, [test "$enable_libdumbworldb" != "no"])

dnl --- Checks for programs.
AC_PROG_CC
DUMB_MSG_REQUIRE_YES(GCC, [GCC])
dnl GCC always supports -c -o
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_LN_S

AC_USE_SYSTEM_EXTENSIONS

dnl --- Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_C_BIGENDIAN
DUMB_C_ATTR_REGPARM
AC_TYPE_SIZE_T
dnl These work when cross-compiling too.
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF([void *])
AC_MSG_CHECKING(which fbrerend to use)
fbrerend="generic"
if test $ac_cv_c_bigendian = no; then
  dnl le64 uses uint64_t; pick it for 64-bit CPUs, also where long is
  dnl 32 bits (64-bit Windows).
  if test $ac_cv_sizeof_void_p -ge 8; then
    fbrerend="le64"
  elif test $ac_cv_sizeof_int -ge 4; then
    fbrerend="le32"
  fi
fi
AC_MSG_RESULT($fbrerend)
DUMB_FBREREND_SOURCES="${fbrerend}_fbrerend.c"
AC_SUBST(DUMB_FBREREND_SOURCES)

dnl --- C++ compilation hack.
dnl The C-as-C++ compiler invocation command is saved in $CXX_HACK_CC
dnl and not directly in $CC, so that configure tests still use C.
AC_ARG_ENABLE([c],
  [AS_HELP_STRING([--disable-c], [always compile as C++, not C])],
  [if test "$enableval" != "no"; then
    AC_MSG_WARN([--enable-c not supported])
  fi],
  [enable_c=yes])
if test "$ac_cv_c_bigendian" = "yes" || test "$enable_c" = "no"; then
  DUMB_PROG_CXX_C
  CXX_HACK_CC="$dumb_cv_prog_cxx_c"
  # The Makefiles in the hack-needing directories assign $CXX_HACK_CC
  # to CC.  $CC is used for linking too and the -x c++ in it would
  # cause the .o files to be interpreted as C++ too!  Override that.
  # This assumes GCC but that's fine.
  LDFLAGS="-x none $LDFLAGS"
else
  CXX_HACK_CC="$CC"
fi
AC_SUBST(CXX_HACK_CC)

dnl --- Internationalization.
dnl po/Makefile.in.in is DUMB's own; it also installs the -utf8 catalogs.
dnl DJGPP's libc has a conio gettext() that conflicts with libintl.
case "$host_os" in
  msdosdjgpp*) dumb_nls_default=no ;;
  *) dumb_nls_default=yes ;;
esac
DUMB_NLS([$dumb_nls_default])
ALL_LINGUAS="fi"
if test -n "${LINGUAS+set}"; then
  dumb_linguas=""
  for lang in $ALL_LINGUAS; do
    case " $LINGUAS " in *" $lang "*) dumb_linguas="$dumb_linguas $lang";; esac
  done
else
  dumb_linguas="$ALL_LINGUAS"
fi
POFILES=""; GMOFILES=""; CATALOGS=""
for lang in $ALL_LINGUAS; do
  POFILES="$POFILES $lang.po"
  GMOFILES="$GMOFILES $lang.gmo"
done
for lang in $dumb_linguas; do
  CATALOGS="$CATALOGS $lang.gmo"
done
CATOBJEXT=".gmo"
INSTOBJEXT=".mo"
DATADIRNAME="share"
AC_SUBST([POFILES])
AC_SUBST([GMOFILES])
AC_SUBST([CATALOGS])
AC_SUBST([CATOBJEXT])
AC_SUBST([INSTOBJEXT])
AC_SUBST([DATADIRNAME])
POTFILES=`sed -e '/^#/d' -e '/^[[ 	]]*$/d' -e 's,^,$(top_srcdir)/,' "$srcdir/po/POTFILES.in" | tr '\n' ' '`
AC_SUBST([POTFILES])
DUMB_DEFINE_UNQUOTED(LOCALEDIR, "${dumb_expanded_datadir}/locale")

dnl --- X
AC_PATH_X
AC_PATH_XTRA
AM_CONDITIONAL(DUMB_CONFIG_X, test -z "$no_x")
if test -z "$no_x"; then
  # Always have the X include and library directories in the search paths,
  # even if the file being compiled or linked doesn't use X.
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  LDFLAGS="$LDFLAGS $X_LIBS"
  # This variable is used below, and in the aalib check.
  x_ldadd="$X_PRE_LIBS -lX11 $X_POST_LIBS"
  # sparc-sun-solaris2.5.1 lacks XInternalConnectionNumbers().
  save_LIBS="$LIBS"
  LIBS="$LIBS $x_ldadd"
  AC_CHECK_FUNCS([XInternalConnectionNumbers])
  LIBS="$save_LIBS"
else
  x_ldadd=""
  AC_MSG_WARN([X not found; programs 'xdumb', 'mkdfnt', 'XWad' and 'XProtoThing' will not be built])
fi

dnl --- Netpbm library.
dumb_ppm_h=""
DUMB_PPM_LDADD=""
dumb_have_libppm=no
dnl Current netpbm installs its headers in netpbm/.
AC_CHECK_HEADER(ppm.h, [dumb_ppm_h="<ppm.h>"],
  [AC_CHECK_HEADER(netpbm/ppm.h, [dumb_ppm_h="<netpbm/ppm.h>"],
    [AC_CHECK_HEADER(gr/ppm.h, [dumb_ppm_h="<gr/ppm.h>"])])])
if test -n "$dumb_ppm_h"; then
  AC_DEFINE_UNQUOTED([DUMB_CONFIG_PPM_H], [$dumb_ppm_h],
    [Define so that `#include DUMB_CONFIG_PPM_H' includes <ppm.h> or similar.])
  AC_CHECK_LIB(ppm, ppm_readppm, dumb_have_libppm=yes, dumb_have_libppm=no,
    -lpgm -lpbm)
  test "$dumb_have_libppm" = "yes" && DUMB_PPM_LDADD="-lppm -lpgm -lpbm"
  if test "$dumb_have_libppm" = "no"; then
    AC_CHECK_LIB(netpbm, ppm_readppm, dumb_have_libppm=yes)
    test "$dumb_have_libppm" = "yes" && DUMB_PPM_LDADD="-lnetpbm"
  fi
fi
if test "$dumb_have_libppm" = "no"; then
  AC_MSG_WARN([ppm_readppm() not found; programs 'dark2trans' and 'ppmtodumb' will not be built])
fi
AM_CONDITIONAL(DUMB_CONFIG_LIBPPM, test "$dumb_have_libppm" = "yes")
AC_SUBST(DUMB_PPM_LDADD)

dnl --- PNG library.
DUMB_CHECK_HDR_LIB_YESNO(png.h, png, png_read_image, [], dumb_have_libpng)
if test "$dumb_have_libpng" = "no"; then
  AC_MSG_WARN([libpng not found; program 'pngtodumb' will not be built])
fi
AM_CONDITIONAL(DUMB_CONFIG_LIBPNG, test "$dumb_have_libpng" = "yes")

dnl --- Math library.
AC_CHECK_LIB(m, sin, ,
  [AC_MSG_ERROR([DUMB requires the math library])])

dnl --- AA-lib.
DUMB_AA_LDADD=""
dumb_have_aalib="no"
AC_CHECK_HEADER([aalib.h],
 [dumb_aa_save_LIBS="$LIBS"
  LIBS=""
  DUMB_SEARCH_LIBSETS([aa_renderpalette],
   ["-laa" "-laa -lslang $x_ldadd" "-laa -lncurses $x_ldadd"],
   [dumb_have_aalib="yes"
    DUMB_AA_LDADD="$LIBS"])
  LIBS="$dumb_aa_save_LIBS"])
AC_SUBST([DUMB_AA_LDADD])
AM_CONDITIONAL(DUMB_CONFIG_AALIB, [test "$dumb_have_aalib" = "yes"])
if test "$dumb_have_aalib" = "no"; then
  AC_MSG_WARN([AA-lib not found; program 'aadumb' will not be built])
fi

dnl --- LibGGI.
DUMB_GGI_SOURCES=""
DUMB_CHECK_HDR_LIB([ggi/ggi.h], [ggi], [ggiParseMode], [],
  [DUMB_CHECK_IFDEF([ggi/ggi.h], [GII_KT_LATIN1],
    [DUMB_GGI_SOURCES="ggi_renoir_video.c"],
    [AC_MSG_WARN([LibGGI must be at least version 2.0b2 (Renoir)])
    AC_MSG_WARN([Program 'ggidumb' will not be built])])],
  [AC_MSG_WARN([LibGGI not found; program 'ggidumb' will not be built])])
dnl AC_SUBST(DUMB_GGI_SOURCES)
AM_CONDITIONAL(DUMB_CONFIG_GGI, [test -n "$DUMB_GGI_SOURCES"])

dnl --- SVGAlib.
DUMB_CHECK_HDR_LIB_YESNO(vga.h, vga, vga_setmode, [],
 dumb_have_svgalib)
AM_CONDITIONAL(DUMB_CONFIG_SVGALIB, test "$dumb_have_svgalib" = "yes")
if test "$dumb_have_svgalib" = "no"; then
  AC_MSG_WARN([SVGAlib not found; program 'ldumb' will not be built])
fi

dnl --- Allegro.
dnl In Allegro 4, allegro_init() is a macro for this function.
dumb_have_allegro=no
AC_CHECK_HEADER([allegro.h],
  [AC_CHECK_LIB([alleg], [_install_allegro_version_check],
    [dumb_have_allegro=yes],
    [AC_CHECK_LIB([alleg], [allegro_init], [dumb_have_allegro=yes])])])
AM_CONDITIONAL(DUMB_CONFIG_DOS, test "$dumb_have_allegro" = "yes")
if test "$dumb_have_allegro" = "no"; then
  AC_MSG_WARN([Allegro not found; program 'dosdumb' will not be built])
fi

dnl --- SDL3: video, input and sound for Wayland, X11 and Windows.
dnl Off by default on DJGPP, where pkg-config would otherwise find the
dnl build machine's SDL.
AC_ARG_WITH([sdl],
  [AS_HELP_STRING([--without-sdl], [don't use SDL3 (for sdldumb and sound)])],
  [], [case "$host_os" in msdosdjgpp*) with_sdl=no ;; *) with_sdl=check ;; esac])
dumb_have_sdl=no
if test "$with_sdl" != no; then
  PKG_CHECK_MODULES([SDL], [sdl3], [dumb_have_sdl=yes],
    [if test "$with_sdl" = yes; then
       AC_MSG_ERROR([--with-sdl was given, but SDL3 was not found])
     fi])
fi
AM_CONDITIONAL(DUMB_CONFIG_SDL, test "$dumb_have_sdl" = "yes")
if test "$dumb_have_sdl" = "no"; then
  AC_MSG_WARN([SDL3 not found; program 'sdldumb' will not be built])
fi

dnl --- Checks for header files.
AC_HEADER_SYS_WAIT
DUMB_MSG_REQUIRE_YES(ac_cv_header_sys_wait_h, [sys/wait.h])
AC_CHECK_HEADERS(fcntl.h limits.h sys/ioctl.h sys/time.h unistd.h)
DUMB_MSG_REQUIRE_YES(ac_cv_header_fcntl_h, [fcntl.h])
DUMB_MSG_REQUIRE_YES(ac_cv_header_limits_h, [limits.h])
DUMB_MSG_REQUIRE_YES(ac_cv_header_sys_time_h, [sys/time.h])
DUMB_MSG_REQUIRE_YES(ac_cv_header_unistd_h, [unistd.h])
dnl sys/ioctl.h is needed for sound only.

dnl --- Checks for library functions.
dnl DUMB only uses if(memcmp(...)) and if(!(memcmp(...)) and doesn't care of
dnl the sign of the return value, so memcmp() needn't be 8-bit clean.
dnl AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_CHECK_FUNCS(strdup strstr fork setitimer)
dnl sparc-sun-solaris2.5.1 keeps towlower() etc. in libw.so.
dnl DJGPP 2.05 has none of them; libmissing then has simple versions.
AC_SEARCH_LIBS(towlower, w,
  [AC_DEFINE([HAVE_TOWLOWER], [1],
    [Define if you have towlower() and the other <wctype.h> functions.])],
  [AC_LIBOBJ([wctype])])
AC_REPLACE_FUNCS(snprintf strnlen)

dnl --- Checks for system services.
DUMB_SYS_SHMAT_RMID
if test "$dumb_cv_sys_shmat_rmid" = yes; then
  AC_DEFINE([DUMB_CONFIG_SYS_SHMAT_RMID], [1],
    [Define if shmat() can attach to segments already marked IPC_RMID.])
fi

dnl --- Video.
dnl This is almost straight from info (automake)Conditionals ...
AC_ARG_ENABLE([dummy-video],
  [AS_HELP_STRING([--enable-dummy-video], [make a version that doesn't show what happens])],
[case "${enableval}" in
  yes) dummyvideo=true ;;
  no)  dummyvideo=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-dummy-video) ;;
esac], [dummyvideo=false])
AM_CONDITIONAL(DUMB_CONFIG_DUMMYVIDEO, test x$dummyvideo = xtrue)

DUMB_XSHM_LDADD=""
DUMB_DGA_LDADD=""
DUMB_XKB_LDADD=""
if test -z "$no_x"; then
  dumb_old_LIBS="$LIBS"
  LIBS="$LIBS $X_PRE_LIBS"
  DUMB_CHECK_HDR_LIB(X11/extensions/XShm.h, Xext, XShmCreateImage,
   [-lX11 $X_EXTRA_LIBS],
   [AC_DEFINE([DUMB_CONFIG_XSHM], [1], [Define if you have the MIT Shared Memory extension.])
    DUMB_XSHM_LDADD="-lXext"], [])
  # This may cause -lXext to be given twice
  # but I don't think that's a problem.
  DUMB_CHECK_HDR_LIB(X11/extensions/xf86dga.h, Xxf86dga, XF86DGADirectVideo,
   [-lXext -lX11 $X_EXTRA_LIBS],
   [AC_DEFINE([DUMB_CONFIG_XF86DGA], [1], [Define if you have the XFree86 DGA extension.])
    DUMB_DGA_LDADD="-lXxf86dga -lXext"], [])
  DUMB_CHECK_HDR_LIB(X11/XKBlib.h, X11, XkbSetDetectableAutoRepeat,
   [$X_EXTRA_LIBS],
   [AC_DEFINE([DUMB_CONFIG_XKB], [1], [Define if you have the X Keyboard Extension.])
    DUMB_XKB_LDADD="" # integrated in Xlib], [])
  LIBS="$dumb_old_LIBS"
fi
AC_SUBST(DUMB_XSHM_LDADD)
AC_SUBST(DUMB_DGA_LDADD)
AC_SUBST(DUMB_XKB_LDADD)

dnl --- Sound.
dnl This is pretty complicated.  The user can:
dnl  - select one sound system (--with-oss, --with-mme, --with-allegro)
dnl  - disable sound (--without-sound)
dnl  - autodetect but disable some systems (--without-oss, --without-mme,
dnl    --without-allegro)
dnl Disabling sound completely should have the same result as
dnl disabling each sound system separately.
dnl Note: Autoconf doesn't tell us which order the options were in, so
dnl we can't rely on that for priority.  I arbitrarily decide that
dnl --without-sound overrides individual sound systems.  The opposite
dnl way would be useless; --with-oss disables all other sound systems
dnl even without --without-sound.
dumb_sound_sdl="autodetect"
dumb_sound_oss="autodetect"
dumb_sound_mme="autodetect"
dumb_sound_allegro="autodetect"
dumb_sound_specified=""
DUMB_CONFIG_SOUND=""
DUMB_CONFIG_SOUND_LDADD=""
DUMB_ARG_HELP([[  Sound: Choose one system.  [autodetect]]])
AC_ARG_WITH([sound],
  [AS_HELP_STRING([--without-sound], [disable sound altogether])],
  [if test "$withval" = "no"; then
    DUMB_CONFIG_SOUND=dummy
    AC_MSG_CHECKING([for sound])
    AC_MSG_RESULT([disabled])
  else
    AC_MSG_WARN([--with-sound not supported; use --with-oss or --with-mme])
  fi])
DUMB_ARG_MULTICHOICE(sound, sdl, sdl-sound,
  [AS_HELP_STRING([--with-sdl-sound], [use SDL3 for sound (needs SDL3)])])
DUMB_ARG_MULTICHOICE(sound, oss, oss,
  [AS_HELP_STRING([--with-oss], [use Open Sound System (Linux usually has this)])])
DUMB_ARG_MULTICHOICE(sound, mme, mme,
  [AS_HELP_STRING([--with-mme], [use MME for sound (for Alphas)])])
DUMB_ARG_MULTICHOICE(sound, allegro, allegro,
  [AS_HELP_STRING([--with-allegro], [use Allegro for sound (for DOS)])])
DUMB_TEST_MULTICHOICE(DUMB_CONFIG_SOUND, sound, sdl, SDL3 sound,
  [test "$dumb_have_sdl" = yes ||
     AC_MSG_ERROR([--with-sdl-sound needs SDL3])
   DUMB_CONFIG_SOUND="sdl"
   DUMB_CONFIG_SOUND_LDADD='$(SDL_LIBS)'],
  [if test "$dumb_have_sdl" = yes; then
     DUMB_CONFIG_SOUND="sdl"
     DUMB_CONFIG_SOUND_LDADD='$(SDL_LIBS)'
   fi])
DUMB_TEST_MULTICHOICE(DUMB_CONFIG_SOUND, sound, oss, OSS,
  DUMB_CONFIG_SOUND="linux"
  DUMB_CONFIG_SOUND_LDADD="",
  AC_CHECK_HEADER(linux/soundcard.h,
    DUMB_CONFIG_SOUND="linux"
    DUMB_CONFIG_SOUND_LDADD=""))
DUMB_TEST_MULTICHOICE(DUMB_CONFIG_SOUND, sound, mme, MME,
  DUMB_CONFIG_SOUND="mme"
  DUMB_CONFIG_SOUND_LDADD="-lmme",
  DUMB_CHECK_HDR_LIB(mme/mme_api.h, mme, waveOutGetNumDevs, [],
    DUMB_CONFIG_SOUND="mme"
    DUMB_CONFIG_SOUND_LDADD="-lmme", []))
DUMB_TEST_MULTICHOICE(DUMB_CONFIG_SOUND, sound, allegro, ALLEGRO,
  DUMB_CONFIG_SOUND="dos"
  DUMB_CONFIG_SOUND_LDADD="-lalleg",
  AC_CHECK_HEADER(allegro.h,
    DUMB_CONFIG_SOUND="dos"
    DUMB_CONFIG_SOUND_LDADD="-lalleg", []))
AC_MSG_CHECKING(which sound driver to use)
if test -z "$DUMB_CONFIG_SOUND"; then
  DUMB_CONFIG_SOUND="dummy"
fi
AC_SUBST(DUMB_CONFIG_SOUND)
AC_SUBST(DUMB_CONFIG_SOUND_LDADD)
AC_MSG_RESULT($DUMB_CONFIG_SOUND)

dnl --- Networking.
dnl Because only Unix and dummy networking are supported,
dnl I left out the force/disable options.
dumb_network=""
AC_CHECK_HEADER(sys/socket.h,
  [dnl size_t would be logical, but sparc-sun-solaris2.5.1 really uses int.
  DUMB_CHECK_TYPE([socklen_t], [int], [#include <sys/socket.h>])
  dumb_network="unix"
  # AC_SEARCH_LIBS puts the libraries in $LIBS but we want them in
  # DUMB_NETWORK_LDADD.  This requires some extra work.
  dumb_network_old_LIBS="$LIBS"
  LIBS=""
  AC_SEARCH_LIBS(socket, socket, [],
    [AC_MSG_WARN([you have socket.h but no socket()])
     dumb_network=""])
  AC_SEARCH_LIBS(gethostbyname, nsl, [],
    [AC_MSG_WARN([you don't have gethostbyname()])
     dumb_network=""])])
DUMB_NETWORK_LDADD="$LIBS"
LIBS="$dumb_network_old_LIBS"
AC_MSG_CHECKING(which network driver to use)
if test -z "$dumb_network"; then
  dumb_network="dummy"
  DUMB_NETWORK_LDADD=""
  AC_MSG_WARN([you won't be able to play network games])
fi
DUMB_NETWORK_SOURCES="${dumb_network}_net.c"
AC_SUBST(DUMB_NETWORK_SOURCES)
AC_SUBST(DUMB_NETWORK_LDADD)
AC_MSG_RESULT($dumb_network)
if test "$dumb_network" = unix; then
  # We'll manage even without select() but use it if it exists.
  AC_CHECK_FUNCS(select)
  # Only unix_net.c uses hstrerror(), and the replacement needs <netdb.h>.
  AC_REPLACE_FUNCS(hstrerror)
fi

dnl --- Data files.
dnl Building the WADs runs ptcomp and wadtool, which can't run on the
dnl build machine when cross-compiling.  The WADs are not
dnl platform-specific, so take them from a native build instead.
AM_CONDITIONAL([DUMB_BUILD_WADS], [test "$cross_compiling" != yes])
if test "$cross_compiling" = yes; then
  AC_MSG_WARN([cross-compiling: doom4dum.wad and htic4dum.wad will not be built])
fi

dnl --- Output.
AH_BOTTOM([/* With NLS, <libintl.h> comes from the C library or libintl.  */
#ifdef ENABLE_NLS
#define HAVE_LIBINTL_H 1
#endif])
AC_CONFIG_FILES([Makefile libmissing/Makefile libdumbutil/Makefile
  libdumbwad/Makefile libdumbworldb/Makefile libdumbrender/Makefile
  libdumb/Makefile dumb/Makefile xwad/Makefile ptcomp/Makefile
  tool/Makefile doom/Makefile htic/Makefile test/Makefile
  docs/Makefile docs/dos/Makefile docs/maint/Makefile
  docs/patches/Makefile po/Makefile:po/Makefile.in.in scripts/Makefile])
AC_OUTPUT

dnl Local Variables:
dnl comment-start: "dnl "
dnl comment-end: ""
dnl comment-start-skip: "\\bdnl\\b\\s *"
dnl info-lookup-mode: autoconf-mode
dnl End:
