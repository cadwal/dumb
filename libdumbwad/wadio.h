
#ifndef WADIO_H
#define WADIO_H

void reset_wad(void);
void init_iwad(const char *fname, const char *const *path);
void init_pwad(const char *fname, const char *const *path);

void init_wadhashing(void);
void reset_wadhashing(void);

typedef unsigned int LumpNum;

#define LUMP_WADNUM(x) (((LumpNum)x)>>16)
#define LUMP_DIRNUM(x) (((LumpNum)x)&0xffff)
#define LUMPNUM(w,d) ( (((LumpNum)w)<<16) | (d&0xffff) )

#define BAD_LUMPNUM ((LumpNum)0xffffffff)
#define LUMPNUM_OK(l) (l!=BAD_LUMPNUM)

LumpNum lumpnext(LumpNum l,int crosswad);
LumpNum lumplook(LumpNum l,const char *name);
int count_lumps_between(const char *after,const char *before);

LumpNum safe_lookup_lump(const char *name,const char *after,const char *before,int lvl);
#define lookup_lump(x,y,z) safe_lookup_lump(x,y,z,-1)
#define getlump(n) safe_lookup_lump(n,NULL,NULL,LOG_FATAL)
#define have_lump(n) LUMPNUM_OK(lookup_lump(n,NULL,NULL))

int get_lump_fd(LumpNum ln);
unsigned int get_lump_ofs(LumpNum ln);
unsigned int get_lump_len(LumpNum ln);
const void *get_lump_map(LumpNum ln); 
char *get_lump_name(LumpNum ln,char *buf); 

int get_num_wads(void);
int get_num_lumps(unsigned int wadnum);

const void *load_lump(LumpNum ln);
void *copy_lump(LumpNum ln);
void release_lump(LumpNum ln);
void free_lump(LumpNum ln);
/* free_lump is for when we don't think we'll be wanting this one again */
void free_all_lumps(void);
void free_dead_lumps(void);

#endif
