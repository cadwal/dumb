#ifndef SAFEIO_H
#define SAFEIO_H

#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* these mmap fns should only be used for *readonly* mmaps */
const void *safe_mmap(const char *name,int fd,unsigned int offset,size_t len);
void safe_munmap(const char *name,const void *ptr,size_t len);

void safe_read(const char *name,int fd,void *buf,size_t len);
int safe_open(const char *fname,int omode,int fail_lvl);
void safe_close(const char *name,int fd);

/* Like safe_open() except looks in all directories in the
 * NULL-terminated array PATH.  If REALNAME is non-null, saves the
 * real filename in *REALNAME.  The caller is responsible for freeing
 * the name with free() when it's no longer needed.  If the file isn't
 * found and fail_lvl!=LOG_FATAL, returns -1 without changing
 * *REALNAME.  If FNAME begins with a slash ('/'), doesn't use PATH at all.
 *
 * This won't work sensibly with O_CREAT.  */
int safe_open_path(const char *fname, int omode, int fail_lvl,
		   const char *const path[], char **realname);

#endif
