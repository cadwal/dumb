#include <config.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <unistd.h>

#include "confeng.h"

/* #define CONF_DEBUG */

/* Find the ConfItem the name S refers to, and return its address or
 * NULL.  Parameter CONF points to an array of ConfModules, terminated
 * by name=NULL.  */
ConfItem *
conf_lookup_longname(const ConfModule *conf,const char *s)
{
   ConfItem *ci;
   size_t l=0;
   while(conf->name) {
      l=strlen(conf->name);
      if(strlen(s)>l&&s[l]=='-'&&!strncmp(conf->name,s,l)) break;
      conf++;
   }
   if(conf->name==NULL)
      return NULL;
   s+=l+1;
   for(ci=conf->items;ci->name;ci++)
      if(!strcmp(ci->name,s)) return ci;
   return NULL;
}

/* Find the ConfItem the character CH refers to, and return its
 * address or NULL.  Parameter CONF points to an array of ConfModules,
 * terminated by name=NULL.  */
ConfItem *
conf_lookup_shortname(const ConfModule *conf, char ch)
{
   while(conf->name) {
      ConfItem *ci=conf->items;
      while(ci->name) {
	 if(ci->shortname==ch) return ci;
	 ci++;
      }
      conf++;
   }
   return NULL;
}

static int
lookup_etype(const ConfEnum *ce,const char *s)
{
   int i;
   for(i=0;ce[i].name;i++)
      if(!strcasecmp(ce[i].name,s)) return i;
   /* FIXME: halt the program if given from cmdline */
   /* FIXME: stderr */
   printf("\"%s\" is not an option.  I'll use \"%s\" instead.\n",
	  s, ce[0].name);
   return 0;
}

/* CONF_TYPE_STR:
 * If ci->dirtlvl <= DIRT_ARGS, ci->strval points to a constant and
 * ci->maxlen doesn't matter.  Otherwise, ci->strval points to a
 * malloc(ci->maxlen) area that can be free()d.
 *
 * CONF_TYPE_LIST:
 * ci->listval may either be NULL or point to a realloc()able table of
 * pointers to strings.  The strings are allocated with strdup().
 * There is a NULL pointer marking the end of the table.
 */
void
set_conf(ConfItem *ci,char *val,int dirt)
{
#ifdef CONF_DEBUG
   printf("set_conf(%s,%s,%d)\n",ci->name,val?val:"<null>",dirt);
#endif
   switch (ci->type) {
   case CONF_TYPE_INT:
      /* fill in new intval */
      if (val) 
	 ci->intval = atoi(val);
      else {
	 /* FIXME: show module name too */
	 fprintf(stderr, "switch requires an argument -- %s\n",
		 ci->name);
	 return; /* without changing ci->dirtlvl */
      }
      break;
   case CONF_TYPE_BOOL:
      if (val)
	 ci->intval = atoi(val);
      else
	 ci->intval = 1;
      break;
   case CONF_TYPE_ENUM:
      /* deal with enums */
      if (val) {
	 if (isdigit(*val))
	    ci->intval = atoi(val);
	 else
	    ci->intval = lookup_etype(ci->etype, val);
      } else {
	 /* FIXME: show module name too */
	 fprintf(stderr, "switch requires an argument -- %s\n",
		 ci->name);
	 return; /* without changing ci->dirtlvl */
      }
      break;
   case CONF_TYPE_STR:
      /* fill in new strval */
      if(val&&dirt>DIRT_ARGS) {
	 if(ci->strval==NULL||ci->dirtlvl<=DIRT_ARGS) 
	    ci->strval=(char *)malloc(ci->maxlen);
	 if(!ci->strval) return;
	 strncpy(ci->strval,val,ci->maxlen-1);
	 ci->strval[ci->maxlen-1]=0;
      } else {
	 if(ci->strval&&ci->dirtlvl>DIRT_ARGS) 
	    free(ci->strval);
	 ci->strval=val;
      }
      break;
   case CONF_TYPE_LIST:
      /* fill in new listval */
      if(ci->listval==NULL) {
	 ci->listval=(char **)calloc(2,sizeof(char *));
	 if(val) ci->listval[0]=strdup(val);
      } else {
	 int n=0;
	 while(ci->listval[n]) n++;
	 ci->listval=(char **)realloc(ci->listval,(n+2)*sizeof(char *));
	 ci->listval[n]=strdup(val);
	 ci->listval[n+1]=(char *)NULL;
      }
      break;
   default:
      fprintf(stderr, "internal error: set_conf(): strange type %d\n",
	      ci->type);
      break;
   }
   /* fill in new dirtlevel */
   ci->dirtlvl=dirt;
}

void
conf_clear_list(ConfItem *ci)
{
   assert(ci->type == CONF_TYPE_LIST);
   if (ci->listval) {
      char **p;
      for (p = ci->listval; *p; p++)
	 free(*p);
   }
   free(ci->listval);
   ci->listval = NULL;
   ci->dirtlvl = DIRT_MODF;
}

int
conf_greatest_dirtlevel(const ConfModule *conf)
{
   ConfItem *ci;
   int maxdirt=0;
   for(;conf->name;conf++) for(ci=conf->items;ci->name;ci++)
      if(ci->dirtlvl>maxdirt) maxdirt=ci->dirtlvl;
   return maxdirt;
}

// Local Variables:
// c-basic-offset: 3
// End:
