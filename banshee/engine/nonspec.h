/*
 * Copyright (c) 2000-2004
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef NONSPEC_H
#define NONSPEC_H
#include <stdio.h>
#include "linkage.h"
#include "bool.h"
#include "list.h"

EXTERN_C_BEGIN

typedef enum 
{
  flowrow_sort,
  setif_sort,
  setst_sort,
  term_sort,
} sort_kind;

typedef struct gen_e_ *gen_e;

DECLARE_LIST(gen_e_list, gen_e);

typedef enum 
{
  vnc_pos,
  vnc_neg,
  vnc_non
} vnc_kind;

struct sig_elt_
{
  vnc_kind variance;
  sort_kind sort;
};

typedef struct sig_elt_ sig_elt;
typedef struct constructor_ *constructor;
typedef struct flowrow_field_ *flowrow_field;

DECLARE_LIST(sig_elt_list,sig_elt*);
DECLARE_LIST(flowrow_map,flowrow_field);

typedef struct {
  constructor c;
  int i;
  gen_e e;
} pattern;

// Fully expanded sort kinds. Translate to sort_kinds as needed
// These are needed when we have row(base) and need to know the base
// which occurs when we have 0,1, or _ row expressions (for ibanshee)
typedef enum {
  e_setif_sort,
  e_term_sort,
  e_flowrow_setif_sort,
  e_flowrow_term_sort
} e_sort;


/*
   Flags
*/
extern bool flag_merge_projections;
extern bool flag_eliminate_cycles;
extern bool flag_occurs_check;
extern bool flag_warning_msgs;

/* 
   Operations for building terms
*/

/* Defines a new constructor for sort s with the given signature */
constructor make_constructor(const char *name,sort_kind sort, sig_elt[],
			     int arity);

constructor make_constructor_from_list(const char*name, sort_kind sort,
				       sig_elt_list elts);

/* Build the term c(exps[0]....exps[n]) */
gen_e constructor_expr(constructor c, gen_e exps[], int arity);

/*  gen_e constructor_expr_from_list(constructor c, gen_e_list exps); */

/* make a constant of sort s */
gen_e setif_constant(const char *name);

gen_e setst_constant(const char *name);

gen_e term_constant(const char *name);

/* Creates a projection pattern projpat(c,i,e) */
gen_e setif_proj_pat(constructor c,int i,gen_e e);

gen_e setst_proj_pat(constructor c, int i, gen_e e);

/* Adds a constraint e <= projpat(c,i,fv) where fv is a fresh variable */
gen_e setif_proj(constructor c, int i, gen_e e);

gen_e setst_proj(constructor c, int i, gen_e e);

/* Make a new variable of sort s */
gen_e setif_fresh(const char *name);

gen_e term_fresh(const char *name);

gen_e flowrow_fresh(const char *name);

gen_e setst_fresh(const char *name);

/* Operations for unions */

gen_e setif_union(gen_e_list exps);

gen_e setif_inter(gen_e_list exps);

gen_e setst_union(gen_e_list exps);

gen_e setst_inter(gen_e_list exps);

/* Empty set of sort s */
gen_e setif_zero(void);

gen_e setst_zero(void);

gen_e flowrow_zero(sort_kind base_sort);

gen_e term_zero(void);

/* Universal set of sort s */
gen_e setif_one(void);

gen_e setst_one(void);

gen_e flowrow_one(sort_kind base_sort);

gen_e term_one(void);

/*
  Operations for building flowrows 
*/

/* Closed flowrow of base sort s */
gen_e flowrow_abs(sort_kind base_sort);

/* Wild flowrow of base sort s */
gen_e flowrow_wild(sort_kind base_sort);

/* Add a field */
flowrow_field flowrow_make_field(const char *name, gen_e e);

/* Build a flowrow of <l : e>_fields o <rest>  */
gen_e flowrow_make_row(flowrow_map fields, gen_e rest);

/* 
   Inclusion functions
*/
int call_sort_inclusion(gen_e e1, gen_e e2);
int call_sort_unify(gen_e e1, gen_e e2);

int call_setif_inclusion(gen_e e1,gen_e e2);

int call_setif_unify(gen_e e1, gen_e e2);

int call_setst_inclusion(gen_e e1, gen_e e2);
int call_setst_unify(gen_e e1, gen_e e2);

int call_flowrow_inclusion(gen_e e1,gen_e e2);
int call_flowrow_unify(gen_e e1, gen_e e2);

int call_term_unify(gen_e e1, gen_e e2);
int call_term_cunify(gen_e e1, gen_e e2);

/*
  Extracting solutions 
 */
struct decon
{
  char *name;
  int arity;
  gen_e *elems;
};

struct decon deconstruct_expr(constructor c,gen_e e);

struct decon deconstruct_any_expr(gen_e e);

gen_e_list setif_tlb(gen_e e);

gen_e_list setst_tlb(gen_e e);

gen_e term_get_ecr(gen_e e);

gen_e flowrow_extract_field(const char *label,gen_e row);
flowrow_map flowrow_extract_fields(gen_e row);
gen_e flowrow_extract_rest(gen_e row);

void nonspec_init(void);
void nonspec_reset(void);

void nonspec_stats(FILE *f);

void expr_print(FILE *f,gen_e e);

bool expr_eq(gen_e e1, gen_e e2);

bool expr_is_constant(gen_e e);

char *expr_constant_name(gen_e e);

int expr_stamp(gen_e e);

/* Keep these in sync with banshee.h */
typedef enum banshee_error_kind
{
  bek_cons_mismatch, 	        // c(...) <= d(...)
  bek_occurs_check,		// occurs check failed (cyclic unification)
} banshee_error_kind;

/* type for error callbacks */
typedef void (*banshee_error_handler_fn) (gen_e e1, gen_e e2, banshee_error_kind kind);

void register_error_handler(banshee_error_handler_fn error_handler);

/* Constructor groups */
typedef struct cons_group_ *cons_group;

cons_group make_cons_group(const char *name, sig_elt s[], int arity);

/* Add a constructor to a cons group. Must have the same signature as
   the group, and be a setif constructor */
void cons_group_add(cons_group g, constructor c);

/* Create a group projection pattern */
gen_e setif_group_proj_pat(cons_group g, int i, gen_e e);

EXTERN_C_END

#endif /* NONSPEC_H */
