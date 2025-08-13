#include <config.h>

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

#include <mme/mme_api.h>

#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "sound.h"

/* configuration */

/* size of window in hundredths of seconds */
#define WINDOW_IN_HSEC 20

/* fragment size in bytes */
#define FRAGSIZE 4096
#define NUMBUF 4

/* #undef DUMB_CONFIG_SOUND_STEREO */

#ifdef DUMB_CONFIG_SOUND_16BIT
typedef signed short MonoSample;
#define char2MonoSample(c) (((int)(c)-127)*24)
#define SAMPLE_BITNESS 16
#else  /* !DUMB_CONFIG_SOUND_16BIT */
typedef unsigned char MonoSample;
#define char2MonoSample(c) ((c)/4)
#define SAMPLE_BITNESS 8
#endif /* !DUMB_CONFIG_SOUND_16BIT */

#ifdef DUMB_CONFIG_SOUND_STEREO
typedef struct {
   MonoSample l,r;
} Sample;
static inline Sample char2Sample(unsigned char c,SoundBalance bal) {
   Sample s;
   s.l=fixmul(char2MonoSample(c),bal.lfvol);
   s.r=fixmul(char2MonoSample(c),bal.rfvol);
   return s;
}
static inline void addsamples(Sample *d,Sample s) {
   d->l+=s.l;
   d->r+=s.r;
}
#define SAMPLE_CHANNELS 2
#else  /* !DUMB_CONFIG_SOUND_STEREO */
typedef MonoSample Sample;
#define char2Sample(c,bal) char2MonoSample(c)
#define addsamples(d,s) (*(d))+=(s)
#define SAMPLE_CHANNELS 1
#endif /* !DUMB_CONFIG_SOUND_STEREO */


/* sound queue stuff */
typedef struct {
   const unsigned char *data;
   SoundBalance bal;
   int count;
   int myspeed;
} SQEnt;

#define SQMAX 26

static SQEnt sq[SQMAX];

/* a buffer */
static Sample *fragbuf=NULL;

/* some control data */
static int speed;
static fixed volume=FIXED_ONE_HALF;

/* if driver has more than this many frags free, it's time to send more data */
static int slush_fragments;

/* how big a fragment is in Samples (not bytes) */
static unsigned int fragment_size;

/* descriptor amd buffers for the sounddevice */
static  HWAVEOUT       mms_device_handle = 0;
static  int            mms_nextbuf = 0;
static  LPSTR          mms_audio_buffer = NULL;
static  LPWAVEHDR      WaveHeader = NULL;

/* callbackfunction used by mme */
static void my_callback(HANDLE hWaveOut,
			UINT wMsg,
			DWORD dwInstance,
			LPARAM lParam1,
			LPARAM lParam2)
{
  switch(wMsg) {
  case WOM_OPEN:
  case WOM_CLOSE:
    /* Ignore these */
    break;
  case WOM_DONE:
    slush_fragments--;
    break;
  default:
    break;
  }
}


void init_sound(int s) {
   MMRESULT	  status;
   LPPCMWAVEFORMAT lpWaveFormat;

   int bitness=SAMPLE_BITNESS,stereority=SAMPLE_CHANNELS;
   int window, outdevs;

   /* check, are we already inited? */
   if(mms_device_handle != 0) reset_sound();

   /* clear queue */
   memset(&sq,0,sizeof(SQEnt)*SQMAX);

   /* check if sounddevice is available */
   if ((outdevs = waveOutGetNumDevs()) < 1) {
     logprintf(LOG_ERROR,'S',"init_sound: no sounddevice present");
     return;
   }
      
   /* configure and open it */
   
   if((lpWaveFormat = (LPPCMWAVEFORMAT)
       mmeAllocMem(sizeof(PCMWAVEFORMAT))) == NULL ) {
     logprintf(LOG_ERROR,'S',"Failed to allocate PCMWAVEFORMAT struct");
     return;
   }

   lpWaveFormat->wf.nSamplesPerSec = s;
   lpWaveFormat->wf.nChannels = stereority;
   lpWaveFormat->wBitsPerSample = bitness;
   lpWaveFormat->wf.wFormatTag = WAVE_FORMAT_PCM;
   
   lpWaveFormat->wf.nBlockAlign = lpWaveFormat->wf.nChannels *
     ((lpWaveFormat->wBitsPerSample+7)/8);
   lpWaveFormat->wf.nAvgBytesPerSec = lpWaveFormat->wf.nBlockAlign *
     lpWaveFormat->wf.nSamplesPerSec;
   
   /* Open the audio device in the appropriate rate/format */
   status = waveOutOpen( &mms_device_handle,
			 WAVE_MAPPER,
			 (LPWAVEFORMAT)lpWaveFormat,
			 (void (*)())my_callback,
			 (unsigned int)NULL,
			 CALLBACK_FUNCTION | WAVE_OPEN_SHAREABLE );
   mmeFreeMem(lpWaveFormat);
   
   if(status != MMSYSERR_NOERROR) {
     logprintf(LOG_ERROR,'S',"waveOutOpen failed - status = %d\n", status);
     return;
   }
   
   /* Allocate wave header for use in write */
   if((WaveHeader = (LPWAVEHDR)
       mmeAllocMem(sizeof(WAVEHDR))) == NULL ) {
     logprintf(LOG_ERROR,'S',"Failed to allocate WAVEHDR struct");
     return;
   }

   /* figure out window size */
   fragment_size=FRAGSIZE/sizeof(Sample);
   window=(WINDOW_IN_HSEC*speed)/(100*fragment_size);
   if(window<2) window=2;

   /* allocate buffer */
   if ((fragbuf = (Sample*) mms_audio_buffer = mmeAllocBuffer(FRAGSIZE*NUMBUF)) == NULL) {
     logprintf(LOG_ERROR,'S',"Failed to allocate shared audio buffer");
     mmeFreeMem(WaveHeader);
     return;
   }

   /* say what we've done */
   logprintf(LOG_INFO,'S',
	     "init_sound: mme_handle=%d fragsize=%d (%ld bytes)",
	     mms_device_handle,fragment_size,fragment_size*sizeof(Sample));
   logprintf(LOG_INFO,'S',
	     "init_sound: speed=%d window=%d slush=%d",
	     speed,window,
	     slush_fragments);
}

/* stop playing now, and throw away anything you were about to play */
void purge_sound(void) {
   /* clear queue */
   memset(&sq,0,sizeof(SQEnt)*SQMAX);
   /* TODO: purge any buffers the kernel might have */
}

/* do all of the above, and shut down the sound device, clean up, etc */
void reset_sound(void) {
   purge_sound();
   if(mms_audio_buffer) mmeFreeBuffer(mms_audio_buffer);
   if(mms_device_handle != 0) {
     mmeFreeMem(WaveHeader);
     waveOutClose(mms_device_handle);
   }
   mms_device_handle=0;
   fragbuf=NULL;
}

void set_sound_volume(fixed vol) {
   volume=vol;
}

void play_sound(const unsigned char *buf,int count,
		SoundBalance bal,int myspeed) {
   int i;
   /*logprintf(LOG_DEBUG,'S',"play_sound: %d bytes: %f seconds",
	     count,((float)count)/((float)myspeed));*/
   if(mms_device_handle == 0) return;
   /* find a free queue slot */
   for(i=0;i<SQMAX;i++) if(sq[i].count==0) break;
   if(i>=SQMAX) {
      logprintf(LOG_ERROR,'S',
		"play_sound: out of queue slots (sound discarded)");
      return;
   }
   /* enqueue the sound */
   sq[i].data=buf;
   sq[i].count=count;
   sq[i].bal=bal;
   sq[i].myspeed=myspeed;
}

static int sqe2buf(Sample *buf,const SQEnt *s) {
   int ns=0,nb=0;
   int db=256 + s->bal.bend * 4;

   if(db<32) db=32;
   while(ns<fragment_size&&(nb>>8)<s->count) {
      addsamples(buf+ns,char2Sample(s->data[nb >> 8],s->bal));
      ns++;
      nb+=db;
   }
   return nb >> 8;  
}

void poll_sound() {
   if(mms_device_handle == 0) return;
   while(1) {
      int i,nplayed=0;
      if (mmeCheckForCallbacks())
	mmeProcessCallbacks();
      /* if there is <=slush space free, don't stuff in any more */
      if (slush_fragments >= NUMBUF)
	break;
      /* prepare a new fragment */
      memset(fragbuf, 0, FRAGSIZE);
      for(i=0;i<SQMAX;i++) if(sq[i].count) {
	 int bytes;
	 nplayed++;
	 bytes=sqe2buf(fragbuf,sq+i);
	 if(sq[i].count<bytes) sq[i].count=0;
	 else {
	    sq[i].count-=bytes;
	    sq[i].data+=bytes;
	 }
      }
      /* write it */
      /*logprintf(LOG_DEBUG,'S',"poll_sound: writing fragment (%d sounds)",
	nplayed);*/
      WaveHeader->lpData = (LPSTR)fragbuf;
/*      printf("Slush: %d, nextbuf: %d\n", slush_fragments, mms_nextbuf);*/
      mms_nextbuf++;
      if (mms_nextbuf == NUMBUF)
	mms_nextbuf = 0;
      fragbuf = (Sample*)(mms_audio_buffer+mms_nextbuf*FRAGSIZE);
      WaveHeader->dwBufferLength = FRAGSIZE;
      if (waveOutWrite(mms_device_handle, WaveHeader,
		       sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
	  logprintf(LOG_INFO,'S',"waveOutWrite failed");
      }
      else
	slush_fragments++;
   }
}
