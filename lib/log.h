
#ifndef LOG_H
#define LOG_H

void logvprintf(int lev,char cl,const char *fmt,va_list argl);

void logprintf(int lev,char cl,const char *fmt,...)
 __attribute__((format (printf, 3, 4)));

void logfatal(char cl,const char *fmt,...) 
 __attribute__((noreturn, format (printf, 2, 3)));

void log_stream(FILE *f,int level,const char *cl);
void log_file(const char *fname,int level,const char *cl);

void log_exit(void);
void log_chkerror(int maxerrs);

#define LOG_FATAL 0
#define LOG_BANNER 1
#define LOG_ERROR 2
#define LOG_INFO 3
#define LOG_XINFO 4
#define LOG_DEBUG 5
#define LOG_ALL 5

#define log_stdout() log_stream(stdout,LOG_ALL,NULL)

#endif
