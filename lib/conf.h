
#ifndef CONF_H
#define CONF_H

typedef struct {
   const char *name;
   long value;
} ConfEnum;

enum conf_type {
   CONF_TYPE_INT,
   CONF_TYPE_BOOL,
   CONF_TYPE_ENUM,
   CONF_TYPE_STR,
   CONF_TYPE_LIST
};

typedef struct {
   const char *name, *menuname;
   char shortname;
   const char *help;
   enum conf_type type;
   const ConfEnum *etype;	/* enum */
   unsigned char maxlen;	/* str -- remove? */
   char flags;
   int intval;			/* int, bool, enum */
   char *strval;		/* str */
   char **listval;		/* list */
   char dirtlvl;
} ConfItem;

#define CONFI(n,mn,sn,h,df) {n,mn,sn,h,CONF_TYPE_INT,NULL,0,0,df,NULL,NULL,DIRT_NONE} /* integer(df) */
#define CONFB(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_BOOL,NULL,0,0,0,NULL,NULL,DIRT_NONE} /* boolean(false) */
#define CONFNB(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_BOOL,NULL,0,0,1,NULL,NULL,DIRT_NONE} /* boolean(true) */
#define CONFNS(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_BOOL,NULL,0,CI_NOSAVE,0,NULL,NULL,DIRT_NONE} /* boolean(false), no save */
#define CONFE(n,mn,sn,h,df,e) {n,mn,sn,h,CONF_TYPE_ENUM,e,0,0,df,NULL,NULL,DIRT_NONE} /* enum(df) */
#define CONFS(n,mn,sn,h,df,l) {n,mn,sn,h,CONF_TYPE_STR,NULL,l,0,0,df,NULL,DIRT_NONE} /* string(df) */
#define CONFL(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_LIST,NULL,0,0,0,NULL,NULL,DIRT_NONE} /* list({}) */

typedef struct {
   ConfItem *items;
   const char *name,*desc;
} ConfModule;

#define DIRT_NONE 0  /* value is built-in default */ 
#define DIRT_ARGS 1  /* value was specified on the command line */
#define DIRT_FILE 2  /* value loaded from config file */
#define DIRT_MODF 3  /* value modified (eg. from menu) */

#define CI_NOSAVE 0x01

char *find_conf_file(char *buf,const char *basename);

int load_conf(const ConfModule *conf,const char *fn);
int save_conf(const ConfModule *conf,const char *fn,int dirt);

void set_conf(ConfItem *ci,char *val,int dirt);
void conf_clear_list(ConfItem *ci);

/* return non-zero if we ought to quit (eg. if args didn't make sense) */
int conf_args(const ConfModule *conf,int argc,char **argv);

int conf_greatest_dirtlevel(const ConfModule *conf);

#endif

// Local Variables:
// c-basic-offset: 3
// End:
