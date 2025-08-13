
#ifndef TIMER_H
#define TIMER_H

/* how long a tick is, in micro-seconds */
#define MSEC_PER_TICK 10

#define USEC_PER_TICK (MSEC_PER_TICK*1000)

void init_timer(void);
void reset_timer(void);
int read_timer(void);

#endif
