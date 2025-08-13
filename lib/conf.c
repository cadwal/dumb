#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "conf.h"

/* #define CONF_DEBUG */

char *find_conf_file(char *buf,const char *basename) {
   char *s;
   FILE *f;
   strcpy(buf,basename);
   if((f=fopen(buf,"r"))!=NULL) {
      fclose(f);
      return buf;
   };
   s=getenv("HOME");
   if(!s) return buf;
   strcpy(buf,s);
   if(buf[strlen(buf)-1]!='/') strcat(buf,"/");
   strcat(buf,basename);
   return buf;
};

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

static void set_conf_list(ConfItem *ci,char *val,int dirt) {
   char *s;
   while(val&&*val) {
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
      if(ci->flags&CI_LIST) 
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
	 if(ci->flags&CI_LIST) {
	    char **s=ci->listval;
	    while(s&&*s) {
	       fprintf(fout,"%s",*s);
	       s++;
	       if(*s) putc(' ',fout);
	    };
	    putc('\n',fout);
	 }
	 else if(ci->flags&CI_STR) {
	    if(ci->strval) fprintf(fout,"%s\n",ci->strval);
	    else putc(' ',fout);
	 }
	 else if(ci->flags&CI_ENUM) {
	    fprintf(fout,"%s\n",ci->etype[ci->intval].name);
	 }
	 else fprintf(fout,"%d\n",ci->intval);
      };
   };
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
      printf("--%s-%s ",conf->name,it->name);
      if(it->flags&CI_ENUM) printf("<enum> ");
      else if(it->flags&CI_INT) printf("<integer> ");
      else if(it->flags&CI_STR) printf("<string> ");
      else if(it->flags&CI_LIST) printf("<list> ");
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
   if(ci->flags&CI_BOOL) {
      printf("  Type=%s\n","Boolean");
      printf("  Default=%d\n",ci->intval);
   };
   if(ci->flags&CI_INT) {
      printf("  Type=%s\n","Integer");
      printf("  Default=%d\n",ci->intval);
   };
   if(ci->flags&CI_STR) {
      printf("  Type=%s\n","String");
      printf("  Default=%s\n",ci->strval);
   };
   if(ci->flags&CI_ENUM) {
      printf("  Type=%s\n","Enumerated");
      printf("  Default=%s\n",ci->etype[ci->intval].name);
      printf("  Values=");
      pr_etype(ci->etype);
      putchar('\n');
   };
   if(ci->flags&CI_LIST) printf("  Type=%s\n","List");
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
   return 0;
};

void set_conf(ConfItem *ci,char *val,int dirt) {
#ifdef CONF_DEBUG
   printf("set_conf(%s,%s,%d)\n",ci->name,val?val:"<null>",dirt);
#endif
   /* fill in new intval */
   if(ci->flags&CI_BOOL) ci->intval=1;
   if(val) ci->intval=atoi(val);
   /* deal with enums */
   if((ci->flags&CI_ENUM)&&val&&isalpha(*val))
      ci->intval=lookup_etype(ci->etype,val);
   /* fill in new strval */
   if(ci->flags&CI_STR) { 
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
   };
   /* fill in new listval */
   if(ci->flags&CI_LIST) {
      if(ci->listval==NULL) {
	 ci->listval=(char **)calloc(2,sizeof(char *));
	 if(dirt>DIRT_ARGS) ci->listval[0]=strdup(val);
	 else ci->listval[0]=val;
      }
      else {
	 int n=0;
	 while(ci->listval[n]) n++;
	 ci->listval=(char **)realloc(ci->listval,(n+2)*sizeof(char *));
	 if(dirt>DIRT_ARGS) ci->listval[n]=strdup(val);
	 else ci->listval[n]=val;
	 ci->listval[n+1]=(char *)NULL;
      };
   };
   /* fill in new dirtlevel */
   ci->dirtlvl=dirt;
};

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
	 else set_conf(ci,NULL,DIRT_ARGS);
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


