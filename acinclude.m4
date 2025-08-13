dnl acinclude.m4 -- this file is included in aclocal.m4 by aclocal

dnl Usage:
dnl DUMB_ARG_MULTICHOICE(class, vartail, switchname, help)

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
dnl I copied this test from the gtk+-0.99.4/configure.in and made it a macro.

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

AC_DEFUN(DUMB_ARG_HELP,
[AC_DIVERT_PUSH(AC_DIVERSION_NOTICE)dnl
ac_help="$ac_help
[$1]"
AC_DIVERT_POP()])

dnl Usage:
dnl DUMB_MSG_REQUIRE(testcmd, errmsg_if_false)

AC_DEFUN(DUMB_MSG_REQUIRE,
[[$1] || AC_MSG_ERROR([$2])])

dnl Usage:
dnl DUMB_MSG_REQUIRE_YES(cachevariable, required_name)

AC_DEFUN(DUMB_MSG_REQUIRE_YES,
[DUMB_MSG_REQUIRE([test "$$1" = "yes"], [DUMB currently requires $2])])

dnl Usage:
dnl DUMB_MSG_REQUIRE_NO(cachevariable, required_name)

AC_DEFUN(DUMB_MSG_REQUIRE_NO,
[DUMB_MSG_REQUIRE([test "$$1" = "no"], [DUMB can't currently cope with $2])])

dnl Usage:
dnl DUMB_PROG_CXX_C
dnl
dnl Cache variable: dumb_cv_prog_cxx_c

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
AC_DEFUN(DUMB_CHECK_HDR_LIB,
 [AC_CHECK_HEADER([$1],
   [AC_CHECK_LIB([$2], [$3], [$5], [$6], [$4])],
   [$6])])

dnl Usage:
dnl DUMB_CHECK_HDR_LIB_YESNO(headerfile, lib, libfunction, otherlibs,
dnl                          yesnovar)
AC_DEFUN(DUMB_CHECK_HDR_LIB_YESNO,
 [DUMB_CHECK_HDR_LIB([$1], [$2], [$3], [$4], [$5=yes], [$5=no])])

dnl Usage:
dnl DUMB_SCAN_LIBS(function, scanlist, foundaction, notfoundaction)
dnl
dnl When FOUNDACTION is taken, $library contains the name
dnl of the library where the function was found, prepended with -l.  
dnl Or $library may be empty if no extra libraries are needed.
AC_DEFUN(DUMB_SCAN_LIBS,
 [library=""
  AC_CHECK_FUNC([$1],
   [$3],
   [for dumb_scan_libs_name in [$2]; do
      AC_CHECK_LIB($dumb_scan_libs_name, [$1],
       [library="-l$dumb_scan_libs_name"
        break])
    done;
    if test -n "$library"; then
      $3
    else
      $4
    fi])])
 
