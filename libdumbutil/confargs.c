#include <config.h>

#include <stdlib.h>
#include <ctype.h>

#include "confargs.h"
#include "confeng.h"
#include "confhelp.h"

/* parse argument list in argc,argv using array of modules conf */ 
int conf_args(const ConfModule *conf,int argc,char **argv) {
   int i;
   ConfItem *ci=NULL;
   for(i=1;i<argc;i++) {
      if(argv[i][0]!='-'||isdigit(argv[i][1])) {
	 if(ci) set_conf(ci,argv[i],DIRT_ARGS);
	 else return conf_usage(conf,argv[0],argv[i]);
      } else if(argv[i][1]=='-') {
	 ci=conf_lookup_longname(conf,argv[i]+2);
	 if(!ci) return conf_usage(conf,argv[0],argv[i]);
	 if(ci->type==CONF_TYPE_BOOL) 
	    set_conf(ci,NULL,DIRT_ARGS);
      } else if(argv[i][1]=='?') {
	 if(argc>i+1) return conf_help(conf,argv[0],argv[i+1]);
	 else return conf_usage(conf,argv[0],NULL);
      } else {
	 ci=conf_lookup_shortname(conf,argv[i][1]);
	 if(!ci) return conf_usage(conf,argv[0],argv[i]);
	 if(argv[i][2]) set_conf(ci,argv[i]+2,DIRT_ARGS);
	 else if(ci->type==CONF_TYPE_BOOL) set_conf(ci,NULL,DIRT_ARGS);
      }
   }
   return 0;
}
