/* The name of this package, as a string constant.  */
#undef PACKAGE

/* What version of the package this is.  */
#undef VERSION

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

/* Define to get stereo sound (instead of mono).  */
#undef DUMB_CONFIG_SOUND_STEREO

/* Define to get 16-bit sound (instead of 8-bit).  */
#undef DUMB_CONFIG_SOUND_16BIT

/* Define if shmat() can attach to shared memory segments already
 * marked for deletion with shmctl(id, IPC_RMID, NULL).  */
#undef DUMB_CONFIG_SYS_SHMAT_RMID
