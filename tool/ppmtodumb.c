#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#include <ppm.h>
}
#else
#include <ppm.h>
#endif

#include "libdumbwad/wadstruct.h"

typedef enum {NoMode,
	      MkFlat,MkPatch,
	      MkColormap,MkPlayPal} Mode;

int compat=0,bpp=1,trans=1;

Mode mode=NoMode;
const char *playpal_fn="data/PLAYPAL.lump";
unsigned char playpal[256*3];

static void usage(void) {
   fprintf(stderr,
	   "Usage: ppmtodumb [option [argument]]...\n\n"
	   "ppmtodumb is a filter used to convert graphic files from the\n"
	   "portable pixmap format (PPM) to dumb's internal\n"
	   "formats.  It usually reads an input file from standard input\n"
	   "and writes its output to standard output.\n\n"
	   "The options are:\n"
	   "  -M <playpal>  : read the playpal data from <playpal>,\n"
	   "                  rather than from %s\n"
	   "  -m            : create a playpal\n"
	   "  -c            : create a colormap\n"
	   "  -2            : 2 byte pixels\n"
	   "  -4            : 4 byte pixels\n"
	   "  -f            : create a 64x64 flat-format graphic\n"
	   "  -p            : create a transparent patch\n"
	   "  -P            : create a non-transparent patch\n"
	   "  -i            : turn on id compatibility\n"  
	   "\n",playpal_fn);
   exit(2);
}

static void err(const char *m) {
   fprintf(stderr,"ppmtodumb: %s (errno=%d)\n",m,errno);
   exit(1);
}
/*static void warn(const char *m) {
   fprintf(stderr,"ppmtodumb: %s (converted anyway)\n",m);
}*/

static int mylog2(int i) {
   int j=0;
   while(j<32&&i>(1<<j)) j++;
   return j;
}

void loadpal(void) {
   FILE *f=fopen(playpal_fn,"rb");
   if(f==NULL) err("can't open playpal");
   fread(playpal,3,256,f);
   fclose(f);
}

#define SQ(i) (((double)(i))*((double)(i)))
unsigned char lookup_closest(int r, int g, int b) 
{
   int i;
   double mindist=65536.0*3.0;
   int minent=255;
   for(i=0;i<256;i++) {
      const unsigned char *pp=playpal+(i*3);
      double dist=0.0;
      dist+=SQ(r-pp[0]);
      dist+=SQ(g-pp[1]);
      dist+=SQ(b-pp[2]);
      if(dist<mindist) {
	 minent=i;
	 mindist=dist;
      }
      if(mindist<1.0) break;
   }
   return minent;
}

unsigned char lookup_pal(pixel p,pixval maxval) {
   int i;
   float mvf=maxval+1;
   /* check for transparent */
   if(trans&&p.r==0&&p.g==0&&p.b==1) return 255;
   /* fix up depth */
   if(maxval!=255) {
      p.r=(int)(((float)p.r/mvf)*256.0);
      p.g=(int)(((float)p.g/mvf)*256.0);
      p.b=(int)(((float)p.b/mvf)*256.0);
   }
   /* normal lookup */
   for(i=0;i<256;i++) {
      const unsigned char *pp=playpal+(i*3);
      if(p.r==pp[0]&&p.g==pp[1]&&p.b==pp[2]) return i;
   }
   /*warn("pixel matched none in palette");
   return 255;*/
   return lookup_closest(p.r,p.g,p.b);
}


void mkflat(void) {
   int cols,rows,x,y;
   pixval maxval;
   pixel **pix;
   loadpal();
   pix=ppm_readppm(stdin,&cols,&rows,&maxval);
   if(pix==NULL) err("bad ppm");
   if((cols==64&&rows==64)||(compat==0&&cols==128&&rows==128)) 
      for(y=0;y<rows;y++) for(x=0;x<cols;x++)
	 putchar(lookup_pal(pix[y][x],maxval));
   else err("this ppm is the wrong size for a flat");
}

void mkpatch(void) {
   PictData *p;
   int cols,rows,x,y,o;
   pixval maxval;
   pixel **pix;
   loadpal();
   pix=ppm_readppm(stdin,&cols,&rows,&maxval);
   if(pix==NULL) err("bad ppm");
   p=(PictData*)malloc(8+cols*4+(rows+4)*cols);
   if(p==NULL) err("out of memory");
   p->UMEMB(hdr).width=cols;
   p->UMEMB(hdr).height=rows;
   p->UMEMB(hdr).xoffset=p->UMEMB(hdr).yoffset=0;
   o=8+cols*4;
   for(x=0;x<cols;x++) {
      p->UMEMB(hdr).idx[x]=o;
      p->data[o++]=0;    /* initial row */
      p->data[o++]=rows; /* # of rows */
      p->data[o++]=0;
      for(y=0;y<rows;y++)
	 p->data[o++]=lookup_pal(pix[y][x],maxval);
      p->data[o++]=0;
      p->data[o++]=0xff; /* no more posts in this column */
   }
   fwrite(p,1,o,stdout);
}

void mkjpatch(void) {
   LE_int32 lebuf;
   int cols,rows,x,y,ph;
   pixval maxval;
   pixel **pix;
   loadpal();
   pix=ppm_readppm(stdin,&cols,&rows,&maxval);
   if(pix==NULL) err("bad ppm");
   putchar('J');
   putchar('1');
   putchar(mylog2(cols));
   putchar(ph=mylog2(rows));
   lebuf=cols;fwrite(&lebuf,sizeof(int),1,stdout);
   lebuf=rows;fwrite(&lebuf,sizeof(int),1,stdout);
   ph=1<<ph;
   for(x=0;x<cols;x++) for(y=0;y<ph;y++)
      if(y<rows) putchar(lookup_pal(pix[(rows-1)-y][(cols-1)-x],maxval));
      else putchar(255);
}

void mkpal(void) {
   int cols,rows,x,y,i=0;
   pixval maxval;
   pixel **pix;
   pix=ppm_readppm(stdin,&cols,&rows,&maxval);
   if(pix==NULL) err("bad ppm");
   for(y=0;y<rows;y++) for(x=0;x<cols;x++) {
      if(i>=256) break;
      putchar((unsigned char)(pix[y][x].r));
      putchar((unsigned char)(pix[y][x].g));
      putchar((unsigned char)(pix[y][x].b));
      i++;
   }
   while(i++<256) {
      putchar(0);
      putchar(0);
      putchar(0);
   }
}

static unsigned int pack_colour(unsigned int r,unsigned int g,unsigned int b)
{ 
   unsigned int p;
   p=b>>3;
   p|=(g>>2)<<5;
   p|=(r>>3)<<11;
   return p;
}

#define NCMAPS 30
void mkcmap(int bpp) {
   int i,j;
   if(bpp!=1&&bpp!=2&&bpp!=4) err("bad bpp value in mkcmap()");
   loadpal();
   for(i=0;i<=NCMAPS;i++) {
      for(j=0;j<256;j++) {
	 unsigned int r=playpal[3*j],g=playpal[3*j+1],b=playpal[3*j+2];
	 r=(r*(NCMAPS-i))/NCMAPS;
	 g=(g*(NCMAPS-i))/NCMAPS;
	 b=(b*(NCMAPS-i))/NCMAPS;
	 switch(bpp) {
	 case(1):
	    putchar(lookup_closest(r,g,b));
	    break;
	 case(2): {
	    unsigned int p=pack_colour(r,g,b);
	    putchar(p&0xff);
	    putchar(p>>8);
	    } break;
	 case(4):
	    putchar(b);putchar(g);putchar(r);putchar(0);
	    break;
	 }
      }
   }
}

int main(int argc, char **argv) {
   int i;
   for(i=1;i<argc;i++) {
      if(argv[i][0]!='-') usage();
      else switch(argv[i][1]) {
      case('M'): playpal_fn=argv[++i]; break;
      case('m'): mode=MkPlayPal; break;
      case('c'): mode=MkColormap; break;
      case('1'): bpp=1; break;
      case('2'): bpp=2; break;
      case('4'): bpp=4; break;
      case('f'): mode=MkFlat; break;
      case('p'): mode=MkPatch; break;
      case('P'): mode=MkPatch; break;
      case('i'): compat=1; break;
      default: usage();
      }
   }
   /* don't need to be interactive */
   setvbuf(stdout,NULL,_IOFBF,4096);
   setvbuf(stdin,NULL,_IOFBF,4096);
   /* do the conversion */
   switch(mode) {
   case(MkFlat): mkflat(); break;
   case(MkPatch): if(compat) mkpatch(); else mkjpatch(); break;
   case(MkPlayPal): mkpal(); break;
   case(MkColormap): mkcmap(bpp); break;
   case(NoMode): usage(); break;
   }
   return 0;
}




