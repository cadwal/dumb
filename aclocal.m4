dnl aclocal.m4 generated automatically by aclocal 1.3

dnl Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
dnl This Makefile.in is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

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
 

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AM_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION"))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])


# serial 1

AC_DEFUN(AM_PROG_INSTALL,
[AC_REQUIRE([AC_PROG_INSTALL])
test -z "$INSTALL_SCRIPT" && INSTALL_SCRIPT='${INSTALL_PROGRAM}'
AC_SUBST(INSTALL_SCRIPT)dnl
])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Define a conditional.

AC_DEFUN(AM_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

