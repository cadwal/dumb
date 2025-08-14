/* acconfig.h
   Process this file with autoheader to produce config.h.in,
   which config.status then translates to config.h.  */

/* Define if your locale.h file contains LC_MESSAGES.  */
#undef HAVE_LC_MESSAGES

/* Define to 1 if NLS is requested.  */
#undef ENABLE_NLS

/* Define as 1 if you have catgets and don't want to use GNU gettext.  */
#undef HAVE_CATGETS

/* Define as 1 if you have gettext and don't want to use GNU gettext.  */
#undef HAVE_GETTEXT

/* Define as 1 if you have the stpcpy function.  */
#undef HAVE_STPCPY

/* Define as the type of the value to which the last parameter of
   recvfrom() must point, if <sys/socket.h> doesn't define.  */
#undef socklen_t

/* Define as a function attribute to pass parameters in registers.
 * Define as empty if register parameters aren't supported.
 * This macro is used in the prototype like this:
 *
 * int foo(int) ATTR_REGPARM;
 *
 * The macro is not used again in the actual definition of the
 * function.  */
#define ATTR_REGPARM

/* Define so that
 *
 * #include DUMB_CONFIG_PPM_H
 *
 * includes <ppm.h>, <gr/ppm.h> or some such.
 * Needed by tool/dark2trans.c and tool/ppmtodumb.c.  */
#undef DUMB_CONFIG_PPM_H

/* Define if you have the XFree86 DGA extension.  This requires
   <X11/extensions/xf86dga.h> and -lXxf86dga.  */
#undef DUMB_CONFIG_XF86DGA

/* Define if you have the MIT Shared Memory extension.  This requires
   <X11/extensions/XShm.h> and support for it in -lXext.  */
#undef DUMB_CONFIG_XSHM

/* Define if you have the X Keyboard Extension.  This requires
   <X11/XKBlib.h> and support for it in -lX11.  */
#undef DUMB_CONFIG_XKB

/* Define if shmat() can attach to shared memory segments already
   marked for deletion with shmctl(id, IPC_RMID, NULL).  */
#undef DUMB_CONFIG_SYS_SHMAT_RMID

/* Define to enable libdumbworldb */
#undef DUMB_CONFIG_LDWB


/* These four should be moved to dircfg.h but I don't know how to do
   that with Autoconf.  */

/* Define as the directory to load locale data from.  */
#undef LOCALEDIR

/* A standard place to look for Doom .WADs.
   In GNUspeak, "path" means a search path; since this variable can
   name only one directory, it's _DIR.  */
#undef DUMB_CONFIG_DOOM_DIR

/* A standard place to look for HERETIC .WADs */
#undef DUMB_CONFIG_HERETIC_DIR

/* ${datadir}/dumb; used in default wadpath */
#undef DUMB_CONFIG_DUMBDATADIR

@BOTTOM@

/* DUMB always has <libintl.h> -- the included GNU gettext library
   provides it if the system doesn't.  */
#ifdef ENABLE_NLS
#define HAVE_LIBINTL_H 1
#endif
