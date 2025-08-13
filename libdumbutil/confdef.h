/* DUMB: A Doom-like 3D game engine.
 *
 * libdumbutil/confdef.h: How to define configuration items.
 * Copyright (C) 1998 by Josh Parsons <josh@coombs.anu.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111,
 * USA.
 */

#ifndef CONFDEF_H
#define CONFDEF_H

typedef struct {
   const char *name;
   long value;
} ConfEnum;

enum conf_type {
   CONF_TYPE_INT,
   CONF_TYPE_BOOL,
   CONF_TYPE_ENUM,
   CONF_TYPE_STR,
   CONF_TYPE_LIST
};

typedef struct {
   const char *name, *menuname;	/* will be translated when used */
   char shortname;
   const char *help;
   enum conf_type type;
   const ConfEnum *etype;	/* enum */
   unsigned char maxlen;	/* str -- remove? */
   char flags;
   /* runtime-changing section -- the following macros linewrap here */
   int intval;			/* int, bool, enum */
   char *strval;		/* str */
   char **listval;		/* list */
   char dirtlvl;
} ConfItem;

#define CONFI(n,mn,sn,h,df) {n,mn,sn,h,CONF_TYPE_INT,NULL,0,0, \
    df,NULL,NULL,DIRT_NONE}	/* integer(df) */
#define CONFB(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_BOOL,NULL,0,0, \
    0,NULL,NULL,DIRT_NONE}	/* boolean(false) */
#define CONFB_1(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_BOOL,NULL,0,0, \
    1,NULL,NULL,DIRT_NONE}	/* boolean(true) */
#define CONFB_NS(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_BOOL,NULL,0,CI_NOSAVE, \
    0,NULL,NULL,DIRT_NONE}	/* boolean(false), no save */
#define CONFE(n,mn,sn,h,df,e) {n,mn,sn,h,CONF_TYPE_ENUM,e,0,0, \
    df,NULL,NULL,DIRT_NONE}	/* enum(df) */
#define CONFS_L(n,mn,sn,h,df,l) {n,mn,sn,h,CONF_TYPE_STR,NULL,l,0, \
    0,df,NULL,DIRT_NONE}	/* string(df), with maximum length */
#define CONFS(n,mn,sn,h,df) {n,mn,sn,h,CONF_TYPE_STR,NULL,254,0, \
    0,df,NULL,DIRT_NONE}	/* string(df) */
#define CONFL(n,mn,sn,h) {n,mn,sn,h,CONF_TYPE_LIST,NULL,0,0, \
    0,NULL,NULL,DIRT_NONE}	/* list({}) */

/* the first member is what matters, but listing them all avoids
   warnings with gcc -b i486-linux -V egcs-2.91.60 -W */
#define CONFITEM_END {NULL,NULL,0,NULL,CONF_TYPE_INT,NULL,0,0,0,NULL,NULL,0}
#define CONFENUM_END {NULL,0}

typedef struct {
   ConfItem *items;
   const char *name, *desc;
} ConfModule;

/* ConfItem::dirtlvl */
#define DIRT_NONE 0		/* value is built-in default */
#define DIRT_ARGS 1		/* value was specified on the command line */
#define DIRT_FILE 2		/* value loaded from config file */
#define DIRT_MODF 3		/* value modified (eg. from menu) */

/* ConfItem::flags */
#define CI_NOSAVE 0x01

#endif

// Local Variables:
// c-basic-offset: 3
// End:
