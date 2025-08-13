#include <config.h>

#include <stdio.h>
#include <string.h>

#include "confhelp.h"
#include "confdef.h"
#include "confeng.h"

static void swhelp(ConfItem *ci,const char *mod);
static void modhelp(const ConfModule *conf);
static void pr_etype(const ConfEnum *ce);

int 
conf_help(const ConfModule *conf,const char *prog,const char *mod)
{
   conf_usage(conf,prog,NULL);
   if(mod[0]=='-') { 
      /* help with a switch */
      ConfItem *ci;
      if(mod[1]=='?') return 1;
      else if(mod[1]=='-') ci=conf_lookup_longname(conf,mod+2);
      else ci=conf_lookup_shortname(conf,mod[1]);
      if(ci) swhelp(ci,mod);
      else printf("`%s' is not a switch I recognise\n",mod);
   } else {
      /* help with a module */
      if(!strcasecmp(mod,"all")) mod=NULL;
      while(conf->name) {
	 if(mod==NULL||!strcasecmp(conf->name,mod)) {
	    modhelp(conf);
	    putchar('\n');
	 }
	 conf++;
      }
   }
   return 1;
}

int
conf_usage(const ConfModule *conf,const char *prog,const char *err)
{
   if(err) printf("I don't understand your command line.  The token which "
		  "is causing trouble\nis `%s'.\n\n",err);
   printf("usage: %s [switch [parameter]] [switch [parameter]]...\n\n",prog);
   printf("For a list of all switches available, invoke %s -? all\n\n",prog);
   printf("Or, for switches relating to a particular module, %s -? <module>\n"
	  "where module is one of: all",prog);
   while(conf->name) {
      printf(", %s",conf->name);
      conf++;
   }
   printf("\n\nFor information on a particular switch: %s -? <switch>\n\n",
	  prog);
   return 1;
}

static void
modhelp(const ConfModule *conf)
{
   const ConfItem *it=conf->items;
   printf("Module `%s': %s\n",conf->name,conf->desc);
   if(!it->name) printf("<no configurable items>\n");
   else while(it->name) {
      printf("--%s-%s",conf->name,it->name);
      switch (it->type) {
      case CONF_TYPE_INT: printf(" <integer>"); break;
      case CONF_TYPE_BOOL: /* no-op */ break;
      case CONF_TYPE_ENUM: printf(" <enum>"); break;
      case CONF_TYPE_STR: printf(" <string>"); break;
      case CONF_TYPE_LIST: printf(" <list>"); break;
      default: 
	 printf(" <internal error: strange type %d>", (int) it->type);
	 break;
      }
      if(it->help) printf(": %s",it->help);
      if(it->shortname) printf(" (short form -%c)",it->shortname);
      putchar('\n');
      it++;
   }
}

static void
swhelp(ConfItem *ci,const char *mod)
{
   printf("Help for %s:\n  `%s'\n",mod,ci->help);
   switch (ci->type) {
   case CONF_TYPE_INT:
      printf("  Type=%s\n","Integer");
      printf("  Default=%d\n",ci->intval);
      break;
   case CONF_TYPE_BOOL:
      printf("  Type=%s\n","Boolean");
      printf("  Default=%d\n",ci->intval);
      break;
   case CONF_TYPE_ENUM:
      printf("  Type=%s\n","Enumerated");
      printf("  Default=%s\n",ci->etype[ci->intval].name);
      printf("  Values=");
      pr_etype(ci->etype);
      putchar('\n');
      break;
   case CONF_TYPE_STR:
      printf("  Type=%s\n","String");
      printf("  Default=%s\n",ci->strval);
      break;
   case CONF_TYPE_LIST:
      printf("  Type=%s\n","List");
      break;
   default:
      printf("  Type=%d (internal error)\n", (int) ci->type);
      break;
   }
   if(ci->shortname) printf("  Short Form=-%c\n",ci->shortname);
   if(ci->maxlen) printf("  Maxlen=%d\n",(int)(ci->maxlen));
   if(ci->flags&CI_NOSAVE) printf("  Cannot be saved in a config file.\n");
}

static void
pr_etype(const ConfEnum *ce)
{
   while(ce->name) {
      printf("%s",ce->name);
      ce++;
      if(ce->name) putchar(',');
   }
}

// Local Variables:
// c-basic-offset: 3
// End:
