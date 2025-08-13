#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display *dpy=NULL;
int screen;
Window root;
Pixmap pix;
GC gc;
XFontStruct *font;
int anti_alias=0;

void img2pgm(FILE *fout,XImage *img,int dx,int dy) {
   int x,y;
   fprintf(fout,"P5\n%d %d\n1\n",dx,dy);
   for(y=0;y<dy;y++) for(x=0;x<dx;x++)
      putc(XGetPixel(img,x,y),fout);
};
void img2pgm_aa(FILE *fout,XImage *img,int dx,int dy,int aaf) {
   int x,y;
   fprintf(fout,"P5\n%d %d\n%d\n",dx/aaf,dy/aaf,aaf);
   for(y=0;y<dy;y+=aaf) for(x=0;x<dx;x+=aaf) {
      int t=0,ix,iy;
      for(iy=0;iy<aaf;iy++) for(ix=0;ix<aaf;ix++)
	 t+=XGetPixel(img,x+ix,y+iy);
      if(t>aaf) putc(aaf,fout);
      else putc(t,fout);
   };
};

void do_char(FILE *fout,char ch) {      
   XImage *img;
   XCharStruct xcs;
   int ascent,descent,direction;
   XTextExtents(font,&ch,1,&direction,&ascent,&descent,&xcs);
   XDrawImageString(dpy,pix,gc,0,ascent,&ch,1);
   img=XGetImage(dpy,pix,
		 0,0,
		 xcs.width,ascent+descent,
		 XYPixmap,1);
   if(img==NULL) fprintf(stderr,"trouble with XGetImage\n");
   else if(anti_alias<2) img2pgm(stdout,img,xcs.width,ascent+descent);
   else img2pgm_aa(stdout,img,xcs.width,ascent+descent,anti_alias);
};

int main(int argc,char **argv) {
   const char *dpyname=NULL;

   if(argc<3||argc>4) {
      printf("usage: mkdfnt <font> <ch> [<aaf>]\n");
      return 2;
   };

   /* start Xlib */
   dpy=XOpenDisplay(dpyname);
   if(dpy==NULL) {
      printf("Can't open display %s",dpyname?dpyname:"<default>");
      return 1;
   };
   root=DefaultRootWindow(dpy);
   screen=DefaultScreen(dpy);
   pix=XCreatePixmap(dpy,root,320,320,1);
   gc=XCreateGC(dpy,pix,0,NULL);
   font=XLoadQueryFont(dpy,/*"-*-helvetica-medium-r-*-*-10-*"*/argv[1]);
   if(font==NULL) {
      fprintf(stderr,"font trouble\n");
      return 3;
   };
   XSetFont(dpy,gc,font->fid);
   XSetForeground(dpy,gc,1);
   XSetBackground(dpy,gc,0);

   //fprintf(stderr,"pix=%d font=%d\n",pix,font->fid);

   if(argc>=4) anti_alias=atoi(argv[3]);
   do_char(stdout,argv[2][0]);

   /* close Xlib */
   XFreePixmap(dpy,pix);
   XCloseDisplay(dpy);

   /* done */
   return 0;
};



