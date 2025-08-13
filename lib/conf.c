#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "conf.h"

/* #define CONF_DEBUG */

/* Find the configuration file and copy its name in BUF which must
 * have enough room -- overflow is not checked (FIXME).  Return BUF.
 */
char *find_conf_file(char *buf,const char *basename) {
   char *s;
   FILE *f;
   strcpy(buf,basename);
   if((f=fopen(buf,"r"))!=NULL) {
      fclose(f);
      return buf;
   };
   s=getenv("HOME");
   /* Someone might get nasty and set "HOME=" (empty string) */
   if(!s || !s[0]) return buf;
   strcpy(buf,s);
   if(buf[strlen(buf)-1]!='/') strcat(buf,"/");
   strcat(buf,basename);
   return buf;
};

/* Find the ConfItem the name S refers to, and return its address or
 * NULL.  Parameter CONF points to an array of ConfModules, terminated
 * by name=NULL.  */
static ConfItem *lookup_longname(const ConfModule *conf,const char *s) {
   ConfItem *ci;
   size_t l=0;
   while(conf->name) {
      l=strlen(conf->name);
      if(strlen(s)>l&&s[l]=='-'&&!strncmp(conf->name,s,l)) break;
      conf++;
   };
   if(conf->name==NULL)
      return NULL;
   s+=l+1;
   for(ci=conf->items;ci->name;ci++)
      if(!strcmp(ci->name,s)) return ci;
   return NULL;
};
/* Find the ConfItem the character CH refers to, and return its
 * address or NULL.  Parameter CONF points to an array of ConfModules,
 * terminated by name=NULL.  */
static ConfItem *lookup_shortname(const ConfModule *conf,char ch) {
   while(conf->name) {
      ConfItem *ci=conf->items;
      while(ci->name) {
	 if(ci->shortname==ch) return ci;
	 ci++;
      };
      conf++;
   };
   return NULL;
};

/* Append all whitespace-separated strings from VAL to CI.  If
 * anything is appended, set the dirt level to DIRT.  Destroy the
 * string CI in the process.  */
static void set_conf_list(ConfItem *ci,char *val,int dirt) {
   char *s;
   while(val&&*val) {
      /* FIXME: use isspace()?  */
      while(*val==' '||*val=='\t') val++;
      if(!*val) break;
      s=strchr(val,' ');
      if(s) {
	 *s=0;
	 s++;
      };
      set_conf(ci,val,dirt);
      val=s;
   };
};

#define LOAD_CONF_BUFLEN 256
int load_conf(const ConfModule *conf,const char *fn) {
   FILE *fin=fopen(fn,"r");
   if(!fin) return 1;
   while(!feof(fin)) {
      ConfItem *ci;
      char buf[LOAD_CONF_BUFLEN],*s,*t;
      if(fgets(buf,LOAD_CONF_BUFLEN,fin)==NULL) break;
      /* eat rest of line, if longer than buflen */
      s=strchr(buf,'\n');
      if(s) *s=0;
      else while(!feof(fin)&&getc(fin)!='\n');
      /* trim comment */
      s=strchr(buf,';');
      if(s) *s=0;
      /* find seperator */
      s=strchr(buf,'=');
      if(!s) continue;
      *s=0;
      s++;
      /* find confitem */
      t=buf;
      while(*t==' '||*t=='\t') t++;
      ci=lookup_longname(conf,t);
      if(!ci) continue;
      /* set it */
      if(ci->type == CONF_TYPE_LIST) 
	 set_conf_list(ci,s,DIRT_FILE);
      else 
	 set_conf(ci,s,DIRT_FILE);
   };
   return 0;
};

int save_conf(const ConfModule *conf,const char *fn,int dirt) {
   FILE *fout=fopen(fn,"w");
   if(!fout) return 1;
   fprintf(fout,"; %s\n; this file was created automatically, but you can"
	   " modify it if you want\n",fn);
   for(;conf->name;conf++) {
      const ConfItem *ci;
      int nout=0;
      for(ci=conf->items;ci->name;ci++) {
	 if(ci->dirtlvl<dirt) continue;
	 if(ci->flags&CI_NOSAVE) continue;
	 if((nout++)==0)
	    fprintf(fout,"\n; Module `%s': %s\n",conf->name,conf->desc);
	 fprintf(fout,"%s-%s=",conf->name,ci->name);
	 switch (ci->type) {
	 default:
	    fprintf(stderr, "internal error: save_conf():"
		    " strange type %d, treating as int\n", (int) ci->type);
	    /* fallthrough */
	 case CONF_TYPE_INT:
	 case CONF_TYPE_BOOL:
	    fprintf(fout,"%d\n",ci->intval);
	    break;
	 case CONF_TYPE_ENUM:
	    fprintf(fout,"%s\n",ci->etype[ci->intval].name);
	    break;
	 case CONF_TYPE_STR: 
	    if(ci->strval) fprintf(fout,"%s\n",ci->strval);
	    else putc(' ',fout);
	    break;
	 case CONF_TYPE_LIST:
	    {
	       char **s=ci->listval;
	       while(s&&*s) {
		  fprintf(fout,"%s",*s);
		  s++;
		  if(*s) putc(' ',fout);
	       };
	       putc('\n',fout);
	    }
	    break;
	 }
      }
   }
   putc('\n',fout);
   fclose(fout);
   return 0;
};

static int usage(const ConfModule *conf,const char *prog,const char *err) {
   if(err) printf("I don't understand your command line.  The token which "
		  "is causing trouble\nis `%s'.\n\n",err);
   printf("usage: %s [switch [parameter]] [switch [parameter]]...\n\n",prog);
   printf("For a list of all switches available, invoke %s -? all\n\n",prog);
   printf("Or, for switches relating to a particular module, %s -? <module>\n"
	  "where module is one of: all",prog);
   while(conf->name) {
      printf(", %s",conf->name);
      conf++;
   };
   printf("\n\nFor information on a particular switch: %s -? <switch>\n\n",
	  prog);
   return 1;
};

static void modhelp(const ConfModule *conf) {
   const ConfItem *it=conf->items;
   printf("Module `%s': %s\n",conf->name,conf->desc);
   if(!it->name) printf("<no configurable items>\n");
   else while(it->name) {
      printf("--%s-%s",conf->name,it->name);
      switch (it->type) {
      case CONF_TYPE_INT: printf(" <integer>"); break;
      case CONF_TYPE_BOOL: /* no-op */ break;
      case CONF_TYPE_ENUM: printf(" <enum>"); break;
      case CONF_TYPE_STR: printf(" <string>"); break;
      case CONF_TYPE_LIST: printf(" <list>"); break;
      default: 
	 printf(" <internal error: strange type %d>", (int) it->type);
	 break;
      }
      if(it->help) printf(": %s",it->help);
      if(it->shortname) printf(" (short form -%c)",it->shortname);
      putchar('\n');
      it++;
   };
};

static void pr_etype(const ConfEnum *ce) {
   while(ce->name) {
      printf("%s",ce->name);
      ce++;
      if(ce->name) putchar(',');
   };
};

static void swhelp(ConfItem *ci,const char *mod) {
   printf("Help for %s:\n  `%s'\n",mod,ci->help);
   switch (ci->type) {
   case CONF_TYPE_INT:
      printf("  Type=%s\n","Integer");
      printf("  Default=%d\n",ci->intval);
      break;
   case CONF_TYPE_BOOL:
      printf("  Type=%s\n","Boolean");
      printf("  Default=%d\n",ci->intval);
      break;
   case CONF_TYPE_ENUM:
      printf("  Type=%s\n","Enumerated");
      printf("  Default=%s\n",ci->etype[ci->intval].name);
      printf("  Values=");
      pr_etype(ci->etype);
      putchar('\n');
      break;
   case CONF_TYPE_STR:
      printf("  Type=%s\n","String");
      printf("  Default=%s\n",ci->strval);
      break;
   case CONF_TYPE_LIST:
      printf("  Type=%s\n","List");
      break;
   default:
      printf("  Type=%d (internal error)\n", (int) ci->type);
      break;
   }
   if(ci->shortname) printf("  Short Form=-%c\n",ci->shortname);
   if(ci->maxlen) printf("  Maxlen=%d\n",(int)(ci->maxlen));
   if(ci->flags&CI_NOSAVE) printf("  Cannot be saved in a config file.\n");
};

static int help(const ConfModule *conf,const char *prog,const char *mod) {
   usage(conf,prog,NULL);
   /* help with a switch */
   if(mod[0]=='-') {
      ConfItem *ci;
      if(mod[1]=='?') return 1;
      else if(mod[1]=='-') ci=lookup_longname(conf,mod+2);
      else ci=lookup_shortname(conf,mod[1]);
      if(ci) swhelp(ci,mod);
      else printf("`%s' is not a switch I recognise\n",mod);
   }
   /* help with a module */
   else {
      if(!strcasecmp(mod,"all")) mod=NULL;
      while(conf->name) {
	 if(mod==NULL||!strcasecmp(conf->name,mod)) {
	    modhelp(conf);
	    putchar('\n');
	 };
	 conf++;
      };
   };
   return 1;
};

static int lookup_etype(const ConfEnum *ce,const char *s) {
   int i;
   for(i=0;ce[i].name;i++)
      if(!strcasecmp(ce[i].name,s)) return i;
   /* FIXME: halt the program if given from cmdline */
   /* FIXME: stderr */
   printf("\"%s\" is not an option.  I'll use \"%s\" instead.\n",
	  s, ce[0].name);
   return 0;
};

/* CONF_TYPE_STR:
 * If ci->dirtlvl <= DIRT_ARGS, ci->strval points to a constant and
 * ci->maxlen doesn't matter.  Otherwise, ci->strval points to a
 * malloc(ci->maxlen) area that can be free()d.
 *
 * CONF_TYPE_LIST:
 * ci->listval may either be NULL or point to a realloc()able table of
 * pointers to strings.  There is a NULL pointer marking the end of
 * the table.  Some of the strings may be read-only constants and the
 * rest from strdup(); there's no way to know what is what.  This implies
 * that you can't remove them from the list without leaking memory.
 * [UPDATE] Now strings in the list are always allocated with strdup().
 */
void set_conf(ConfItem *ci,char *val,int dirt) {
#ifdef CONF_DEBUG
   printf("set_conf(%s,%s,%d)\n",ci->name,val?val:"<null>",dirt);
#endif
   switch (ci->type) {
   case CONF_TYPE_INT:
      /* fill in new intval */
      if (val) 
	 ci->intval = atoi(val);
      else {
	 /* FIXME: show module name too */
	 fprintf(stderr, "switch requires an argument -- %s\n",
		 ci->name);
	 return; /* without changing ci->dirtlvl */
      }
      break;
   case CONF_TYPE_BOOL:
      if (val)
	 ci->intval = atoi(val);
      else
	 ci->intval = 1;
      break;
   case CONF_TYPE_ENUM:
      /* deal with enums */
      if (val) {
	 if (isdigit(*val))
	    ci->intval = atoi(val);
	 else
	    ci->intval = lookup_etype(ci->etype, val);
      } else {
	 /* FIXME: show module name too */
	 fprintf(stderr, "switch requires an argument -- %s\n",
		 ci->name);
	 return; /* without changing ci->dirtlvl */
      }
      break;
   case CONF_TYPE_STR:
      /* fill in new strval */
      if(val&&dirt>DIRT_ARGS) {
	 if(ci->strval==NULL||ci->dirtlvl<=DIRT_ARGS) 
	    ci->strval=(char *)malloc(ci->maxlen);
	 if(!ci->strval) return;
	 strncpy(ci->strval,val,ci->maxlen-1);
	 ci->strval[ci->maxlen-1]=0;
      }
      else {
	 if(ci->strval&&ci->dirtlvl>DIRT_ARGS) 
	    free(ci->strval);
	 ci->strval=val;
      };
      break;
   case CONF_TYPE_LIST:
      /* fill in new listval */
      if(ci->listval==NULL) {
	 ci->listval=(char **)calloc(2,sizeof(char *));
	 if(val) ci->listval[0]=strdup(val);
      }
      else {
	 int n=0;
	 while(ci->listval[n]) n++;
	 ci->listval=(char **)realloc(ci->listval,(n+2)*sizeof(char *));
	 ci->listval[n]=strdup(val);
	 ci->listval[n+1]=(char *)NULL;
      };
      break;
   default:
      fprintf(stderr, "internal error: set_conf(): strange type %d\n",
	      ci->type);
      break;
   }
   /* fill in new dirtlevel */
   ci->dirtlvl=dirt;
};

void
conf_clear_list(ConfItem *ci)
{
   assert(ci->type == CONF_TYPE_LIST);
   if (ci->listval) {
      char **p;
      for (p = ci->listval; *p; p++)
	 free(*p);
   }
   free(ci->listval);
   ci->listval = NULL;
   ci->dirtlvl = DIRT_MODF;
}

/* parse argument list in argc,argv using array of modules conf */ 
int conf_args(const ConfModule *conf,int argc,char **argv) {
   int i;
   ConfItem *ci=NULL;
   for(i=1;i<argc;i++) {
      if(argv[i][0]!='-'||isdigit(argv[i][1])) {
	 if(ci) set_conf(ci,argv[i],DIRT_ARGS);
	 else return usage(conf,argv[0],argv[i]);
      }
      else if(argv[i][1]=='-') {
	 ci=lookup_longname(conf,argv[i]+2);
	 if(!ci) return usage(conf,argv[0],argv[i]);
	 if(ci->type==CONF_TYPE_BOOL) 
	    set_conf(ci,NULL,DIRT_ARGS);
      }
      else if(argv[i][1]=='?') {
	 if(argc>i+1) return help(conf,argv[0],argv[i+1]);
	 else return usage(conf,argv[0],NULL);
      }
      else {
	 ci=lookup_shortname(conf,argv[i][1]);
	 if(!ci) return usage(conf,argv[0],argv[i]);
	 if(argv[i][2]) set_conf(ci,argv[i]+2,DIRT_ARGS);
	 else if(ci->type==CONF_TYPE_BOOL) set_conf(ci,NULL,DIRT_ARGS);
      };
   };
   return 0;
};

int conf_greatest_dirtlevel(const ConfModule *conf) {
   ConfItem *ci;
   int maxdirt=0;
   for(;conf->name;conf++) for(ci=conf->items;ci->name;ci++)
      if(ci->dirtlvl>maxdirt) maxdirt=ci->dirtlvl;
   return maxdirt;
};

// Local Variables:
// c-basic-offset: 3
// End:
