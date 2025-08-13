
#ifndef CONF_H
#define CONF_H

typedef struct {
   const char *name;
   long value;
} ConfEnum;

typedef struct {
   const char *name,*menuname,*help;
   const ConfEnum *etype;
   int intval;
   char *strval,**listval;
   char shortname;
   unsigned char maxlen;
   char dirtlvl;
   char flags;
} ConfItem;

#define CONFE(n,mn,sn,h,df,e) {n,mn,h,e,df,NULL,NULL,sn,0,DIRT_NONE,CI_ENUM}
#define CONFI(n,mn,sn,h,df) {n,mn,h,NULL,df,NULL,NULL,sn,0,DIRT_NONE,CI_INT}
#define CONFS(n,mn,sn,h,df,l) {n,mn,h,NULL,0,df,NULL,sn,l,DIRT_NONE,CI_STR}
#define CONFL(n,mn,sn,h) {n,mn,h,NULL,0,NULL,NULL,sn,0,DIRT_NONE,CI_LIST}
#define CONFB(n,mn,sn,h) {n,mn,h,NULL,0,NULL,NULL,sn,0,DIRT_NONE,CI_BOOL}
#define CONFNB(n,mn,sn,h) {n,mn,h,NULL,1,NULL,NULL,sn,0,DIRT_NONE,CI_BOOL}
#define CONFNS(n,mn,sn,h) {n,mn,h,NULL,0,NULL,NULL,sn,0,DIRT_NONE,CI_BOOL|CI_NOSAVE}

typedef struct {
   ConfItem *items;
   const char *name,*desc;
} ConfModule;

#define DIRT_NONE 0  /* value is built-in default */ 
#define DIRT_ARGS 1  /* value was specified on the command line */
#define DIRT_FILE 2  /* value loaded from config file */
#define DIRT_MODF 3  /* value modified */

#define CI_LIST 0x01
#define CI_INT 0x02
#define CI_STR 0x04
#define CI_BOOL 0x08
#define CI_NOSAVE 0x10
#define CI_ENUM 0x20

char *find_conf_file(char *buf,const char *basename);

int load_conf(const ConfModule *conf,const char *fn);
int save_conf(const ConfModule *conf,const char *fn,int dirt);

void set_conf(ConfItem *ci,char *val,int dirt);

/* return non-zero if we ought to quit (eg. if args didn't make sense) */
int conf_args(const ConfModule *conf,int argc,char **argv);

int conf_greatest_dirtlevel(const ConfModule *conf);

#endif
