dnl acinclude.m4 -- this file is included in aclocal.m4 by aclocal

dnl Usage:
dnl DUMB_ARG_MULTICHOICE(class, vartail, switchname, help)
dnl
AC_DEFUN(DUMB_ARG_MULTICHOICE,
[AC_ARG_WITH($3, [$4],
[if test "$withval" = "no"; then
  dumb_$1_$2="no"
else
  if test -n "$dumb_$1_specified"; then
    AC_MSG_WARN([--with-$dumb_$1_specified overridden by --with-$3])
    dumb_$1_$dumb_$1_specified="no"
  fi
  dumb_$1_$2="yes"
  dumb_$1_specified="$2"
fi])])

dnl Usage:
dnl DUMB_TEST_MULTICHOICE(testvar, class, vartail, checkingname,
dnl                       forceaction, testaction)
dnl
AC_DEFUN(DUMB_TEST_MULTICHOICE,
[if test -z "$$1"; then
  case $dumb_$2_$3 in
    [no)]
      AC_MSG_CHECKING(for $4)
      AC_MSG_RESULT(disabled);;
    [yes)]
      AC_MSG_CHECKING(for $4)
      AC_MSG_RESULT((forced) yes)
      $5;;
    [autodetect)]
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
AC_DEFUN(DUMB_SYS_SHMAT_RMID,
[AC_CACHE_CHECK(whether shmctl IPC_RMID allows subsequent attaches,
dumb_cv_sys_shmat_rmid,
[AC_TRY_RUN(
[#include <sys/types.h>
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
],
dumb_cv_sys_shmat_rmid=yes,
dumb_cv_sys_shmat_rmid=no,
dumb_cv_sys_shmat_rmid=no)])])

dnl Usage:
dnl DUMB_ARG_HELP(text)
dnl
dnl This I copied from Autoconf 2.12's acgeneral.m4 where it was buried
dnl inside AC_ARG_ENABLE.  Perhaps a newer version of Autoconf already
dnl has this as a separate macro.
dnl
AC_DEFUN(DUMB_ARG_HELP,
[AC_DIVERT_PUSH(AC_DIVERSION_NOTICE)dnl
ac_help="$ac_help
[$1]"
AC_DIVERT_POP()])

dnl Usage:
dnl DUMB_MSG_REQUIRE(testcmd, errmsg_if_false)
dnl
AC_DEFUN(DUMB_MSG_REQUIRE,
[[$1] || AC_MSG_ERROR([$2])])

dnl Usage:
dnl DUMB_MSG_REQUIRE_YES(cachevariable, required_name)
dnl
AC_DEFUN(DUMB_MSG_REQUIRE_YES,
[DUMB_MSG_REQUIRE([test "$$1" = "yes"], [DUMB currently requires $2])])

dnl Usage:
dnl DUMB_MSG_REQUIRE_NO(cachevariable, required_name)
dnl
AC_DEFUN(DUMB_MSG_REQUIRE_NO,
[DUMB_MSG_REQUIRE([test "$$1" = "no"], [DUMB can't currently cope with $2])])

dnl Usage:
dnl DUMB_PROG_CXX_C
dnl
dnl Cache variable: dumb_cv_prog_cxx_c
dnl
AC_DEFUN(DUMB_PROG_CXX_C,
[AC_CACHE_CHECK([how to compile .c files as C++], [dumb_cv_prog_cxx_c],
[AC_REQUIRE([AC_PROG_CXX])
if test "$GXX" = "yes"; then
  dumb_cv_prog_cxx_c="$CXX -x c++ -fenum-int-equiv"
else
  AC_MSG_WARN([you aren't using the GNU C++ compiler])
  dumb_cv_prog_cxx_c="$CXX"
fi])])

dnl Usage:
dnl DUMB_CHECK_HDR_LIB(headerfile, lib, libfunction, otherlibs,
dnl                    yesaction, noaction)
dnl
AC_DEFUN(DUMB_CHECK_HDR_LIB,
 [AC_CHECK_HEADER([$1],
   [AC_CHECK_LIB([$2], [$3], [$5], [$6], [$4])],
   [$6])])

dnl Usage:
dnl DUMB_CHECK_HDR_LIB_YESNO(headerfile, lib, libfunction, otherlibs,
dnl                          yesnovar)
dnl
AC_DEFUN(DUMB_CHECK_HDR_LIB_YESNO,
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
AC_DEFUN(DUMB_SEARCH_LIBSETS,
[AC_CACHE_CHECK([for library containing $1], [dumb_cv_search_$1],
[dumb_func_search_save_LIBS="$LIBS"
dumb_cv_search_$1="no"
for i in $2; do
LIBS="$i $dumb_func_search_save_LIBS"
AC_TRY_LINK_FUNC([$1],
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
AC_DEFUN(DUMB_C_ATTR_REGPARM,
  [AC_MSG_CHECKING(how to pass parameters in registers)
  AC_CACHE_VAL(dumb_cv_c_attr_regparm,
    [AC_TRY_RUN(
[int func(int a, int b, int c, int d) __attribute__((regparm(3)));
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
}],
      [dumb_cv_c_attr_regparm="__attribute__((regparm(3)))"],
      [dumb_cv_c_attr_regparm=""],
      [dumb_cv_c_attr_regparm=""])])
  if test -z "$dumb_cv_c_attr_regparm"; then
    AC_MSG_RESULT(no)
  else
    AC_MSG_RESULT($dumb_cv_c_attr_regparm)
  fi
  AC_DEFINE_UNQUOTED(ATTR_REGPARM, $dumb_cv_c_attr_regparm)])

dnl Usage:
dnl DUMB_CHECK_IFDEF(headerfile, symbol, yesaction, noaction)
dnl HEADERFILE and SYMBOL must be literals.
dnl
dnl Cache variable: dumb_cv_ifdef_$1_$2
dnl with some transliteration.
dnl
AC_DEFUN(DUMB_CHECK_IFDEF,
  [pushdef([DUMB_IFDEF_VAR],
    [dumb_cv_ifdef_]translit([$1_$2], [./+-], [__p_]))
  AC_CACHE_CHECK([for $2 in <$1>],
    DUMB_IFDEF_VAR,
    [AC_TRY_CPP(
[#include <$1>
#ifndef $2
#error $2 was not defined in <$1>
#endif
],
    [DUMB_IFDEF_VAR=yes],
    [DUMB_IFDEF_VAR=no])])
  if test $DUMB_IFDEF_VAR = yes; then
    ifelse([$3], [], [:], [$3])
  ifelse([$4], [], [], [else
    $4])
  fi
  popdef([DUMB_IFDEF_VAR])])

dnl Usage:
dnl DUMB_C_RESTRICT
dnl
dnl Like AC_C_CONST, but for the C9X `restrict' keyword.
dnl
AC_DEFUN(DUMB_C_RESTRICT,
  [AC_CACHE_CHECK([for restrict], dumb_cv_c_restrict,
    [AC_TRY_COMPILE([],
      [int *restrict a;],
      dumb_cv_c_restrict=yes, dumb_cv_c_restrict=no)])
  if test $dumb_cv_c_restrict = no; then
    AC_DEFINE(restrict, [], [Define to empty if the keyword does not work. ])
  fi])

dnl Usage:
dnl DUMB_CHECK_TYPE(TYPE, DEFAULT [, INCLUDES])
dnl
dnl Mostly copied from AC_CHECK_TYPE (which is Copyright FSF), but
dnl allows a third argument INCLUDES.  If present, it is used instead
dnl of including <sys/types.h> etc.
dnl
AC_DEFUN(DUMB_CHECK_TYPE,
[AC_REQUIRE([AC_HEADER_STDC])dnl
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(dumb_cv_type_$1,
[AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<(^|[^a-zA-Z_0-9])$1[^a-zA-Z_0-9]>>dnl
changequote([,]), ifelse([$#], [3], [[$3]],
[[#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif]]), [dumb_cv_type_$1=yes], [dumb_cv_type_$1=no])])dnl
AC_MSG_RESULT([$dumb_cv_type_$1])
if test $dumb_cv_type_$1 = no; then
  AC_DEFINE($1, $2)
fi])
