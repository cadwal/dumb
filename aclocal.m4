dnl Usage: DUMB_ARG_MULTICHOICE(class, vartail, switchname, help)
AC_DEFUN(DUMB_ARG_MULTICHOICE,
  [AC_ARG_WITH($3, [$4],
    if test "$withval" = "no"; then
      dumb_$1_$2="no"
    else
      if test -n "$dumb_$1_specified"; then
        AC_MSG_WARN(--with-$dumb_$1_specified overridden by --with-$3)
        dumb_$1_$dumb_$1_specified="no"
      fi
      dumb_$1_$2="yes"
      dumb_$1_specified="$2"
    fi)])

dnl Usage: DUMB_TEST_MULTICHOICE(testvar, class, vartail, checkingname, 
dnl                              forceaction, testaction)
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

dnl Usage: DUMB_SYS_SHMAT_RMID
dnl Cache variable: dumb_cv_sys_shmat_rmid
dnl I copied this test from the gtk+-0.99.4/configure.in and made it a macro.
AC_DEFUN(DUMB_SYS_SHMAT_RMID,
  [AC_CACHE_CHECK(whether shmctl IPC_RMID allows subsequent attaches,
    dumb_cv_sys_shmat_rmid,
    [AC_TRY_RUN([
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
      ],
      dumb_cv_sys_shmat_rmid=yes,
      dumb_cv_sys_shmat_rmid=no,
      dumb_cv_sys_shmat_rmid=no)])])

dnl Usage: DUMB_ARG_HELP(text)
dnl This I copied from Autoconf's acgeneral.m4 where it was buried inside
dnl AC_ARG_ENABLE.  Perhaps a newer version of Autoconf already has this
dnl as a separate macro.
AC_DEFUN(DUMB_ARG_HELP,
[AC_DIVERT_PUSH(AC_DIVERSION_NOTICE)dnl
ac_help="$ac_help
[$1]"
AC_DIVERT_POP()])
