#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conffile.h"
#include "confeng.h"

static void set_conf_list(ConfItem *ci,char *val,int dirt);

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
   }
   s=getenv("HOME");
   /* Someone might get nasty and set "HOME=" (empty string) */
   if(!s || !s[0]) return buf;
   strcpy(buf,s);
   if(buf[strlen(buf)-1]!='/') strcat(buf,"/");
   strcat(buf,basename);
   return buf;
}

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
      ci=conf_lookup_longname(conf,t);
      if(!ci) continue;
      /* set it */
      if(ci->type == CONF_TYPE_LIST) 
	 set_conf_list(ci,s,DIRT_FILE);
      else 
	 set_conf(ci,s,DIRT_FILE);
   }
   return 0;
}

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
	       }
	       putc('\n',fout);
	    }
	    break;
	 }
      }
   }
   putc('\n',fout);
   fclose(fout);
   return 0;
}

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
      }
      set_conf(ci,val,dirt);
      val=s;
   }
}

