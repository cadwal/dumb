#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/soundcard.h>

#include "lib/log.h"
#include "lib/safem.h"
#include "sound.h"

/* configuration */

/* size of window in hundredths of seconds */
#define WINDOW_IN_HSEC 20

/* size of fragment to ask sound driver for, in bytes log 2 */
#define FRAGLEN_LOG2 8 /* ie. 256 bytes */

/*#define NO_STEREO_SOUND*/

#ifdef NO_16BIT_SOUND
typedef unsigned char MonoSample;
#define char2MonoSample(c) ((c)/4)
#define SAMPLE_BITNESS 8
#else
typedef signed short MonoSample;
#define char2MonoSample(c) (((int)(c)-127)*64)
#define SAMPLE_BITNESS 16
#endif

#ifdef NO_STEREO_SOUND
typedef MonoSample Sample;
#define char2Sample(c,bal) char2MonoSample(c)
#define addsamples(d,s) (*(d))+=(s)
#define SAMPLE_CHANNELS 1
#else
typedef struct {
   MonoSample l,r;
} Sample;
static inline Sample char2Sample(unsigned char c,SoundBalance bal) {
   Sample s;
   s.l=fixmul(char2MonoSample(c),bal.lfvol);
   s.r=fixmul(char2MonoSample(c),bal.rfvol);
   return s;
};
static inline void addsamples(Sample *d,Sample s) {
   d->l+=s.l;
   d->r+=s.r;
};
#define SAMPLE_CHANNELS 2
#endif


/* sound queue stuff */
typedef struct {
   const unsigned char *data;
   SoundBalance bal;
   int count;
   int myspeed;
} SQEnt;

#define SQMAX 64

static SQEnt sq[SQMAX];

static int first_poll;

/* a buffer */
static Sample *fragbuf=NULL;

/* some control data */
static int speed;
static fixed volume=FIXED_ONE_HALF;

/* if driver has more than this many frags free, it's time to send more data */
static int slush_fragments;

/* how big a fragment is in Samples (not bytes) */
static unsigned int fragment_size;

/* the file descriptor for the soundcard device */
static int fd=-1;

void init_sound(int s) {
   int bitness=SAMPLE_BITNESS,stereority=(SAMPLE_CHANNELS==2)?1:0;
   int fraginfo=0x7fff0000|FRAGLEN_LOG2;
   audio_buf_info abi;
   int window;

   /* check, are we already inited? */
   if(fd>=0) reset_sound();

   /* clear queue */
   memset(&sq,0,sizeof(SQEnt)*SQMAX);

   /* try to open dsp */
   fd=open("/dev/dsp",O_WRONLY);
   if(fd<0) {
      logprintf(LOG_ERROR,'S',"init_sound: open failed, errno=%d",errno);
      return;
   };

   /* andconfigure it */
   if(ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&fraginfo)) {
      logprintf(LOG_ERROR,'S',
		"init_sound: set %s failed, errno=%d",
		"fragsize",errno);
      close(fd);
      fd=-1;
      return;
   };
   if(ioctl(fd,SNDCTL_DSP_STEREO,&stereority)) {
      logprintf(LOG_ERROR,'S',
		"init_sound: set %s failed, errno=%d",
		"stereo",errno);
      close(fd);
      fd=-1;
      return;
   };
   if(ioctl(fd,SNDCTL_DSP_SETFMT,&bitness)) {
      logprintf(LOG_ERROR,'S',
		"init_sound: set %d-bit failed, errno=%d",
		bitness,errno);
      close(fd);
      fd=-1;
      return;
   };
   speed=s;
   if(ioctl(fd,SNDCTL_DSP_SPEED,&speed)) {
      logprintf(LOG_ERROR,'S',
		"init_sound: set %s failed, errno=%d",
		"speed",errno);
      close(fd);
      fd=-1;
      return;
   };

   if(fd<0) return;

   /* figure out window size */
   ioctl(fd,SNDCTL_DSP_GETOSPACE,&abi);
   fragment_size=abi.fragsize/sizeof(Sample);
   window=(WINDOW_IN_HSEC*speed)/(100*fragment_size);
   if(window<2) window=2;
   if(window>abi.fragstotal) window=abi.fragstotal;

   /* and work out slush frags from that */
   slush_fragments=abi.fragstotal-window;

   /* allocate buffer */
   fragbuf=safe_calloc(fragment_size,sizeof(Sample));

   /* say what we've done */
   logprintf(LOG_INFO,'S',
	     "init_sound: fd=%d fragsize=%d (%d bytes)",
	     fd,fragment_size,fragment_size*sizeof(Sample));
   logprintf(LOG_INFO,'S',
	     "init_sound: speed=%d window=%d (%f seconds) slush=%d",
	     speed,window,
	     (float)(window*fragment_size)/(float)speed,
	     slush_fragments);

   first_poll=1;
};

/* stop playing now, and throw away anything you were about to play */
void purge_sound(void) {
   /* clear queue */
   memset(&sq,0,sizeof(SQEnt)*SQMAX);
   /* TODO: purge any buffers the kernel might have */
};

/* do all of the above, and shut down the sound device, clean up, etc */
void reset_sound(void) {
   purge_sound();
   if(fragbuf) safe_free(fragbuf);
   if(fd>=0) close(fd);
   fd=-1;
   fragbuf=NULL;
};

void set_sound_volume(fixed vol) {
   volume=vol;
};

void play_sound(const unsigned char *buf,int count,
		SoundBalance bal,int myspeed) {
   int i;
   /*logprintf(LOG_DEBUG,'S',"play_sound: %d bytes: %f seconds",
	     count,((float)count)/((float)myspeed));*/
   if(fd<0) return;
   /* find a free queue slot */
   for(i=0;i<SQMAX;i++) if(sq[i].count==0) break;
   if(i>=SQMAX) {
      logprintf(LOG_ERROR,'S',
		"play_sound: out of queue slots (sound discarded)");
      return;
   };
   /* enqueue the sound */
   sq[i].data=buf;
   sq[i].count=count;
   sq[i].bal=bal;
   sq[i].myspeed=myspeed;
};

static int sqe2buf(Sample *buf,const SQEnt *s) {
   int ns=0,nb=0;
   int db=256 + s->bal.bend * 4;

   if(db<32) db=32;
   while(ns<fragment_size&&(nb>>8)<s->count) {
      addsamples(buf+ns,char2Sample(s->data[nb >> 8],s->bal));
      ns++;
      nb+=db;
   };
   return nb >> 8;
};

void poll_sound() {
   if(fd<0) return;
   while(1) {
      audio_buf_info abi;
      int i,nplayed=0;
      /* if there is <=slush space free, don't stuff in any more */
      ioctl(fd,SNDCTL_DSP_GETOSPACE,&abi);
      /*logprintf(LOG_DEBUG,'S',"poll_sound: %d frags free",abi.fragments);*/
      if(abi.fragments<=slush_fragments) break;
      if(abi.fragments==abi.fragstotal&&!first_poll)
	 logprintf(LOG_INFO,'S',"poll_sound: underrun detected");
      /* prepare a new fragment */
      memset(fragbuf,0,sizeof(Sample)*fragment_size);
      for(i=0;i<SQMAX;i++) if(sq[i].count) {
	 int bytes;
	 nplayed++;
	 bytes=sqe2buf(fragbuf,sq+i);
	 if(sq[i].count<bytes) sq[i].count=0;
	 else {
	    sq[i].count-=bytes;
	    sq[i].data+=bytes;
	 };
      };
      /* write it */
      /*logprintf(LOG_DEBUG,'S',"poll_sound: writing fragment (%d sounds)",
	nplayed);*/
      write(fd,fragbuf,fragment_size*sizeof(Sample));
   };
   first_poll=0;
};












