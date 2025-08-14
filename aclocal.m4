dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

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

dnl Usage:
dnl DUMB_DEFINE_UNQUOTED(VARIABLE [, VALUE])
dnl
dnl Exactly like AC_DEFINE_UNQUOTED, but won't be caught by `autoheader'.
dnl
define(DUMB_DEFINE_UNQUOTED,
[cat >> confdefs.h <<EOF
[#define] $1 ifelse($#, 2, [$2], $#, 3, [$2], 1)
EOF
])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
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


# serial 5

AC_DEFUN(AM_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)
      AC_MSG_CHECKING([whether included gettext is requested])
      AC_ARG_WITH(included-gettext,
        [  --with-included-gettext use the GNU gettext library included here],
        nls_cv_force_use_gnu_gettext=$withval,
        nls_cv_force_use_gnu_gettext=no)
      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   dnl TODO: rename gt_cv_func_gettext_libintl to
	   dnl gt_tmp_func_gettext_libintl or something, as the value
	   dnl in config.cache is never used.
	   dnl (ac_cv_func_gettext_libintl takes care of the caching.)
	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CHECK_LIB(intl, gettext,
		 gt_cv_func_gettext_libintl=yes INTLLIBS=-lintl,
		 gt_cv_func_gettext_libintl=no)],
	       gt_cv_func_gettext_libintl=no)
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		dnl Use $INTLLIBS when testing for dcgettext() and
		dnl _nl_msg_cat_cntr.  Otherwise, they wouldn't be found
		dnl if they were in -lintl.
		gt_tmp_oldlibs="$LIBS"
		LIBS="$INTLLIBS $LIBS"
		dnl The macros here don't care of whether dcgettext()
		dnl is defined but the program may.
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
		LIBS="$gt_tmp_oldlibs"
	      fi
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
	       if test "$GENCAT" != "no"; then
		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
		 if test "$GMSGFMT" = "no"; then
		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
		 fi
		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		 USE_INCLUDED_LIBINTL=yes
		 CATOBJEXT=.cat
		 INSTOBJEXT=.cat
		 DATADIRNAME=lib
		 INTLDEPS='$(top_builddir)/intl/libintl.a'
		 INTLLIBS=$INTLDEPS
		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
		 nls_cv_header_intl=intl/libintl.h
		 nls_cv_header_libgt=intl/libgettext.h
	       fi])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" = "yes"; then
        dnl Mark actions used to generate GNU NLS library.
        INTLOBJS="\$(GETTOBJS)"
        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
        AC_SUBST(MSGFMT)
	USE_INCLUDED_LIBINTL=yes
        CATOBJEXT=.gmo
        INSTOBJEXT=.mo
        DATADIRNAME=share
	INTLDEPS='$(top_builddir)/intl/libintl.a'
	INTLLIBS=$INTLDEPS
	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
        nls_cv_header_intl=intl/libintl.h
        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


    # If this is used in GNU gettext we have to set USE_NLS to `yes'
    # because some of the sources are only built for this goal.
    if test "$PACKAGE" = gettext; then
      USE_NLS=yes
      USE_INCLUDED_LIBINTL=yes
    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd memcpy munmap putenv setenv setlocale strchr \
strcasecmp strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for lang in ${LINGUAS=$ALL_LINGUAS}; do
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but $(top_srcdir).
   dnl Try to locate it.
   dnl Format @MKINSTALLDIRS@ so that it can be used in Makefiles of all
   dnl directories.  This may involve referring to make variables.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     dnl In Autoconf 2.12, $ac_aux_dir can be either absolute
     dnl (if $srcdir was absolute) or relative to the top build directory.
     if echo "$ac_aux_dir" | grep "^/" >/dev/null; then
       MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
     else
       MKINSTALLDIRS="\$(top_builddir)/$ac_aux_dir/mkinstalldirs"
     fi
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])



# serial 1

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])



# serial 1

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES)
    fi
  fi])


