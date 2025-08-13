#include <config.h>

#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "timer.h"

/* alarm will occur after this many micro-seconds */
#define INIT_USEC 999999
#define INIT_SEC 0

void init_timer(void) {
   reset_timer();
   signal(SIGALRM,SIG_IGN);
}

void reset_timer(void) {
   struct itimerval it;
   it.it_value.tv_usec=INIT_USEC;
   it.it_value.tv_sec=INIT_SEC;
   it.it_interval.tv_usec=0;
   it.it_interval.tv_sec=0;
   setitimer(ITIMER_REAL,&it,NULL);
}

int read_timer(void) {
   struct itimerval it;
   getitimer(ITIMER_REAL,&it);
   if(it.it_value.tv_sec>0) return 0;
   return (INIT_USEC-it.it_value.tv_usec)/USEC_PER_TICK;
}
