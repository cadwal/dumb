#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libdumbutil/log.h"
#include "libdumbwad/wadio.h"
#include "libdumbwad/wadwr.h"

int usage(void)  {
   printf("Usage:  wadtool <option> <file> <file>... <option> <file>...\n\n"
	  "Options:\n"
	  " -r <wadfile> : read this file as an IWAD\n"
	  " -p <wadfile> : read these files as PWADs\n"
	  " -w <wadfile> : start writing output to this PWAD\n"
	  " -W <wadfile> : start writing output to this IWAD\n"
	  " -D <dir>     : start writing output to this directory\n"
	  " -c <wadfile> : copy all lumps in this wadfile (without patching)\n"
	  " -l <lump>    : copy these lumps to the current wad\n"
	  " -L <lump>    : spit out an empty lump\n"
	  " -x <lump>    : copy these lumps to raw files\n"
	  /*" -X <s> <d>   : copy lump <s> to raw file <d>\n"*/
	  " -f <rawfile> : copy raw file to lump (guessing lumpname)\n"
	  " -F <rawfile> : copy raw file to lump (adding to current)\n"
	  " -n <s> <d>   : copy lump 's' to lump 'd'\n");
   exit(1);
}

typedef enum  {
   None, ReadWad, WriteIWad, WritePWad, WriteDir, PatchWad, CatWad, 
   WriteLump, RenameLump,
   WriteRaw, WriteRawCurrent, SpitLump, ExtractLump /*, ExtractLumpAs*/
} Mode;

void
catwad(WADWR *wr,FILE *fin)
{
   WadHeader hdr;
   WadDirEntry *dir;
   unsigned lumpnum;
   char namebuf[12];
   rewind(fin);
   fread(&hdr,sizeof(hdr),1,fin);
   dir=(WadDirEntry*)malloc(sizeof(WadDirEntry)*hdr.nlumps);
   if(dir==NULL) return;
   fseek(fin,hdr.diroffset,SEEK_SET);
   fread(dir,sizeof(WadDirEntry),hdr.nlumps,fin);
   for (lumpnum=0; lumpnum<hdr.nlumps; lumpnum++) {
      strncpy(namebuf, dir[lumpnum].name, 8);
      namebuf[8]=0;
      printf("copying %s...\n",namebuf);
      wadwr_lump(wr,namebuf);
      if (dir[lumpnum].size > 0) {
	 /* alright so this isn't a very efficient way of doing things! */
	 void *buf = malloc(dir[lumpnum].size);
	 fseek(fin, dir[lumpnum].offset, SEEK_SET);
	 fread(buf, dir[lumpnum].size, 1, fin);
	 wadwr_write(wr, buf, dir[lumpnum].size);
	 free(buf);
      }
   }
   free(dir);
}

int main(int argc,char **argv) {
   int i;
   LumpNum ln;
   Mode mode=None;
   WADWR *wr=NULL;
   log_stdout();
   if(argc<2) usage();
   for(i=1;i<argc;i++) {
      if(argv[i][0]=='-') switch(argv[i][1])  {
      case('r'): mode=ReadWad; break;
      case('p'): mode=PatchWad; break;
      case('W'): mode=WriteIWad; break;
      case('w'): mode=WritePWad; break;
      case('D'): mode=WriteDir; break;
      case('c'): mode=CatWad; break;
      case('l'): mode=WriteLump; break;
      case('x'): mode=ExtractLump; break;
	 /*case('X'): mode=ExtractLumpAs; break;*/
      case('n'): mode=RenameLump; break;
      case('f'): mode=WriteRaw; break;
      case('F'): mode=WriteRawCurrent; break;
      case('L'): mode=SpitLump; break;
      default: usage();
      }
      else switch(mode)  {
      case(ReadWad): init_iwad(argv[i], NULL); break;
      case(PatchWad): init_pwad(argv[i], NULL); break;
      case(WriteIWad): 
	 if(wr) wadwr_close(wr);
	 wr=wadwr_open(argv[i],'I');
	 break;
      case(WritePWad): 
	 if(wr) wadwr_close(wr);
	 wr=wadwr_open(argv[i],'P');
	 break;
      case(WriteDir): 
	 if(wr) wadwr_close(wr);
	 wr=wadwr_open(argv[i],'d');
	 break;
      case(WriteRaw):
	 if(wr) {
	    char buf[16];
	    char *s=strrchr(argv[i],'/');
	    if(s) s++;
	    else s=argv[i];
	    strncpy(buf,s,8);
	    buf[8]=0;
	    s=strrchr(buf,'.');
	    if(s) *s=0;
	    for(s=buf;*s;s++) *s=toupper(*s);
	    logprintf(LOG_INFO,'W',"copying %s to %s",argv[i],buf);
	    wadwr_lump(wr,buf);
	 }
	 /* nobreak */
      case(WriteRawCurrent):
	 if(wr) {
	    char buf[1024];
	    FILE *f=fopen(argv[i],"rb");
	    while(f&&!feof(f)) 
	       wadwr_write(wr,buf,fread(buf,1,1024,f));
	    if(f) fclose(f);
	 }
	 break;
      case(SpitLump):
	 if(wr) wadwr_lump(wr,argv[i]);
	 break;
      case(WriteLump):
	 ln=getlump(argv[i]);
	 if(wr&&LUMPNUM_OK(ln))  {
	    logprintf(LOG_INFO,'W',"copying %s...",argv[i]);
	    wadwr_lump(wr,argv[i]);
	    wadwr_write(wr,load_lump(ln),get_lump_len(ln));
	    free_lump(ln);
	 }
	 break;
      case(ExtractLump):
	 ln=getlump(argv[i]);
	 if(LUMPNUM_OK(ln))  {
	    char buf[16];
	    FILE *f;
	    strcpy(buf,argv[i]);
	    strcat(buf,".lump");
	    f=fopen(buf,"wb");
	    logprintf(LOG_INFO,'W',"copying %s to %s",argv[i],buf);
	    if(f) {
	       fwrite(load_lump(ln),get_lump_len(ln),1,f);
	       fclose(f);
	    }
	    free_lump(ln);
	 }
	 break;
      case(RenameLump):
	 ln=getlump(argv[i]);
	 if(wr&&LUMPNUM_OK(ln)&&i+1<argc)  {
	    logprintf(LOG_INFO,'W',"copying %s to %s...",argv[i],argv[i+1]);
	    wadwr_lump(wr,argv[i+1]);
	    wadwr_write(wr,load_lump(ln),get_lump_len(ln));
	    free_lump(ln);
	    i++;
	 }
	 break;
      case(CatWad): 
	 if(wr) {
	    FILE *fin=fopen(argv[i],"rb");
	    if(fin) {
	       catwad(wr,fin);
	       fclose(fin);
	    }
	 }
	 break;
      case(None): usage(); break;
      }
   }
   if(wr) wadwr_close(wr);
   reset_wad();
   return 0;
}
