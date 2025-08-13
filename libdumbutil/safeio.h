
#ifndef SAFEIO_H
#define SAFEIO_H

/* you should *always* include limits.h before safeio.h */
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* these mmap fns should only be used for *readonly* mmaps */
const void *safe_mmap(const char *name,int fd,unsigned int offset,size_t len);
void safe_munmap(const char *name,const void *ptr,size_t len);

void safe_read(const char *name,int fd,void *buf,size_t len);
int safe_open(const char *fname,int omode,int fail_lvl);
void safe_close(const char *name,int fd);

#endif
