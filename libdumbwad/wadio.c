#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <limits.h>

#include "libdumbutil/safeio.h"
#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "wadstruct.h"
#include "wadio.h"


/* this enables hashing for lump names (fast!) */
#define USE_HASHING

/* tell us how the hashing is getting on */
#define HASHING_REPORT

#define HASHSIZE 401 /* a nice big prime is best */

/* 
   this just disables some code that tried to cope with bigendian processors 
   by recognising bigendian wads.  Now we just convert the wads internally.
*/
#define NO_BYTEORDER_TRICKS

#define WDEBUG(x) logprintf(LOG_DEBUG,'W',x)

#ifndef __MSDOS__
#define O_BINARY 0
#endif

#ifdef NO_MMAP
#define DO_BIG_MMAP 0
#else
#ifdef NO_BIG_MMAP
#define DO_BIG_MMAP 0
#else
#define DO_BIG_MMAP 1
#endif
#endif

typedef struct _WadFile {
   struct _WadFile *next;
   int fd;
   unsigned int nlumps;
   const WadDirEntry *dir;
   const void *whole_map;
   const void *mapalloc;
   size_t maplen,wadlen;
   char fname[PATH_MAX];
} WadFile;

#define WHOLEMAP_PTR(wad,off) ((const void *) ((const char *)(wad->whole_map)+off) )

static int nwads=0;
static WadFile *wads=NULL;

#ifdef USE_HASHING
typedef struct _HashEnt {
   struct _HashEnt *next;
   LumpNum ln;
} HashEnt;

static HashEnt **hashtbl=NULL;

static int hashfunc(const char *s) {
   const char *t=s+8;
   const char *garble="garble1^";
   unsigned int i=0;
   while(*s&&s<t) i+=*(garble++)*toupper(*s++);
   return i%HASHSIZE;
}

void reset_wadhashing(void) {
   int i;
   if(hashtbl==NULL) return;
   for(i=0;i<HASHSIZE;i++) {
      HashEnt *h=hashtbl[i];
      while(h) {HashEnt *v=h; h=h->next; safe_free(v);}
   }
   safe_free(hashtbl);
   hashtbl=NULL;
}

static void add_hashent(const char *name,LumpNum ln) {
   HashEnt **h=hashtbl+hashfunc(name);
   while(*h) h=&((*h)->next);
   *h=safe_calloc(1,sizeof(HashEnt));
   (*h)->ln=ln;
}

static LumpNum lumphash(LumpNum after,const char *name) {
   char buf[10];
   HashEnt *h;
   h=hashtbl[hashfunc(name)];
   buf[8]=0;
   while(h) {
      if(h->ln>=after&&!strcasecmp(name,get_lump_name(h->ln,buf))) 
	 return h->ln;
      h=h->next;
   }
   return BAD_LUMPNUM;
}

void init_wadhashing(void) {
   WadFile *w=wads;
   int wadnum=0,i;
#ifdef HASHING_REPORT
   int unused,maxuse=0;
#endif
   if(hashtbl) reset_wadhashing();
   /*logprintf(LOG_DEBUG,'W',"building hashtable...");*/
   hashtbl=safe_calloc(HASHSIZE,sizeof(HashEnt *));
   /* now build table */
   while(w) {
      for(i=0;i<w->nlumps;i++)
	 add_hashent(w->dir[i].name,LUMPNUM(wadnum,i));
      w=w->next;
      wadnum++;
   }
#ifdef HASHING_REPORT
   /* report some info */
   unused=0;
   for(i=0;i<HASHSIZE;i++) {
      int j;
      HashEnt *h;
      if(hashtbl[i]==NULL) unused++;
      for(j=0,h=hashtbl[i];h;j++,h=h->next);
      /*logprintf(LOG_DEBUG,'W',"hashval %d, chain length %d",i,j);*/
      if(j>maxuse) maxuse=j;
   }
   logprintf(LOG_INFO,'W',"wad hashtable: %d entries unused, max chain %d",
	     unused,maxuse);
#endif
}

#endif /* USE_HASHING */

int get_num_wads(void) {return nwads;}

void reset_wad(void) {
#ifdef USE_HASHING
   reset_wadhashing();
#endif
   free_all_lumps();
   while(wads!=NULL) {
      WadFile *wad=wads;
      wads=wad->next;
      if(wad->mapalloc) safe_munmap(wad->fname,wad->mapalloc,wad->maplen);
      if(wad->fd>=0) safe_close(wad->fname,wad->fd);
      safe_free(wad);
   }
   nwads=0;
}

static void check_sanity(const WadFile *wad,const char *fname) {
   int i;
   const WadDirEntry *wd=wad->dir;
   for(i=0;i<wad->nlumps;i++,wd++) {
      if(wd->name[0]==0) 
	 logprintf(LOG_ERROR,'W',"check_sanity(%s,%d): nameless lump",
		   fname,i);
      if(wd->size==0) continue; /* don't worry about extent check for markers */
      if(wd->offset<sizeof(WadHeader)||wd->offset+wd->size>wad->wadlen)
	 logprintf(LOG_FATAL,'W',"check_sanity(%s,%d): lump too big for wad",
		   fname,i);
   }
}

static void load_wad(const char *fname,const char *_sig,int mapall) {
   WadHeader whbuf;
   const WadHeader *wh=&whbuf;
   WadFile *wad=(WadFile*)safe_calloc(sizeof(WadFile),1);
#ifdef USE_HASHING
   reset_wadhashing();
#endif
   wad->next=wads;
   strncpy(wad->fname,fname,PATH_MAX);
   /* open the file */
   wad->fd=safe_open(fname,O_RDONLY|O_BINARY,LOG_FATAL);
   wad->wadlen=lseek(wad->fd,0,SEEK_END);
   lseek(wad->fd,0,SEEK_SET);
   if(wad->wadlen<sizeof(WadHeader)) 
      logfatal('W',"%s is too short to be a valid wadfile",fname); 
   /* now get the header.  how this is done will depend on our mm strategy */
   if(mapall) {
      wad->maplen=wad->wadlen;
      wh=wad->whole_map=wad->mapalloc=safe_mmap(fname,wad->fd,0,wad->maplen);
      logprintf(LOG_INFO,'W',"%s mapped at 0x%lx (%lu bytes)",
		fname,(unsigned long)wh,(unsigned long)wad->maplen); 
   }
   else
     safe_read(fname,wad->fd,&whbuf,sizeof(whbuf));
   /* deal to header */
   if(!memcmp(_sig,wh->sig,4));
   else
     logfatal('W',"Bad signature on %s: expecting %s, got %s",
	      fname,_sig,wh->sig);
   wad->nlumps=wh->nlumps;
   if(wh->diroffset+wad->nlumps*sizeof(WadDirEntry)>wad->wadlen)
      logfatal('W',"directory for %s would be beyond end of file",fname);
   /* say what we've done */
   logprintf(LOG_INFO,'W',"%s %s: %d lumps dir @ %d",
	     _sig,fname,(int)wad->nlumps,(int)wh->diroffset);
   /* load directory */
   if(wad->whole_map) 
     wad->dir=WHOLEMAP_PTR(wad,wh->diroffset);
   else 
     wad->dir=safe_mmap(fname,wad->fd,wh->diroffset,
			wh->nlumps*sizeof(WadDirEntry));
   wads=wad;
   nwads++;
   check_sanity(wad,fname);
}

void init_iwad(const char *fname) {
   reset_wad();
   load_wad(fname,"IWAD",DO_BIG_MMAP);
}
void init_pwad(const char *fname) {
   load_wad(fname,"PWAD",DO_BIG_MMAP);
}

static WadFile *wadnum2file(int wadnum) {
   WadFile *w=wads;
   while(wadnum>0&&w!=NULL) {
      w=w->next;
      wadnum--;
   }
   return w;
}

int get_num_lumps(unsigned int wadnum) {
   return wadnum2file(wadnum)->nlumps;
}

static int lumpnamecmp(const char *s1,const char *s2)  {
   int i;
   for(i=0;i<8;i++)  {
      if(s2[i]!='?'&&toupper(s1[i])!=toupper(s2[i])) return 1;
      if(s2[i]==0) break;
   }
   return 0;
}

LumpNum lumpnext(LumpNum l,int crosswad) {
   int wadnum=LUMP_WADNUM(l),dirnum=LUMP_DIRNUM(l);
   WadFile *wad;
   if(!LUMPNUM_OK(l)) return BAD_LUMPNUM;
   wad=wadnum2file(wadnum);
   dirnum++;
   if(dirnum>=wad->nlumps) {
      if(!crosswad) return BAD_LUMPNUM;
      wadnum++;
      wad=wad->next;
   }
   if(wad==NULL) return BAD_LUMPNUM;
   return LUMPNUM(wadnum,dirnum);
}

LumpNum lumplook(LumpNum l,const char *name) {
   int wadnum=LUMP_WADNUM(l),dirnum=LUMP_DIRNUM(l);
   WadFile *wad;
#ifdef USE_HASHING
   if(hashtbl&&!strchr(name,'?')) 
      return lumphash(l,name);
#endif   
   wad=wadnum2file(wadnum);
   while(wad!=NULL) {
      /*logprintf(LOG_DEBUG,'W',"lumplook(%s) wadnum=%d wad=0x%x",
		  name,wadnum,wad);*/
      while(dirnum<wad->nlumps) {
	 if(!lumpnamecmp(wad->dir[dirnum].name,name))
	   return LUMPNUM(wadnum,dirnum);
	 dirnum++;
      }
      wadnum++;
      dirnum=0;
      wad=wad->next;
   }
   return BAD_LUMPNUM;
}

LumpNum safe_lookup_lump(const char *name,const char *after,const char *before,
			 int lvl) 
{
   LumpNum l1=0,l2=0;
   if(name==NULL||*name==0) return BAD_LUMPNUM;
   if(after) {
      l2=l1=lumplook(0,after);
      if(!LUMPNUM_OK(l1)) {
	 logprintf(lvl,'W',"Can't find lump marker %s (lump %s)",after,name);
	 return BAD_LUMPNUM;
      }
   }
   l1=lumplook(l1,name);
   if(!LUMPNUM_OK(l1)) {
      if(after) logprintf(lvl,'W',"Can't find lump %s:%s",after,name);
      else logprintf(lvl,'W',"Can't find lump %s",name);
      return BAD_LUMPNUM;
   }
   if(before) {
      l2=lumplook(l2,before);
      if(!LUMPNUM_OK(l2)) 
	logprintf(lvl,'W',"Can't find end-marker %s",before);
      else if(l2<l1) 
	logprintf(lvl,'W',"End-marker %s occurs before lump %s",before,name); 
   }
   return l1;
}

int count_lumps_between(const char *n1,const char *n2) {
   LumpNum ln=0,l2=0;
   int tot=0;
   while(1) {
      ln=lumplook(l2,n1);
      if(!LUMPNUM_OK(ln)) break;
      l2=lumplook(ln,n2);
      if(!LUMPNUM_OK(l2)) break;
      if(LUMP_WADNUM(ln)!=LUMP_WADNUM(l2)) continue;
      tot+=(LUMP_DIRNUM(l2)-LUMP_DIRNUM(ln))-1;
   }
   return tot;
}

static WadFile *get_lump_wad(LumpNum ln) {
   WadFile *wad=wadnum2file(LUMP_WADNUM(ln));
   if(wad==NULL) 
     logprintf(LOG_FATAL,'W',"Bad lumpnum (%x) in get_lump_*",
	       (unsigned int)ln);
   return wad;
}
static const WadDirEntry *get_lump_dirent(LumpNum ln) {
   WadFile *wad=wadnum2file(LUMP_WADNUM(ln));
   if(wad==NULL||wad->nlumps<=LUMP_DIRNUM(ln)) 
     logprintf(LOG_FATAL,'W',"Bad lumpnum (%x) in get_lump_*",
	       (unsigned int)ln);
   return wad->dir+LUMP_DIRNUM(ln);
}

int get_lump_fd(LumpNum ln) {return get_lump_wad(ln)->fd;}
unsigned int get_lump_ofs(LumpNum ln) {return get_lump_dirent(ln)->offset;}
unsigned int get_lump_len(LumpNum ln) {return get_lump_dirent(ln)->size;}
char *get_lump_name(LumpNum ln,char *d) {
   strncpy(d,get_lump_dirent(ln)->name,8);
   d[8]=0;
   return d;
}
const void *get_lump_map(LumpNum ln) {
   WadFile *wad=get_lump_wad(ln);
   const WadDirEntry *dent=get_lump_dirent(ln);
   if(wad->whole_map) return WHOLEMAP_PTR(wad,dent->offset);
   else return NULL;
}

// Local Variables:
// c-basic-offset: 3
// End:
