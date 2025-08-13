#include <stdio.h>
#include <stdlib.h>

#include <ppm.h>

int main(int argc,char **argv) {
   pixel **pix;
   int cols,rows;
   pixval maxval;
   int x,y,minval;
   if(argc!=2) {
      fprintf(stderr,"usage: dark2trans <minimum pixel brightness>\n\n");
      return 1;
   };
   minval=atoi(argv[1])*3;
   pix=ppm_readppm(stdin,&cols,&rows,&maxval);
   for(y=0;y<rows;y++) for(x=0;x<cols;x++) {
      pixel *p=pix[y]+x;
      if(p->r+p->g+p->b<minval) {
	 p->r=p->g=0;
	 p->b=1;
      };
   };
   ppm_writeppm(stdout,pix,cols,rows,maxval,0);
   return 0;
};
