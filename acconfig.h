/* Define to the name of the distribution.  */
#undef PACKAGE

/* Define to the version of the distribution.  */
#undef VERSION

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

/* Define as the directory to load locale data from.  */
#undef LOCALEDIR

/* Define as a function attribute to pass parameters in registers.
 * Define as empty if register parameters aren't supported.
 * This macro is used in the prototype like this:
 *
 * int foo(int) ATTR_REGPARM;
 *
 * The macro is not used again in the actual definition of the
 * function.  */
#define ATTR_REGPARM

/* Define if you have the XFree86 DGA extension.  This requires
 * <X11/extensions/xf86dga.h> and -lXxf86dga.  */
#undef DUMB_CONFIG_XF86DGA

/* Define if you have the MIT Shared Memory extension.  This requires
 * <X11/extensions/XShm.h> and support for it in -lXext.  */
#undef DUMB_CONFIG_XSHM

/* Define if you have the X Keyboard Extension.  This requires
 * <X11/XKBlib.h> and support for it in -lX11.  */
#undef DUMB_CONFIG_XKB

/* Define to support framebuffers with 8 bits per pixel.  */
#undef DUMB_CONFIG_8BPP

/* Define to support framebuffers with 16 bits per pixel.  */
#undef DUMB_CONFIG_16BPP

/* Define to support framebuffers with 32 bits per pixel.  */
#undef DUMB_CONFIG_32BPP

/* Define if shmat() can attach to shared memory segments already
 * marked for deletion with shmctl(id, IPC_RMID, NULL).  */
#undef DUMB_CONFIG_SYS_SHMAT_RMID

/* A standard place to look for Doom .WADs */
#undef DUMB_CONFIG_DOOM_PATH

/* A standard place to look for HERETIC .WADs */
#undef DUMB_CONFIG_HERETIC_PATH

/* Define to enable libdumbworldb */
#undef DUMB_CONFIG_LDWB

@BOTTOM@

/* DUMB always has <libintl.h> -- the included GNU gettext library
 * provides it if the system doesn't.  */
#ifdef ENABLE_NLS
#define HAVE_LIBINTL_H 1
#endif
