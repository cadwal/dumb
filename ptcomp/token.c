#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "libdumbutil/safem.h"

#include "token.h"

static FILE *fin=NULL;
static char cppfile[256]="stdin";
static int line=0;

static int tok_ungot=0;
static char tokbuf[256]="";


void
begin_file(FILE *f, const char *name)
{
   fin = f;
   strcpy(cppfile, name);
   line = 0;
}

void
end_file(void)
{
}

const char *
next_token(void)
{
   char *s=tokbuf;
   int i;
   if(line==0) line=1;
   if(tok_ungot) {
      tok_ungot=0;
      return tokbuf;
   }
   memset(tokbuf,0,256);
   while(1) {
      if(feof(fin)) return NULL;
      i=getc(fin);
      *s = (char)i;
      if(*s!=' '&&*s!='\t'&&*s!='\r'&&i!=-1) break;
   }
   if(*s=='#') {
      while((*s++=getc(fin))!='\n') if(feof(fin)) return NULL;
      line=atoi(tokbuf+1);
      s=strchr(tokbuf,'"');
      if(s) {
	 char *t=cppfile;
	 s++;
	 while(*s!='"') *t++=*s++;
	 *t=0;
	 if(*cppfile==0) strcpy(cppfile,"stdin");
      }
      /*printf("line=%d file=(%s)\n",line,cppfile);*/
      s=tokbuf;
      *s=0;
      return "\n";
   }
   if(*s=='\n') {
      line++;
      *s=0;
      return "\n";
   }
   do {
      if(feof(fin)) return tokbuf;
      s++;
      i=getc(fin);
      *s=(char)i;
   } while(*s!=' '&&*s!='\t'&&*s!='\r'&&i!=-1&&*s!='\n'&&*s!='#');
   if(*s=='\n'||*s=='#') ungetc(*s,fin);
   *s=0;
   return tokbuf;
}

void
unget_token(void)
{
   tok_ungot++;
}

void
print_error_prefix(void)
{
   /* FIXME: stderr? */
   printf("%s:%d: ", cppfile, line);
}

void
synerr(const char *detail)
{
   print_error_prefix();
   printf("syntax error near token `%s'\n", tokbuf);
   if (detail)
      printf("\t%s\n", detail);
   exit(3);
}

void
err(const char *detail)
{
   print_error_prefix();
   printf("fatal error\n");
   if (detail)
      printf("\t%s\n", detail);
   exit(3);
}

// Local Variables:
// c-basic-offset: 3
// End:
