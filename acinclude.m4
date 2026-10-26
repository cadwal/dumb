dnl acinclude.m4 -- this file is included in aclocal.m4 by aclocal

dnl Usage:
dnl DUMB_ARG_MULTICHOICE(class, vartail, switchname, help)
dnl
AC_DEFUN([DUMB_ARG_MULTICHOICE],
[AC_ARG_WITH([$3], [$4],
[if test "$withval" = "no"; then
  dumb_$1_$2="no"
else
  if test -n "$dumb_$1_specified"; then
    AC_MSG_WARN([--with-$dumb_$1_specified overridden by --with-$3])
    eval "dumb_$1_$dumb_$1_specified=no"
  fi
  dumb_$1_$2="yes"
  dumb_$1_specified="$2"
fi])])

dnl Usage:
dnl DUMB_TEST_MULTICHOICE(testvar, class, vartail, checkingname,
dnl                       forceaction, testaction)
dnl
AC_DEFUN([DUMB_TEST_MULTICHOICE],
[if test -z "$$1"; then
  case $dumb_$2_$3 in
    no)
      AC_MSG_CHECKING([for $4])
      AC_MSG_RESULT([disabled]);;
    yes)
      AC_MSG_CHECKING([for $4])
      AC_MSG_RESULT([(forced) yes])
      $5;;
    autodetect)
      if test -z "$dumb_$2_specified"; then
        $6
      fi;;
  esac
fi])

dnl Usage:
dnl DUMB_SYS_SHMAT_RMID
dnl
dnl Cache variable: dumb_cv_sys_shmat_rmid
dnl
dnl I copied this test from gtk+-0.99.4/configure.in and made it a macro.
dnl
AC_DEFUN([DUMB_SYS_SHMAT_RMID],
[AC_CACHE_CHECK([whether shmctl IPC_RMID allows subsequent attaches],
[dumb_cv_sys_shmat_rmid],
[AC_RUN_IFELSE([AC_LANG_SOURCE(
[[#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
int main()
{
  int id;
  char *shmaddr, *testaddr;
  id = shmget(IPC_PRIVATE, 4, IPC_CREAT | 0777);
  if (id == -1)
  exit(2);
  shmaddr = shmat(id, 0, 0);
  shmctl(id, IPC_RMID, 0);
  testaddr = shmat(id, 0, 0);
  if (testaddr == (char *) -1) {
    shmdt(shmaddr);
    exit(1);
  }
  shmdt(shmaddr);
  shmdt(testaddr);
  exit(0);
}
]])],
[dumb_cv_sys_shmat_rmid=yes],
[dumb_cv_sys_shmat_rmid=no],
[dumb_cv_sys_shmat_rmid=no])])])

dnl Usage:
dnl DUMB_ARG_HELP(text)
dnl
dnl Add a line of text to the --with-* section of configure --help.
dnl
AC_DEFUN([DUMB_ARG_HELP],
[m4_divert_text([HELP_WITH], [$1])])

dnl Usage:
dnl DUMB_MSG_REQUIRE(testcmd, errmsg_if_false)
dnl
AC_DEFUN([DUMB_MSG_REQUIRE],
[$1 || AC_MSG_ERROR([$2])])

dnl Usage:
dnl DUMB_MSG_REQUIRE_YES(cachevariable, required_name)
dnl
AC_DEFUN([DUMB_MSG_REQUIRE_YES],
[DUMB_MSG_REQUIRE([test "$$1" = "yes"], [DUMB currently requires $2])])

dnl Usage:
dnl DUMB_MSG_REQUIRE_NO(cachevariable, required_name)
dnl
AC_DEFUN([DUMB_MSG_REQUIRE_NO],
[DUMB_MSG_REQUIRE([test "$$1" = "no"], [DUMB can't currently cope with $2])])

dnl Usage:
dnl DUMB_PROG_CXX_C
dnl
dnl Cache variable: dumb_cv_prog_cxx_c
dnl
AC_DEFUN([DUMB_PROG_CXX_C],
[AC_REQUIRE([AC_PROG_CXX])
AC_CACHE_CHECK([how to compile .c files as C++], [dumb_cv_prog_cxx_c],
[if test "$GXX" = "yes"; then
  dumb_cv_prog_cxx_c="$CXX -x c++"
else
  AC_MSG_WARN([you aren't using the GNU C++ compiler])
  dumb_cv_prog_cxx_c="$CXX"
fi])])

dnl Usage:
dnl DUMB_CHECK_HDR_LIB(headerfile, lib, libfunction, otherlibs,
dnl                    yesaction, noaction)
dnl
AC_DEFUN([DUMB_CHECK_HDR_LIB],
 [AC_CHECK_HEADER([$1],
   [AC_CHECK_LIB([$2], [$3], [$5], [$6], [$4])],
   [$6])])

dnl Usage:
dnl DUMB_CHECK_HDR_LIB_YESNO(headerfile, lib, libfunction, otherlibs,
dnl                          yesnovar)
dnl
AC_DEFUN([DUMB_CHECK_HDR_LIB_YESNO],
 [DUMB_CHECK_HDR_LIB([$1], [$2], [$3], [$4], [$5=yes], [$5=no])])

dnl Usage:
dnl DUMB_SEARCH_LIBSETS(FUNCTION, SEARCH-LIBSETS [, ACTION-IF-FOUND
dnl                     [, ACTION-IF-NOT-FOUND]])
dnl
dnl Most of this is copied from AC_SEARCH_LIBS of Autoconf 2.13, except:
dnl 1. SEARCH-LIBSETS entries must contain "-l".
dnl 2. Each entry in SEARCH-LIBSETS can name several libraries if quoted.
dnl 3. This does not automatically try linking without special libraries,
dnl    but you can do that by adding "" in SEARCH-LIBSETS.
dnl 4. There is no OTHER-LIBRARIES argument.
dnl 5. The cache variable is different.
dnl
AC_DEFUN([DUMB_SEARCH_LIBSETS],
[AC_CACHE_CHECK([for library containing $1], [dumb_cv_search_$1],
[dumb_func_search_save_LIBS="$LIBS"
dumb_cv_search_$1="no"
for i in $2; do
LIBS="$i $dumb_func_search_save_LIBS"
AC_LINK_IFELSE([AC_LANG_CALL([], [$1])],
[dumb_cv_search_$1="$i"
break])
done
LIBS="$dumb_func_search_save_LIBS"
test "$dumb_cv_search_$1" = "" && dumb_cv_search_$1="none required"])
if test "$dumb_cv_search_$1" != "no"; then
  test "$dumb_cv_search_$1" = "none required" || LIBS="$dumb_cv_search_$1 $LIBS"
  $3
else :
  $4
fi])

dnl Usage:
dnl DUMB_C_ATTR_REGPARM
dnl
dnl Check if __attribute__((regparm(3))) compiles to working code.
dnl If it does, define ATTR_REGPARM as that.
dnl If not, define ATTR_REGPARM as empty.
dnl When cross compiling, assume it doesn't work.
dnl
dnl At least Debian GNU/Linux libc 5.4.33-3 has a bug in mcheck() that
dnl causes the attribute to garble parameters when profiling (gcc -p).
dnl This bug is detected and ATTR_REGPARM defined as empty.
dnl
dnl Cache variable: dumb_cv_c_attr_regparm
dnl
AC_DEFUN([DUMB_C_ATTR_REGPARM],
  [AC_MSG_CHECKING([how to pass parameters in registers])
  AC_CACHE_VAL([dumb_cv_c_attr_regparm],
    [AC_RUN_IFELSE([AC_LANG_SOURCE(
[[#include <stdlib.h>
int func(int a, int b, int c, int d) __attribute__((regparm(3)));
int main() {
  if (func(123, 456, 789, 1998) == 123+456+789+1998)
    exit(0);
  else
    exit(1);
}
/* GCC 2.7.2.1 docs say that a call that precedes the function's definition
 * can't be inlined.  Good.  */
int func(int a, int b, int c, int d)
{
  return a+b+c+d;
}]])],
      [dumb_cv_c_attr_regparm="__attribute__((regparm(3)))"],
      [dumb_cv_c_attr_regparm=""],
      [dumb_cv_c_attr_regparm=""])])
  if test -z "$dumb_cv_c_attr_regparm"; then
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([$dumb_cv_c_attr_regparm])
  fi
  AC_DEFINE_UNQUOTED([ATTR_REGPARM], [$dumb_cv_c_attr_regparm],
[Define as a function attribute to pass parameters in registers.
   Define as empty if register parameters aren't supported.
   The macro is used in prototypes and in function definitions.])])

dnl Usage:
dnl DUMB_CHECK_IFDEF(headerfile, symbol, yesaction, noaction)
dnl HEADERFILE and SYMBOL must be literals.
dnl
dnl Cache variable: dumb_cv_ifdef_$1_$2
dnl with some transliteration.
dnl
AC_DEFUN([DUMB_CHECK_IFDEF],
  [pushdef([DUMB_IFDEF_VAR],
    [dumb_cv_ifdef_]translit([$1_$2], [./+-], [__p_]))
  AC_CACHE_CHECK([for $2 in <$1>],
    DUMB_IFDEF_VAR,
    [AC_PREPROC_IFELSE([AC_LANG_SOURCE(
[[#include <$1>
#ifndef $2
#error $2 was not defined in <$1>
#endif
]])],
    [DUMB_IFDEF_VAR=yes],
    [DUMB_IFDEF_VAR=no])])
  if test $DUMB_IFDEF_VAR = yes; then
    ifelse([$3], [], [:], [$3])
  ifelse([$4], [], [], [else
    $4])
  fi
  popdef([DUMB_IFDEF_VAR])])

dnl Usage:
dnl DUMB_CHECK_TYPE(TYPE, DEFAULT, INCLUDES)
dnl
dnl If TYPE is not declared after INCLUDES, define TYPE as DEFAULT.
dnl
AC_DEFUN([DUMB_CHECK_TYPE],
[AC_CHECK_TYPE([$1], [],
  [AC_DEFINE([$1], [$2],
    [Define as the type of $1 if the system headers don't define it.])],
  [$3])])

dnl Usage:
dnl DUMB_NLS
dnl
dnl Native Language Support using the C library's gettext or an
dnl installed libintl.  Defines ENABLE_NLS and substitutes USE_NLS,
dnl INTLLIBS, MSGFMT, GMSGFMT and XGETTEXT for po/Makefile.
dnl
AC_DEFUN([DUMB_NLS],
[AC_ARG_ENABLE([nls],
  [AS_HELP_STRING([--disable-nls], [do not use Native Language Support])],
  [USE_NLS=$enableval], [USE_NLS=yes])
INTLLIBS=""
if test "$USE_NLS" = "yes"; then
  AC_CHECK_HEADER([libintl.h], [], [USE_NLS=no])
fi
if test "$USE_NLS" = "yes"; then
  dumb_nls_save_LIBS="$LIBS"
  LIBS=""
  AC_SEARCH_LIBS([bindtextdomain], [intl], [INTLLIBS="$LIBS"], [USE_NLS=no])
  LIBS="$dumb_nls_save_LIBS"
fi
AC_MSG_CHECKING([whether to use NLS])
AC_MSG_RESULT([$USE_NLS])
if test "$USE_NLS" = "yes"; then
  AC_DEFINE([ENABLE_NLS], [1], [Define to 1 if NLS is requested.])
fi
AC_PATH_PROG([MSGFMT], [msgfmt], [:])
AC_PATH_PROG([GMSGFMT], [gmsgfmt], [$MSGFMT])
AC_PATH_PROG([XGETTEXT], [xgettext], [:])
AC_SUBST([USE_NLS])
AC_SUBST([INTLLIBS])])

dnl Usage:
dnl DUMB_DEFINE_UNQUOTED(VARIABLE [, VALUE])
dnl
dnl Exactly like AC_DEFINE_UNQUOTED, but won't be caught by `autoheader'.
dnl
m4_define([DUMB_DEFINE_UNQUOTED],
[cat >> confdefs.h <<_DUMBEOF
[#define] $1 ifelse($#, 2, [$2], $#, 3, [$2], 1)
_DUMBEOF
])
