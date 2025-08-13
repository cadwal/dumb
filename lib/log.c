#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"

typedef struct _LogFile {
   struct _LogFile *next;
   int lvl;
   char cl[32];
   FILE *f,*falloc;
} LogFile;

static LogFile *logs=NULL;
static int errors=0;

static void log_cleanup(void) { 
   while(logs!=NULL) {
      if(logs->falloc) fclose(logs->falloc);
      logs=logs->next;
   }
}

void log_stream(FILE *f,int lvl,const char *cl) {
   LogFile *log=(LogFile*)calloc(sizeof(LogFile),1);
   log->lvl=lvl;
   if(cl==NULL) cl="*";
   strncpy(log->cl,cl,31);
   log->f=f;
   log->falloc=NULL;
   log->next=logs;
   logs=log;
}
void log_file(const char *fname,int lvl,const char *cl) {
   FILE *f=fopen(fname,"a");
   LogFile *log;
   if(f==NULL) {
      logprintf(LOG_ERROR,'L',"Error %d opening logfile '%s'",errno,fname);
      return;
   }
   log=(LogFile*)calloc(sizeof(LogFile),1);
   log->lvl=lvl;
   if(cl==NULL) cl="*";
   strncpy(log->cl,cl,31);
   log->f=f;
   log->falloc=f;
   log->next=logs;
   logs=log;
}

void log_exit(void) { 
   logprintf(LOG_INFO,'L',"Normal exit.");
   log_cleanup();
   exit(0);
}

void log_chkerror(int maxerrs) {
   if(errors>maxerrs) logprintf(LOG_FATAL,'L',"%d errors occured.",errors);
}

static int log_ok(const LogFile *log,int l,char c) {
   if(l>log->lvl) return 0;
   if(*log->cl=='*') return 1;
   else if(*log->cl=='!') return strchr(log->cl,c)==NULL;
   else return strchr(log->cl,c)!=NULL;
}

void do_logfatal(void) {
   logprintf(LOG_INFO,'L',"A fatal error occured.");
   perror("perror");
   log_cleanup();
   exit(1);
};

void logvprintf(int lvl,char cl,const char *fmt,va_list argl) {
   LogFile *log=logs;
   if(lvl<0) return;
   while(log) {
      if(log_ok(log,lvl,cl)) {
	 vfprintf(log->f,fmt,argl);
	 fputc('\n',log->f);
      }
      log=log->next;
   };
   if(lvl==LOG_FATAL) do_logfatal();
   if(lvl==LOG_ERROR) errors++;
}

void logprintf(int lvl,char cl,const char *fmt,...) {
   va_list ap;
   va_start(ap,fmt);
   logvprintf(lvl,cl,fmt,ap);
   va_end(ap);
}
void logfatal(char cl,const char *fmt,...) {
   va_list ap;
   va_start(ap,fmt);
   logvprintf(LOG_FATAL,cl,fmt,ap);
   va_end(ap);
   exit(1); /* should never get here */
}
