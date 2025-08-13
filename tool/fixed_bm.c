#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/time.h>

#include "libdumbutil/fixed.h"

static volatile loop;
static RETSIGTYPE alrm(int dummy) {
   loop=0;
}

static void rtimer(void) {
   struct itimerval it;
   loop=1;
   it.it_value.tv_usec=0;
   it.it_value.tv_sec=2;
   it.it_interval.tv_usec=0;
   it.it_interval.tv_sec=0;
   setitimer(ITIMER_REAL,&it,NULL);
   signal(SIGALRM,alrm);
   signal(SIGVTALRM,alrm);
}

/* these reproduce the losses due to procedure calls with fixmul etc.
   it is impossible to allow inlining with GCC without also allowing
   optimization, which can cheat some of the tests */
#ifdef FIXMUL_AS_MACRO
#define fltmul(a,b) (FIXED_TO_FLOAT(a)*FIXED_TO_FLOAT(b))
#define fltdiv(a,b) (FIXED_TO_FLOAT(a)/FIXED_TO_FLOAT(b))
#else
static inline float fltmul(fixed a,fixed b) 
{return FIXED_TO_FLOAT(a)*FIXED_TO_FLOAT(b);}
static inline float fltdiv(fixed a,fixed b) 
{return FIXED_TO_FLOAT(a)/FIXED_TO_FLOAT(b);}
#endif


int main(int argc,char **argv) {
   unsigned int i;
   setvbuf(stdout,NULL,_IONBF,0);
   if(argc>1) {
      printf("Seeding random number generator\n");
      srandom(atoi(argv[1]));
   }
   /*
   printf("sizeof(long long)=%d sizeof(long)=%d, sizeof(int)=%d\n",
	  sizeof(long long),sizeof(long),sizeof(int));
	  */

   printf("\nAdding random numbers...");
   i=0;
   rtimer();
   while(loop) {
      int n=random()+random();
      i++;
   }
   printf("  Fixed: %d",i);
   i=0;
   rtimer();
   while(loop) {
      float f=FIXED_TO_FLOAT(random())+FIXED_TO_FLOAT(random());
      i++;
   }
   printf("  Float: %d",i);

   printf("\nMultiplying random numbers...");
   i=0;
   rtimer();
   while(loop) {
      int n=fixmul(random(),random());
      i++;
   }
   printf("  Fixed: %d",i);
   i=0;
   rtimer();
   while(loop) {
      float f=fltmul(random(),random());
      i++;
   }
   printf("  Float: %d",i);

   printf("\nDividing random numbers...");
   i=0;
   rtimer();
   while(loop) {
      fixed a=random()&0xffffff;
      int n=fixdiv(a,a+FIXED_ONE);
      i++;
   }
   printf("  Fixed: %d",i);
   i=0;
   rtimer();
   while(loop) {
      fixed a=random()&0xffffff;
      float f=fltdiv(a,a+FIXED_ONE);
      i++;
   }
   printf("  Float: %d",i);

   printf("\n");
   return 0;
}


