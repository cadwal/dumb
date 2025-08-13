/* DUMB: A Doom-like 3D game engine.
 *
 * test/ldltest.c: Test program for libdumbworldb (previously libdumblevel).
 * Copyright (C) 1998 by Kalle Niemitalo <tosi@stekt.oulu.fi>
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

#include <config.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "libdumbutil/dumb-nls.h"
#include "getopt.h"		/* ../libmissing/ */

#include "libdumbutil/bugaddr.h"
#include "libdumbutil/copyright.h"
#include "libdumbutil/log.h"
#include "libdumbutil/safem.h"
#include "libdumbwad/wadio.h"
#include "libdumbworldb/dumbworldb.h"

/* argv[0], for messages.  */
static const char *progname;

/* Set to 1 if --verbose is given.  */
static int verbose_flag = 0;

/* The name given with --map=MAP.  */
const char *mapname = "E1M1";

/* Skill level from --difficulty=N.  1=easy, 5=hard, -1=unspecified.  */
#define ALL_DIFFICULTIES -1
int difficulty = ALL_DIFFICULTIES;

/* Because getopt_long() gives us all the options before the filenames
 * are seen, the operations are collected to a linked list and
 * performed when the files have been loaded.  */
enum operation {
   OP_STATISTICS,
   OP_SECTOR_SIDES,
   OP_SECTOR_THINGS,
   OP_THING_SECTORS,
   OP_OUTPUT_DOOM,
   OP_SET_NAME,
   OP_SET_LONGNAME
};

struct op_node {
   enum operation operation;
   union {
      struct {
	 int sector;
      } sector_sides, sector_things;
      struct {
	 int thing;
      } thing_sectors;
      struct {
	 const char *filename;
      } output_doom;
      struct {
	 const char *name;
      } set_name, set_longname;
   } u;
   struct op_node *next;
};
struct op_node *first_op = NULL, *last_op = NULL;

static void print_help(void);
static void print_version(void);
static void parse_args(int argc, char **argv);
static long parse_long(const char *s, const char *errmsg);
static int parse_int(const char *s, const char *errmsg);
static struct op_node *queue_op(enum operation);
static void perform_op(struct dwdb_level *, const struct op_node *);
static void show_level_statistics(const struct dwdb_level *);
static void show_alloc(const struct dwdb_alloc *);
static void list_sector_sides(const struct dwdb_level *, int sector);
static void list_sector_things(const struct dwdb_level *, int sector);
static void list_thing_sectors(const struct dwdb_level *, int thing);

int
main(int argc, char **argv)
{
#ifdef ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
#endif /* ENABLE_NLS */
   progname = argv[0];
   log_stream(stderr, LOG_ERROR, NULL);	/* for errors from libdumbwad */
   parse_args(argc, argv);
   {
      struct dwdb_level level;
      if (verbose_flag)
	 printf(_("%s: initializing map\n"), progname);
      dwdb_init_doom(&level, mapname);
      if (difficulty != ALL_DIFFICULTIES) {
	 if (verbose_flag)
	    printf(_("%s: setting difficulty\n"), progname);
	 dwdb_set_difficulty(&level, difficulty);
      } {
	 struct op_node *node;
	 for (node = first_op; node; node = node->next)
	    perform_op(&level, node);
      }
      dwdb_fini(&level);
   }
   return 0;
}

static void
print_help(void)
{
   printf(_("Usage: %s [OPTION]... IWAD [PWAD]...\n"
	    "Load a map and operate on it.\n"
	    "\n"), progname);
   printf(_("  Settings:\n"
	    "  -m, --map=NAME             which map to load (default %s)\n"
	    "      --difficulty=N         1=easy ... 5=hard (default all)\n"
	    "      --verbose              explain what is being done\n"),
	  mapname);
   fputs(_( "  Operations:\n"
	    "      --statistics           display statistics on the whole map\n"
	    "      --sector-sides=SECTOR  list sides in SECTOR\n"
	    "      --sector-things=SECTOR list things in SECTOR\n"
	    "      --thing-sectors=THING  list sectors THING is in\n"
	    "      --set-name=NAME        set the name of the map to NAME\n"
	    "      --set-long-name=NAME   set the long name of the map to NAME\n"
	    "      --output-doom=FILE     save the map to FILE in Doom format.\n"
	    "                             .wad won't be appended automatically.\n"), stdout);
   fputs(_( "  Other:\n"
	    "      --help                 display this help and exit\n"
	    "      --version              output version information and exit\n"
	    "\n"), stdout);
   print_bugaddr_message(stdout);
}

static void
print_version(void)
{
   static const struct copyright copyrights[] = {
      { "1998", "Kalle Niemitalo" },
      COPYRIGHT_END
   };
   fputs("ldltest (DUMB) " VERSION "\n", stdout);
   print_copyrights(copyrights);
   fputs(_("This program is free software; you may redistribute it under the terms of\n"
	   "the GNU General Public License.  This program has absolutely no warranty.\n"),
	 stdout);
}

static void
parse_args(int argc, char **argv)
{
   static const struct option long_opts[] =
   {
      {"map", required_argument, NULL, 'm'},
      {"difficulty", required_argument, NULL, 'd'},
      {"verbose", no_argument, &verbose_flag, 1},
      {"statistics", no_argument, NULL, 'i'},
      {"sector-sides", required_argument, NULL, 's'},
      {"sector-things", required_argument, NULL, 't'},
      {"thing-sectors", required_argument, NULL, 'T'},
      {"set-name", required_argument, NULL, 'n'},
      {"set-long-name", required_argument, NULL, 'l'},
      {"output-doom", required_argument, NULL, 'o'},
      {"help", no_argument, NULL, 1},
      {"version", no_argument, NULL, 2},
      {0, 0, 0, 0}
   };
   int index;			/* in argv[], used for filenames */
   while (1) {
      int c = getopt_long(argc, argv, "m:", long_opts, NULL);
      if (c == -1)
	 break;			/* end of options */
      switch (c) {
      case 0:			/* option just set a flag */
	 break;
      case 'm':		/* --map=NAME */
	 mapname = optarg;
	 break;
      case 'd':		/* --difficulty=N */
	 if (difficulty != ALL_DIFFICULTIES) {
	    fprintf(stderr, _("%s: --difficulty given twice\n"), progname);
	    exit(EXIT_FAILURE);
	 }
	 difficulty = parse_int(optarg, _("%s: invalid difficulty `%s'\n"));
	 if (difficulty < 1 || difficulty > 5) {
	    fprintf(stderr, _("%s: difficulty must be between 1 and 5\n"),
		    progname);
	    exit(EXIT_FAILURE);
	 }
	 break;
      case 'i':		/* --statistics */
	 queue_op(OP_STATISTICS);
	 break;
      case 's':		/* --sector-sides=SECTOR */
	 queue_op(OP_SECTOR_SIDES)->u.sector_sides.sector
	     = parse_int(optarg, _("%s: invalid sector number `%s'\n"));
	 /* The number will be checked more extensively after the
	  * level has been loaded.  */
	 break;
      case 't':		/* --sector-things=SECTOR */
	 queue_op(OP_SECTOR_THINGS)->u.sector_things.sector
	     = parse_int(optarg, _("%s: invalid sector number `%s'\n"));
	 /* The number will be checked more extensively after the
	  * level has been loaded.  */
	 break;
      case 'T':		/* --thing-sectors=THING */
	 queue_op(OP_THING_SECTORS)->u.thing_sectors.thing
	     = parse_int(optarg, _("%s: invalid thing number `%s'\n"));
	 /* The number will be checked more extensively after the
	  * level has been loaded.  */
	 break;
      case 'n':		/* --set-name=NAME */
	 queue_op(OP_SET_NAME)->u.set_name.name = optarg;
	 break;
      case 'l':		/* --set-long-name=NAME */
	 queue_op(OP_SET_LONGNAME)->u.set_longname.name = optarg;
	 break;
      case 'o':		/* --output-doom=FILE */
	 queue_op(OP_OUTPUT_DOOM)->u.output_doom.filename = optarg;
	 break;
      case 1:			/* --help */
	 print_help();
	 exit(EXIT_SUCCESS);
      case 2:			/* --version */
	 print_version();
	 exit(EXIT_SUCCESS);
      case '?':
	 /* getopt_long() printed the error message already */
	 fprintf(stderr, _("Try `%s --help' for more information.\n"),
		 progname);
	 exit(EXIT_FAILURE);
      default:
	 abort();
      }
   }
   index = optind;
   if (index < argc) {
      if (verbose_flag)
	 printf(_("%s: opening files\n"), progname);
      init_iwad(argv[index++], NULL);
   } else {
      fprintf(stderr, _("%s: missing IWAD argument\n"), progname);
      exit(EXIT_FAILURE);
   }
   while (index < argc)
      init_pwad(argv[index++], NULL);
}

static long
parse_long(const char *s, const char *errmsg)
{
   char *tail;
   long val = strtol(s, &tail, 0);
   if (*tail || tail == s) {
      fprintf(stderr, errmsg, progname, s);
      exit(EXIT_FAILURE);
   }
   return val;
}

static int
parse_int(const char *s, const char *errmsg)
{
   long val = parse_long(s, errmsg);
   if (val < INT_MIN || val > INT_MAX) {
      fprintf(stderr, errmsg, progname, s);
      exit(EXIT_FAILURE);
   }
   return (int) val;
}

static struct op_node *
queue_op(enum operation op)
{
   struct op_node *node = (struct op_node *)
      safe_malloc(sizeof(struct op_node));
   node->operation = op;
   node->next = NULL;
   if (!first_op)
      first_op = node;
   if (last_op)
      last_op->next = node;
   last_op = node;
   return node;
}

static void
perform_op(struct dwdb_level *level, const struct op_node *node)
{
   switch (node->operation) {
   case OP_STATISTICS:
      show_level_statistics(level);
      break;
   case OP_SECTOR_SIDES:
      list_sector_sides(level, node->u.sector_sides.sector);
      break;
   case OP_SECTOR_THINGS:
      list_sector_things(level, node->u.sector_things.sector);
      break;
   case OP_THING_SECTORS:
      list_thing_sectors(level, node->u.thing_sectors.thing);
      break;
   case OP_SET_NAME:
      dwdb_set_name(level, node->u.set_name.name);
      break;
   case OP_SET_LONGNAME:
      dwdb_set_longname(level, node->u.set_longname.name);
      break;
   case OP_OUTPUT_DOOM:
      dwdb_save_doom(level, node->u.output_doom.filename);
      break;
   default:
      abort();
   }
}

static void
show_level_statistics(const struct dwdb_level *level)
{
   printf(_("Statistics for level %s:"), level->name);
   fputs(_("\n\tVertices: "), stdout);
   show_alloc(&level->vertex_alloc);
   fputs(_("\n\tSectors: "), stdout);
   show_alloc(&level->sector_alloc);
   fputs(_("\n\tSides: "), stdout);
   show_alloc(&level->side_alloc);
   fputs(_("\n\tLines: "), stdout);
   show_alloc(&level->line_alloc);
   fputs(_("\n\tThings: "), stdout);
   show_alloc(&level->thing_alloc);
   putchar('\n');
}

static void
show_alloc(const struct dwdb_alloc *alloc)
{
   printf(_("%u alloced, %u inited, %u free"),
	  alloc->alloced, alloc->inited, alloc->free);
}

static void
list_sector_sides(const struct dwdb_level *level, int sector)
{
   int side;
   if (!dwdb_isok_sector(level, sector)) {
      fprintf(stderr, _("%s: sector number %d doesn't exist\n"),
	      progname, sector);
      exit(EXIT_FAILURE);
   }
   printf(_("Sides in sector %d:\n"), sector);
   for (side = level->sectors[sector].first_side;
	side != -1;
	side = level->sides[side].next_in_chain) {
      int line = level->sides[side].line;
      struct dwdb_line *line_ptr = &level->lines[line];
      int opp_side = (line_ptr->side[0] == side
		      ? line_ptr->side[1]
		      : line_ptr->side[0]);
      int opp_sector = (opp_side == -1
			? -1
			: level->sides[opp_side].sector);
      if (opp_sector == -1)
	 printf(_("\tside %5d, line %5d\n"),
		side, line);
      else
	 printf(_("\tside %5d, line %5d, opposite sector %5d\n"),
		side, line, opp_sector);
   }
}

static void
list_sector_things(const struct dwdb_level *level, int sector)
{
   const struct dwdb_thingsec *thingsec;
   if (!dwdb_isok_sector(level, sector)) {
      fprintf(stderr, _("%s: sector number %d doesn't exist\n"),
	      progname, sector);
      exit(EXIT_FAILURE);
   }
   printf(_("Things in sector %d:"), sector);
   for (thingsec = level->sectors[sector].things;
	thingsec;
	thingsec = thingsec->next_thing)
      printf(" %d", thingsec->thing);
   putchar('\n');
}

static void
list_thing_sectors(const struct dwdb_level *level, int thing)
{
   const struct dwdb_thingsec *thingsec;
   if (!dwdb_isok_thing(level, thing)) {
      fprintf(stderr, _("%s: thing number %d doesn't exist\n"),
	      progname, thing);
      exit(EXIT_FAILURE);
   }
   printf(_("Sectors thing %d is in:"), thing);
   for (thingsec = level->things[thing].sectors;
	thingsec;
	thingsec = thingsec->next_sector)
      printf(" %d", thingsec->sector);
   putchar('\n');
}

// Local Variables:
// c-basic-offset: 3
// End:
