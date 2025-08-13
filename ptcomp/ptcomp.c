#include <config.h>

/* If you add/change keywords, please update the list at the bottom of
 * ../docs/README.ptcomp.  You don't have to write an explanation if
 * you don't want.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libdumbutil/log.h"

#include "token.h"
#include "globals.h"
#include "parm.h"
#include "animcomp.h"
#include "gettcomp.h"
#include "licomp.h"
#include "ltcomp.h"
#include "phasecomp.h"
#include "protocomp.h"
#include "soundcomp.h"

void
ptcomp(void)
{
   while(1) {
      const char *s=next_token();
      if(s==NULL) break;
      else if(*s=='\n');
      else if(!strcasecmp(s,"Proto")) protocomp();
      else if(!strcasecmp(s,"PhaseTable")) phasecomp();
      else if(!strcasecmp(s,"Gettable")) gettcomp();
      else if(!strcasecmp(s,"LineType")) ltcomp(0);
      else if(!strcasecmp(s,"SectorType")) ltcomp(1);
      else if(!strcasecmp(s,"SoundType")) soundcomp();
      else if(!strcasecmp(s,"AnimTexture")) animcomp(0);
      else if(!strcasecmp(s,"SwitchTexture")) animcomp(1);
      else if(!strcasecmp(s,"SwTexture")) animcomp(1);
      else if(!strcasecmp(s,"Level")) licomp();
      else if(!strcasecmp(s,"TimeUnits")) change_time_units();
      else if(!strcasecmp(s,"DefaultSpeed")) change_default_speed();
      else synerr(NULL);
   }
}

int
main(int argc,char **argv)
{
   FILE *fout;
   char foutname[256];
   int errors=0;
   if(argc!=2) {
      printf("Usage: ptcomp <dir>\n\n"
	     "ptcomp is used to compile .pt files into the PHASES,\n"
	     "PROTOS, and GETTABLE lumps used by DUMB.  It reads .pt\n"
	     "source from the standard input, and writes its output\n"
	     "into files of the form <dir>/PROTOS.lump\n"
	     "\n");
      return 1;
   }
   log_stdout(); /* for error messages from safem */
   init_animcomp();
   init_gettcomp();
   init_licomp();
   init_ltcomp();
   init_phasecomp();
   init_protocomp();
   init_soundcomp();

   begin_file(stdin, "stdin");
   printf("ptcomp: compiling...\n");
   ptcomp();
   end_file();
   /*fclose(fin);*/

   printf("ptcomp: writing...\n");
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/PHASES.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrphases(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/PROTOS.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrprotos(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/GETTABLE.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrgetts(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/LINETYPE.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrlts(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/SECTTYPE.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrsts(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/SOUNDS.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrsounds(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/ANIMTEX.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wranims(fout);
      fclose(fout);
   }
   fout=fopen(strcat(strcpy(foutname,argv[1]),"/LEVINFO.lump"),"wb");
   if (!fout)
      perror(foutname), errors++;
   else {
      wrlinfos(fout);
      fclose(fout);
   }
   return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}

// Local Variables:
// c-basic-offset: 3
// End:
