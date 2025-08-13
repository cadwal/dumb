
/* this file included in texture.c */

/* ppot, Pixel, and pixcvt should be defined in parent file */

static void jpot(Texture *t,const AltPictData *pict,int xo,int yo) {
   Pixel *out=(Pixel *)t->texels;
   int x,y;
   int dx=pict->width,dy=pict->height;
   for(x=0;x<dx;x++) for(y=0;y<dy;y++) {
      int xt=x+(t->width)-pict->width-xo;
      int yt=y+(t->height)-pict->height-yo;
      unsigned char tpix; 
      if(xt<0||yt<0||xt>=t->width||yt>=t->height) continue;
      tpix=pict->data[ (x<<pict->log2height) + y ];
      if(tpix==0xff) continue;
      out[ (xt<<t->log2height) + yt ]= pixcvt(tpix);
   }
}

static void ppot(Texture *t,const PictData *pict,int xo,int yo) {
   int col,row,endrow;
   for(col=0;col<pict->UMEMB(hdr).width;col++) {
      const unsigned char *pd=pict->data+pict->UMEMB(hdr).idx[col];
      Pixel *out=(Pixel *)t->texels;
      out+=(t->width-(xo+col+1))<<t->log2height;
      out+=t->height-1;
      while(*pd!=0xff) {
	 row=yo+(*pd++);
         endrow=row+(*pd++);
         pd++; /* dummy byte */
         while(row<endrow) { 
	    if(row>=0&&row<t->height&&(xo+col)>=0&&(xo+col)<t->width) 
	      out[-row]=pixcvt(*pd);
	    row++;
	    pd++;
	 }
	 pd++; /* dummy byte */
      }
   }
}





