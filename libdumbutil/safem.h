#ifndef SAFEM_H
#define SAFEM_H

#include <stddef.h>		/* size_t */

/* all of these logfatal() on error */

void *safe_realloc(void *p,size_t s);
void *safe_calloc(size_t s1,size_t s2);
void *safe_malloc(size_t size);
void safe_free(void *ptr);
char *safe_strdup(const char *s);

#ifndef HAVE_MMAP

#define safe_vcalloc(s) safe_calloc(s,1)
#define safe_vmalloc(s) safe_malloc(s)
#define safe_vfree(p,s) safe_free(p)

#else  /* HAVE_MMAP */

/* 
   vcalloc gets zeroed memory by copy-on-write mmapping /dev/zero 
   "great if you can get it" for big arrays that we want initialised.
*/
void *safe_vcalloc(size_t size);
#define safe_vmalloc safe_vcalloc
void safe_vfree(void *ptr, size_t size);

#endif /* HAVE_MMAP */

#endif /* !SAFEM_H */
