/* This file was modified in 2000-2001 by David Gay for the RC
   compiler, and John Kodumal for banshee-pta.  The changes are
   Copyright (c) 2000-2001 The Regents of the University of
   California.

   This file is distributed under the terms of the GNU General Public License
   (see below).
*/
/* Top level of GNU C compiler
   Copyright (C) 1987, 88, 89, 92-7, 1998 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <signal.h>
#include <unistd.h>
#include <glob.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "parser.h"
#include "input.h"
#include "c-parse.h"
#include "unparse.h"
#include "semantics.h"
#include "rc.h"
#include "analysis.h"

// extern long get_memusage(void);
extern int banshee_get_time();
extern void banshee_backtrack(int);
extern region *get_persistent_regions();

/* Name of program invoked, sans directories.  */
char *progname;

/* Copy of arguments to main, without the filename.  */
int copy_argc;
char **copy_argv;


/* -f flags.  */

/* Nonzero means all references through pointers are volatile.  */
int flag_volatile;

/* Nonzero means treat all global and extern variables as global.  */
int flag_volatile_global;

/* Nonzero means just do syntax checking; don't output anything.  */
int flag_syntax_only;

/* Nonzero means change certain warnings into errors.
   Usually these are warnings about failure to conform to some standard.  */
int flag_pedantic_errors;

/* Tag all structures with __attribute__(packed) */
int flag_pack_struct;

/* Options controlling warnings */

/* Don't print warning messages.  -w.  */
int inhibit_warnings;

/* Print various extra warnings.  -W.  */
int extra_warnings;

/* Treat warnings as errors.  -Werror.  */
int warnings_are_errors;

/* Nonzero to warn about unused local variables.  */
int warn_unused;

/* Nonzero to warn about variables used before they are initialized.  */
int warn_uninitialized;

/* Nonzero means warn about all declarations which shadow others.   */
int warn_shadow;
int error_shadow; /* Make shadow an error */

/* Warn if a switch on an enum fails to have a case for every enum value.  */
int warn_switch;

/* Nonzero means warn about function definitions that default the return type
   or that use a null return and have a return-type other than void.  */
int warn_return_type;

/* Nonzero means warn about pointer casts that increase the required
   alignment of the target type (and might therefore lead to a crash
   due to a misaligned access).  */
int warn_cast_align;

/* Nonzero means warn about any identifiers that match in the first N
   characters.  The value N is in `id_clash_len'.  */
int warn_id_clash;
unsigned id_clash_len;

/* Nonzero means warn about any objects definitions whose size is larger
   than N bytes.  Also want about function definitions whose returned
   values are larger than N bytes. The value N is in `larger_than_size'.  */
int warn_larger_than;
unsigned larger_than_size;

/* Nonzero means warn if inline function is too large.  */
int warn_inline;

/* Warn if a function returns an aggregate,
   since there are often incompatible calling conventions for doing this.  */
int warn_aggregate_return;

/* Nonzero means `$' can be in an identifier.  */
int dollars_in_ident;

/* Nonzero means allow type mismatches in conditional expressions;
   just make their values `void'.   */
int flag_cond_mismatch;

/* Nonzero means don't recognize the keyword `asm'.  */
int flag_no_asm;

/* Nonzero means warn about implicit declarations.  */
int warn_implicit;

/* Nonzero means give string constants the type `const char *'
   to get extra warnings from them.  These warnings will be too numerous
   to be useful, except in thoroughly ANSIfied programs.  */
int warn_write_strings;

/* Nonzero means warn about sizeof (function) or addition/subtraction
   of function pointers.  */
int warn_pointer_arith;

/* Nonzero means warn for all old-style non-prototype function decls.  */
int warn_strict_prototypes;

/* Nonzero means warn about multiple (redundant) decls for the same single
   variable or function.  */
int warn_redundant_decls;

/* Nonzero means warn about extern declarations of objects not at
   file-scope level and about *all* declarations of functions (whether
   extern or static) not at file-scope level.  Note that we exclude
   implicit function declarations.  To get warnings about those, use
   -Wimplicit.  */
int warn_nested_externs;

/* Nonzero means warn about pointer casts that can drop a type qualifier
   from the pointer target type.  */
int warn_cast_qual;

/* Nonzero means warn when casting a function call to a type that does
   not match the return type (e.g. (float)sqrt() or (anything*)malloc()
   when there is no previous declaration of sqrt or malloc.  */
int warn_bad_function_cast;

/* Warn about traditional constructs whose meanings changed in ANSI C.  */
int warn_traditional;

/* Warn about *printf or *scanf format/argument anomalies. */
int warn_format;

/* Warn about a subscript that has type char.  */
int warn_char_subscripts;

/* Warn if a type conversion is done that might have confusing results.  */
int warn_conversion;

/* Warn if main is suspicious. */
int warn_main;

/* Nonzero means warn about use of multicharacter literals.  */
int warn_multichar = 1;

/* Nonzero means do some things the same way PCC does.  */
int flag_traditional;

/* Nonzero means to allow single precision math even if we're generally
   being traditional. */
int flag_allow_single_precision;

/* Nonzero means warn about suggesting putting in ()'s.  */
int warn_parentheses;

/* Warn if initializer is not completely bracketed.  */
int warn_missing_braces;

/* Warn about comparison of signed and unsigned values.  */
int warn_sign_compare;

/* Nonzero means message about use of implicit function declarations;
 1 means warning; 2 means error. */
int mesg_implicit_function_declaration;

bool pedantic; /* Report pedantic warnings if true */

/* Nonzero means warn about use of implicit int. */
int warn_implicit_int;

/* Nonzero means warn for any global function def
   without separate previous prototype decl.  */
int warn_missing_prototypes;

/* Nonzero means warn for any global function def
   without separate previous decl.  */
int warn_missing_declarations;

/* Don't print functions as they are compiled and don't print
   times taken by the various passes.  -quiet.  */
int quiet_flag;

/* Nonzero means don't run real cc1 afterwards */
int flag_parse_only;

/* Nonzero means `char' should be signed.  */
int flag_signed_char;

/* Nonzero means give an enum type only as many bytes as it needs.  */
int flag_short_enums;

/* Nonzero means to treat bitfields as signed unless they say `unsigned'.  */
int flag_signed_bitfields = 1;
int explicit_flag_signed_bitfields = 0;

/* Algorithm to use for placement of RC operations on local pointers */
enum rc_algorithm_type rc_algorithm = rc_functions;

/* Nonzero to rc locals and/or globals */
int rc_locals = 1;
int rc_globals = 1;

/* Nonzero to disable RC operations that are just checks */
int rc_nochecks = 0;
/* Nonzero to never remove RC checks */
int rc_keepchecks = 0;

/* Nonzero to collect statistics (statically and dynamically) */
int rc_collect_statistics = 0, rc_collect_statistics_end = 0;

/* Nonzero to collect detailed statistics */
int rc_stats_detailed = 0;

/* Nonzero to collect RC costs using the cycle counter */
int rc_stats_costs = 0;

/* Nonzero to keep RCs between each pair of regions (makes
   region deletion cheap) */
int rc_pairs = 0;
int rc_pairs_from = 0; /* RCs in source rather than dest */
int rc_pairs_array = 0; /* RCs in separate array */

/* Nonzero if all functions should be assumed to delete regions */
int rc_nodeletes = 0;

/* Nonzero to perform check (samregion, traditional) optimisation */
int rc_chkopt = 0;

/* Nonzero to perform check optimisation on a per-file basis */
int rc_chkopt_perfile = 0;

/* Nonzero for "safe" rc_adjust functions which allow deleteregion_array
   to work with non-pair region libraries.
   Also enabled when not using rc_update_xxx */
int rc_safe_adjustfn = USE_RC_ADJUST_FOR_UPDATE;

/* points-to related flags */
int flag_pta_profile = 1;
int flag_print_constraints = 0;
int flag_print_empty = 0;
int flag_print_results = 0;
int flag_print_stats = 0;
int flag_points_to = 1;
int flag_print_vars = 0;
int flag_print_graph = 0;
int flag_model_strings = 1;
int flag_print_memusage = 0;
int flag_debug_backtrack = 0;
int flag_debug_region_serialization = 0;
int flag_debug_region_deserialization = 0;
int flag_backtrack_constraints = 0;
int flag_serialize_constraints = 0;
int flag_deserialize_constraints = 0;
int debug_backtrack_prefix = 0;
static int debug_backtrack_time = 0;
static int backtrack_time = 0;

/* Table of language-independent -f options.
   STRING is the option name.  VARIABLE is the address of the variable.
   ON_VALUE is the value to store in VARIABLE
    if `-fSTRING' is seen as an option.
   (If `-fno-STRING' is seen as an option, the opposite value is stored.)  */

struct { char *string; int *variable; int on_value;} f_options[] =
{
  {"volatile", &flag_volatile, 1},
  {"volatile-global", &flag_volatile_global, 1},
  {"syntax-only", &flag_syntax_only, 1},
  {"parse-only", &flag_parse_only, 1},
  {"pack-struct", &flag_pack_struct, 1},
  {"profile", &flag_pta_profile,1},
  {"print-constraints", &flag_print_constraints,1},
  {"print-empty",&flag_print_empty,1},
  {"print-results",&flag_print_results,1},
  {"print-stats",&flag_print_stats,1},
  {"points-to",&flag_points_to,1},
  {"serialize-constraints",&flag_serialize_constraints,1},
  {"debug-region-serialization", &flag_debug_region_serialization,1},
  {"debug-region-deserialization", &flag_debug_region_deserialization,1},
  {"deserialize-constraints",&flag_deserialize_constraints,1},
#ifndef ANDERSEN_ST
  {"cycle_elim",(int*)&flag_eliminate_cycles,1},
  {"proj-merge",(int*)&flag_merge_projections,1},
#endif
  {"print-vars",&flag_print_vars,1},
  {"print-graph",&flag_print_graph,1},
  {"model-strings",&flag_model_strings,1},
  {"print-memusage",&flag_print_memusage,1},
  {"field-based",&flag_field_based,1}
};

/* Table of language-specific options.  */

char *lang_options[] =
{
  "-ansi",
  "-fallow-single-precision",

  "-fsigned-bitfields",
  "-funsigned-bitfields",
  "-fno-signed-bitfields",
  "-fno-unsigned-bitfields",
  "-fsigned-char",
  "-funsigned-char",
  "-fno-signed-char",
  "-fno-unsigned-char",

  "-ftraditional",
  "-traditional",
  "-fnotraditional",
  "-fno-traditional",

  "-fasm",
  "-fno-asm",
  "-fbuiltin",
  "-fno-builtin",
  "-fhosted",
  "-fno-hosted",
  "-ffreestanding",
  "-fno-freestanding",
  "-fcond-mismatch",
  "-fno-cond-mismatch",
  "-fdollars-in-identifiers",
  "-fno-dollars-in-identifiers",
  "-fident",
  "-fno-ident",
  "-fshort-double",
  "-fno-short-double",
  "-fshort-enums",
  "-fno-short-enums",

  "-Wall",
  "-Wbad-function-cast",
  "-Wno-bad-function-cast",
  "-Wcast-qual",
  "-Wno-cast-qual",
  "-Wchar-subscripts",
  "-Wno-char-subscripts",
  "-Wcomment",
  "-Wno-comment",
  "-Wcomments",
  "-Wno-comments",
  "-Wconversion",
  "-Wno-conversion",
  "-Wformat",
  "-Wno-format",
  "-Wimport",
  "-Wno-import",
  "-Wimplicit-function-declaration",
  "-Wno-implicit-function-declaration",
  "-Werror-implicit-function-declaration",
  "-Wimplicit-int",
  "-Wno-implicit-int",
  "-Wimplicit",
  "-Wno-implicit",
  "-Wmain",
  "-Wno-main",
  "-Wmissing-braces",
  "-Wno-missing-braces",
  "-Wmissing-declarations",
  "-Wno-missing-declarations",
  "-Wmissing-prototypes",
  "-Wno-missing-prototypes",
  "-Wnested-externs",
  "-Wno-nested-externs",
  "-Wparentheses",
  "-Wno-parentheses",
  "-Wpointer-arith",
  "-Wno-pointer-arith",
  "-Wredundant-decls",
  "-Wno-redundant-decls",
  "-Wsign-compare",
  "-Wno-sign-compare",
  "-Wstrict-prototypes",
  "-Wno-strict-prototypes",
  "-Wtraditional",
  "-Wno-traditional",
  "-Wtrigraphs",
  "-Wno-trigraphs",
  "-Wundef",
  "-Wno-undef",
  "-Wwrite-strings",
  "-Wno-write-strings",

  0
};


/* Likewise for -W.  */

struct { char *string; int *variable; int on_value;} W_options[] =
{
  {"unused", &warn_unused, 1},
  {"error", &warnings_are_errors, 1},
  {"shadow", &warn_shadow, 1},
  {"switch", &warn_switch, 1},
  {"aggregate-return", &warn_aggregate_return, 1},
  {"cast-align", &warn_cast_align, 1},
  {"uninitialized", &warn_uninitialized, 1},
  {"inline", &warn_inline, 1}
};


/* Timing stuff  */
struct timeval parse_time, analyze_time, tlb_time, 
  serialize_time, deserialize_time, rollback_time, 
  region_serialization_time, region_deserialization_time;

static struct timeval start_time, finish_time;

/* Begin timing */
static void begin_time(void)
{
   /*  timerisset(&start_time); */
    gettimeofday(&start_time, NULL);
}

/* End timing, adding interval to timer. */
static void end_time(struct timeval *timer)
{
    struct timeval diff_time;

  /*   timerisset(&start_time); */
    gettimeofday(&finish_time, NULL);
    timersub(&finish_time, &start_time, &diff_time);
    timeradd(timer, &diff_time, timer);
    timerclear(&start_time);
}

void print_time(FILE *f,struct timeval *timer)
{
    double secs, usecs;

    usecs = ((double) timer->tv_usec) / 1000000;
    secs = ((double) timer->tv_sec) + usecs;
    fprintf(f,"%.3f", secs);
}

/* Handler for SIGPIPE.  */

static void pipe_closed (int signo)
{
  fatal("output pipe has been closed");
}

/* Print a fatal error message.  NAME is the text.
   Also include a system error message based on `errno'.  */

void pfatal_with_name(char *name)
{
  fprintf(stderr, "%s: ", progname);
  perror(name);
  exit(FATAL_EXIT_CODE);
}

void outofmemory(void)
{
  fprintf(stderr, "Out of memory - exiting\n");
  exit(FATAL_EXIT_CODE);
}

/* Compile an entire file of output from cpp, named NAME.
   Write a file of assembly output and various debugging dumps.  */

static int files_processed = 0;
static int files_skipped = 0;

static void compile_file(char *name) deletes
{
  FILE *ifile;

  begin_time();
#if !USE_CPPLIB
  /* Open input file.  */

  if (name == 0 || !strcmp (name, "-"))
    {
      ifile = stdin;
      name = "stdin";
    }
  else
    ifile = fopen (name, "r");
  if (ifile == 0)
    pfatal_with_name (name);

#ifdef IO_BUFFER_SIZE
  setvbuf(ifile, (char *) xmalloc (IO_BUFFER_SIZE), _IOFBF, IO_BUFFER_SIZE);
#endif
#endif /* !USE_CPPLIB */

  set_nomem_handler(outofmemory);
  parse_region = newregion();

  init_types();
  cval_init();
  init_semantics();
  set_input(ifile, name);
  init_lex();
  /* analysis_init(); */

  if (yyparse () != 0)
    {
      if (errorcount == 0)
	fprintf (stderr, "Errors detected in input file (your bison.simple is out of date)");
    }

  fclose (ifile);

  end_time(&parse_time);

  if (errorcount == 0)
    {
      if (the_program)
	{
	  files_processed++;


	    {
	      int last_clock;
	      inhibit_warnings = 1; // FIX
	      fprintf(stderr, "Analyzing...");
	      
	      begin_time();
	      if (flag_points_to) analyze(the_program);
	      end_time(&analyze_time);
	      fprintf(stderr,"analysis time so far: ");
	      print_time(stderr,&analyze_time);
	      fprintf(stderr,"\n");
	      last_clock = banshee_get_time();
	      fprintf(stdout, "file: %s clock: %d\n",
		      name, last_clock);
	      inhibit_warnings = 0;
	      errorcount = 0;

	      if (files_processed == debug_backtrack_prefix) {
		debug_backtrack_time = last_clock;
		fprintf(stderr, "%d files analyzed: will backtrack to %d\n",
			files_processed, debug_backtrack_time);
	      }
	    }
	  deleteregion(parse_region);
	  parse_region = newregion();
	}
    }
  else
    {
      files_skipped++;
      fprintf(stderr,"Skipping problematic input...\n\n");
      errorcount = 0;
    }

}

/* Decode the string P as a language-specific option for C. */
void c_decode_option(char *p)
{
  if (!strcmp (p, "-ftraditional") || !strcmp (p, "-traditional"))
    {
      flag_traditional = 1;
    }
  else if (!strcmp (p, "-fallow-single-precision"))
    flag_allow_single_precision = 1;
  else if (!strcmp (p, "-fnotraditional") || !strcmp (p, "-fno-traditional"))
    {
      flag_traditional = 0;
    }
  else if (!strcmp (p, "-fdollars-in-identifiers"))
    dollars_in_ident = 1;
  else if (!strcmp (p, "-fno-dollars-in-identifiers"))
    dollars_in_ident = 0;
  else if (!strcmp (p, "-fsigned-char"))
    flag_signed_char = 1;
  else if (!strcmp (p, "-funsigned-char"))
    flag_signed_char = 0;
  else if (!strcmp (p, "-fno-signed-char"))
    flag_signed_char = 0;
  else if (!strcmp (p, "-fno-unsigned-char"))
    flag_signed_char = 1;
  else if (!strcmp (p, "-fsigned-bitfields")
	   || !strcmp (p, "-fno-unsigned-bitfields"))
    {
      flag_signed_bitfields = 1;
      explicit_flag_signed_bitfields = 1;
    }
  else if (!strcmp (p, "-funsigned-bitfields")
	   || !strcmp (p, "-fno-signed-bitfields"))
    {
      flag_signed_bitfields = 0;
      explicit_flag_signed_bitfields = 1;
    }
  else if (!strcmp (p, "-fshort-enums"))
    flag_short_enums = 1;
  else if (!strcmp (p, "-fno-short-enums"))
    flag_short_enums = 0;
  else if (!strcmp (p, "-fcond-mismatch"))
    flag_cond_mismatch = 1;
  else if (!strcmp (p, "-fno-cond-mismatch"))
    flag_cond_mismatch = 0;
  else if (!strcmp (p, "-fasm"))
    flag_no_asm = 0;
  else if (!strcmp (p, "-fno-asm"))
    flag_no_asm = 1;
  else if (!strcmp (p, "-ansi"))
    flag_no_asm = 1;
  else if (!strcmp (p, "-Werror-implicit-function-declaration"))
    mesg_implicit_function_declaration = 2;
  else if (!strcmp (p, "-Wimplicit-function-declaration"))
    mesg_implicit_function_declaration = 1;
  else if (!strcmp (p, "-Wno-implicit-function-declaration"))
    mesg_implicit_function_declaration = 0;
  else if (!strcmp (p, "-Wimplicit-int"))
    warn_implicit_int = 1;
  else if (!strcmp (p, "-Wno-implicit-int"))
    warn_implicit_int = 0;
  else if (!strcmp (p, "-Wimplicit"))
    {
      warn_implicit_int = 1;
      if (mesg_implicit_function_declaration != 2)
        mesg_implicit_function_declaration = 1;
    }
  else if (!strcmp (p, "-Wno-implicit"))
    warn_implicit_int = 0, mesg_implicit_function_declaration = 0;
  else if (!strcmp (p, "-Wwrite-strings"))
    warn_write_strings = 1;
  else if (!strcmp (p, "-Wno-write-strings"))
    warn_write_strings = 0;
  else if (!strcmp (p, "-Wcast-qual"))
    warn_cast_qual = 1;
  else if (!strcmp (p, "-Wno-cast-qual"))
    warn_cast_qual = 0;
  else if (!strcmp (p, "-Wbad-function-cast"))
    warn_bad_function_cast = 1;
  else if (!strcmp (p, "-Wno-bad-function-cast"))
    warn_bad_function_cast = 0;
  else if (!strcmp (p, "-Wpointer-arith"))
    warn_pointer_arith = 1;
  else if (!strcmp (p, "-Wno-pointer-arith"))
    warn_pointer_arith = 0;
  else if (!strcmp (p, "-Wstrict-prototypes"))
    warn_strict_prototypes = 1;
  else if (!strcmp (p, "-Wno-strict-prototypes"))
    warn_strict_prototypes = 0;
  else if (!strcmp (p, "-Wmissing-prototypes"))
    warn_missing_prototypes = 1;
  else if (!strcmp (p, "-Wno-missing-prototypes"))
    warn_missing_prototypes = 0;
  else if (!strcmp (p, "-Wmissing-declarations"))
    warn_missing_declarations = 1;
  else if (!strcmp (p, "-Wno-missing-declarations"))
    warn_missing_declarations = 0;
  else if (!strcmp (p, "-Wredundant-decls"))
    warn_redundant_decls = 1;
  else if (!strcmp (p, "-Wno-redundant-decls"))
    warn_redundant_decls = 0;
  else if (!strcmp (p, "-Wnested-externs"))
    warn_nested_externs = 1;
  else if (!strcmp (p, "-Wno-nested-externs"))
    warn_nested_externs = 0;
  else if (!strcmp (p, "-Wtraditional"))
    warn_traditional = 1;
  else if (!strcmp (p, "-Wno-traditional"))
    warn_traditional = 0;
  else if (!strcmp (p, "-Wformat"))
    warn_format = 1;
  else if (!strcmp (p, "-Wno-format"))
    warn_format = 0;
  else if (!strcmp (p, "-Wchar-subscripts"))
    warn_char_subscripts = 1;
  else if (!strcmp (p, "-Wno-char-subscripts"))
    warn_char_subscripts = 0;
  else if (!strcmp (p, "-Wconversion"))
    warn_conversion = 1;
  else if (!strcmp (p, "-Wno-conversion"))
    warn_conversion = 0;
  else if (!strcmp (p, "-Wparentheses"))
    warn_parentheses = 1;
  else if (!strcmp (p, "-Wno-parentheses"))
    warn_parentheses = 0;
  else if (!strcmp (p, "-Wreturn-type"))
    warn_return_type = 1;
  else if (!strcmp (p, "-Wno-return-type"))
    warn_return_type = 0;
  else if (!strcmp (p, "-Wmissing-braces"))
    warn_missing_braces = 1;
  else if (!strcmp (p, "-Wno-missing-braces"))
    warn_missing_braces = 0;
  else if (!strcmp (p, "-Wmain"))
    warn_main = 1;
  else if (!strcmp (p, "-Wno-main"))
    warn_main = 0;
  else if (!strcmp (p, "-Wsign-compare"))
    warn_sign_compare = 1;
  else if (!strcmp (p, "-Wno-sign-compare"))
    warn_sign_compare = 0;
  else if (!strcmp (p, "-Wmultichar"))
    warn_multichar = 1;
  else if (!strcmp (p, "-Wno-multichar"))
    warn_multichar = 0;
  else if (!strcmp (p, "-Wall"))
    {
      /* We save the value of warn_uninitialized, since if they put
	 -Wuninitialized on the command line, we need to generate a
	 warning about not using it without also specifying -O.  */
      if (warn_uninitialized != 1)
	warn_uninitialized = 2;
      warn_implicit_int = 1;
      mesg_implicit_function_declaration = 1;
      warn_return_type = 1;
      warn_unused = 1;
      warn_switch = 1;
      warn_format = 1;
      warn_char_subscripts = 1;
      warn_parentheses = 1;
      warn_missing_braces = 1;
      /* We set this to 2 here, but 1 in -Wmain, so -ffreestanding can turn
	 it off only if it's not explicit.  */
      warn_main = 2;
    }
}

void rc_decode_option(char *p)
{
  /* RC-specific options */
  if (!strcmp (p, "-frc-local-opt"))
    rc_algorithm = rc_optimal;
  else if (!strcmp (p, "-frc-local-assign"))
    rc_algorithm = rc_assignment;
  else if (!strcmp (p, "-frc-pairs"))
    rc_pairs = 1;
  else if (!strcmp (p, "-frc-pairs-from"))
    rc_pairs = rc_pairs_from = 1;
  else if (!strcmp (p, "-frc-pairs-array"))
    rc_pairs = rc_pairs_array = 1;
  else if (!strcmp (p, "-frc-stats"))
    rc_collect_statistics = 1;
  else if (!strcmp (p, "-frc-profile"))
    rc_collect_statistics = rc_stats_detailed = 1;
  else if (!strcmp (p, "-frc-cost"))
    rc_collect_statistics = rc_collect_statistics_end = rc_stats_costs = 1;
  else if (!strcmp (p, "-frc-nolocals"))
    rc_locals = 0;
  else if (!strcmp (p, "-frc-noglobals"))
    rc_globals = 0;
  else if (!strcmp (p, "-frc-norc"))
    rc_locals = rc_globals = 0;
  else if (!strcmp (p, "-frc-noqchecks"))
    rc_nochecks = 1;
  else if (!strcmp (p, "-frc-keepqchecks"))
    rc_keepchecks = 1;
  else if (!strcmp (p, "-frc-nodeletes"))
    rc_nodeletes = 1;
  else if (!strcmp (p, "-frc-qopt-perfn"))
    rc_chkopt = 1;
  else if (!strcmp (p, "-frc-qopt"))
    rc_chkopt_perfile = 1;
  else if (!strcmp (p, "-frc-safe-adjustfn"))
    rc_safe_adjustfn = 1;
  else
    fprintf(stderr, "Ignoring unknown rc option `%s'\n", p);
}

static void rcc_aborting(int s)
{
  signal(SIGABRT, 0);
  fprintf(stderr, "rcc: Internal error. Please send a bug report to David Gay at dgay@acm.org\n");
  if (getenv("RCCDEBUG"))
    abort();
  else
    exit(FATAL_EXIT_CODE);
}

/* Entry point of cc1/c++.  Decode command args, then call compile_file.
   Exit code is 35 if can't open files, 34 if fatal error,
   33 if had nonfatal errors, else success.  */

void *orig_brk;

int main(int argc, char **argv) deletes
{
  register int i;
  /*  char *filename = 0; */
  dd_list files;
  dd_list_pos cur;
  int version_flag = 0;
  char *p;
  char *input_pat;
  region main_region;
  region_init();
  
  main_region = newregion();

  signal(SIGABRT, rcc_aborting);

  orig_brk = sbrk(0);

#ifdef USEMALLOC
#ifdef TIMING
  timing_init();
#endif
#endif
  analysis_init();

  copy_argc = 0;
  copy_argv = xmalloc((argc + 1) * sizeof(*copy_argv));
  files = dd_new_list(main_region);

  p = argv[0] + strlen (argv[0]);
  while (p != argv[0] && p[-1] != '/'
#ifdef DIR_SEPARATOR
	 && p[-1] != DIR_SEPARATOR
#endif
	 )
    --p;
  progname = p;

#ifdef SIGPIPE
  signal (SIGPIPE, pipe_closed);
#endif

  copy_argv[0] = argv[0];
  copy_argc = 1;
  for (i = 1; i < argc; i++)
    {
      int j;
      bool copy_arg = TRUE;

      /* If this is a language-specific option,
	 decode it in a language-specific way.  */
      for (j = 0; lang_options[j] != 0; j++)
	if (!strncmp (argv[i], lang_options[j],
		      strlen (lang_options[j])))
	  break;

      if (!strncmp (argv[i], "-frc-", 5))
	{
	  /* An rc specific option. Process specially, do not forward to cc1 */
	  copy_arg = FALSE;
	  rc_decode_option(argv[i]);
	}
      else if (lang_options[j] != 0)
	/* If the option is valid for *some* language,
	   treat it as valid even if this language doesn't understand it.  */
	c_decode_option(argv[i]);
      else if (argv[i][0] == '-' && argv[i][1] != 0)
	{
	  register char *str = argv[i] + 1;
	  if (str[0] == 'Y')
	    str++;

	  if (!strcmp (str, "dumpbase"))
	    copy_argv[copy_argc++] = argv[i++];
	  else if (str[0] == 'f' && str[1] == 'b' && str[2] == 't')
	    {
	      register char *p = &str[3];
	      flag_debug_backtrack = 1;
	      debug_backtrack_prefix = atoi(p);
	      fprintf(stderr,"Debugging backtrack code with %d files\n",
		      debug_backtrack_prefix);
	    }
	  else if (str[0] == 'f' && str[1] == 'b' && str[2] =='a' && str[3] == 'c' && str[4] == 'k' ) 
	    {
	      register char *p = &str[5];
	      flag_backtrack_constraints = 1;
	      backtrack_time = atoi(p);
	      fprintf(stderr,
		      "Deserialized constraints will be rolled back to time %d\n",
		      backtrack_time);
	    }
	  else if (str[0] == 'f')
	    {
	      register char *p = &str[1];
	      int found = 0;

	      /* Some kind of -f option.
		 P's value is the option sans `-f'.
		 Search for it in the table of options.  */

	      for (j = 0;
		   !found && j < sizeof (f_options) / sizeof (f_options[0]);
		   j++)
		{
		  if (!strcmp (p, f_options[j].string))
		    {
		      *f_options[j].variable = f_options[j].on_value;
		      /* A goto here would be cleaner,
			 but breaks the vax pcc.  */
		      found = 1;
		    }
		  if (p[0] == 'n' && p[1] == 'o' && p[2] == '-'
		      && ! strcmp (p+3, f_options[j].string))
		    {
		      *f_options[j].variable = ! f_options[j].on_value;
		      found = 1;
		    }
		}
	    }
	  else if (!strcmp (str, "pedantic"))
	    pedantic = 1;
	  else if (!strcmp (str, "pedantic-errors"))
	    flag_pedantic_errors = pedantic = 1;
	  else if (!strcmp (str, "quiet"))
	    quiet_flag = 1;
	  else if (!strcmp (str, "version"))
	    version_flag = 1;
	  else if (!strcmp (str, "w"))
	    inhibit_warnings = 1;
	  else if (!strcmp (str, "W"))
	    {
	      extra_warnings = 1;
	      /* We save the value of warn_uninitialized, since if they put
		 -Wuninitialized on the command line, we need to generate a
		 warning about not using it without also specifying -O.  */
	      if (warn_uninitialized != 1)
		warn_uninitialized = 2;
	    }
	  else if (str[0] == 'W')
	    {
	      register char *p = &str[1];
	      int found = 0;

	      /* Some kind of -W option.
		 P's value is the option sans `-W'.
		 Search for it in the table of options.  */

	      for (j = 0;
		   !found && j < sizeof (W_options) / sizeof (W_options[0]);
		   j++)
		{
		  if (!strcmp (p, W_options[j].string))
		    {
		      *W_options[j].variable = W_options[j].on_value;
		      /* A goto here would be cleaner,
			 but breaks the vax pcc.  */
		      found = 1;
		    }
		  if (p[0] == 'n' && p[1] == 'o' && p[2] == '-'
		      && ! strcmp (p+3, W_options[j].string))
		    {
		      *W_options[j].variable = ! W_options[j].on_value;
		      found = 1;
		    }
		}

	      if (found)
		;
	      else if (!strncmp (p, "id-clash-", 9))
		{
		  char *endp = p + 9;

		  while (*endp)
		    {
		      if (*endp >= '0' && *endp <= '9')
			endp++;
		      else
			{
			  error ("Invalid option `%s'", argv[i]);
			  goto id_clash_lose;
			}
		    }
		  warn_id_clash = 1;
		  id_clash_len = atoi (str + 10);
		id_clash_lose: ;
		}
	      else if (!strncmp (p, "larger-than-", 12))
		{
		  char *endp = p + 12;

		  while (*endp)
		    {
		      if (*endp >= '0' && *endp <= '9')
			endp++;
		      else
			{
			  error ("Invalid option `%s'", argv[i]);
			  goto larger_than_lose;
			}
		    }
		  warn_larger_than = 1;
		  larger_than_size = atoi (str + 13);
		larger_than_lose: ;
		}
	    }
	  else if (!strcmp (str, "o"))
	    copy_argv[copy_argc++] = argv[i++];
	  else if (str[0] == 'G')
	    {
	      if (str[1] == '\0')
		copy_argv[copy_argc++] = argv[i++];
	    }
	  else if (!strncmp (str, "aux-info", 8))
	    {
	      if (str[8] == '\0')
		copy_argv[copy_argc++] = argv[i++];
	    }
	}
      else if (argv[i][0] == '+')
	;
      else /* allow wildcards */
	{
	  glob_t globbuf;
	  char **cur;
	  
	  input_pat = argv[i];
	  
	  if (glob(argv[i], GLOB_NOSORT, NULL, &globbuf)) 
	    {
	      /* glob returned non-zero error status; abort */
	      fprintf(stderr, "%s: file not found\n", argv[i]);
	      exit(FATAL_EXIT_CODE);
            } else
	      for (cur = globbuf.gl_pathv; *cur; cur++) 
		{
		  dd_add_last(main_region, files,
			      rstrdup(main_region, *cur));
                }
	  
	  copy_arg = FALSE;
	}

      if (copy_arg)
	copy_argv[copy_argc++] = argv[i];
    }
  copy_argv[copy_argc] = NULL;

  /* RC doesn't like shadowing */
  /*warn_shadow = error_shadow = 1;*/


  if (flag_points_to && flag_deserialize_constraints) 
    {
      begin_time();
      analysis_deserialize("andersen.out");
      end_time(&deserialize_time);

      if (flag_backtrack_constraints) {
	fprintf(stderr, "Backtracking...\n");
	begin_time();
	analysis_backtrack(backtrack_time);
	end_time(&rollback_time);
      }
    } 

/*   if (dd_is_empty(files)) */
/*     compile_file(0); */
/*   else */

  if (flag_points_to && flag_debug_region_deserialization)
    {
      translation t;
      region temp = newregion();
      begin_time();
      t = deserialize("data","offsets", NULL, temp);
      end_time(&region_deserialization_time);

      printf("\nRegion deserialization time: ");
      print_time(stdout,&region_deserialization_time);
      printf("\n");
      exit(0);
    }

  if (!dd_is_empty(files))
    dd_scan(cur, files)
      {
	char *file;
	file = DD_GET(char *, cur);
	fprintf(stderr, "Parsing %s...", file);
	compile_file(file);
      }

  fprintf(stdout,"##################\n");
  
  if (flag_debug_backtrack) {
    fprintf(stderr, "Backtracking...\n");
    banshee_backtrack(debug_backtrack_time);
  }

  if(flag_serialize_constraints)
    {
      begin_time();
      analysis_serialize("andersen.out");
      end_time(&serialize_time);
    }

  if (flag_points_to && flag_debug_region_serialization)
    {
      begin_time();
      serialize(get_persistent_regions(), "data", "offsets");
      end_time(&region_serialization_time);
    }

  printf("Andersen's Points to Analysis\n");
  printf("Files analyzed: %d\n",files_processed);
  printf("Files skipped: %d\n",files_skipped);
    

  
  fprintf(stderr,"Computing tlb...\n");
  begin_time();
  print_analysis_results();
  end_time(&tlb_time);
  fprintf(stderr,"Finished computing tlb\n");
  
  
  if(flag_pta_profile) {
    struct rusage usage;
    printf("========Profile=========\n");
    printf("Parse time: ");
    print_time(stdout,&parse_time);
    printf("\nAnalyze time: ");
    print_time(stdout,&analyze_time);
    printf("\nTLB time: ");
    print_time(stdout,&tlb_time);

    if (flag_serialize_constraints) {
    printf("\nSerialize time: ");
    print_time(stdout,&serialize_time);
    }
    
    if (flag_debug_region_serialization) {
    printf("\nRegion serialization time: ");
    print_time(stdout,&region_serialization_time);
    }

    if (flag_deserialize_constraints && flag_backtrack_constraints) {
    printf("\nRollback time: ");
    print_time(stdout,&rollback_time);
    }

    if (flag_deserialize_constraints) {
      printf("\nDeserialize time: ");
      print_time(stdout,&deserialize_time);
    }

    getrusage(RUSAGE_SELF,&usage);
    
    printf("\nUser time:");
    print_time(stdout,&usage.ru_utime); 
    printf("\nSystem time:");
    print_time(stdout,&usage.ru_stime); 
    printf("\n");
  }

  
  if (flag_print_results) 
    print_points_to_sets();        
  if (flag_print_stats)
    {
      analysis_stats(stdout);
    }
  
  if (flag_print_graph)
    {
      analysis_print_graph();
    }
  
  
  /*     if (flag_print_memusage) */
  /*       { */
  /* 	print_memory_usage(); */
  /* 	// printf("\nMemory usage (bytes): %li\n",get_memusage()); */
  /*       } */
  
  
  if (errorcount)
    exit (FATAL_EXIT_CODE);
  exit (SUCCESS_EXIT_CODE);
  return 0;
}
