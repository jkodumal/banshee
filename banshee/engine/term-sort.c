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

#include <assert.h>
#include "regions.h"
#include "term-sort.h"
#include "hash.h"

struct term_constant_ /* extends gen_e */
{
#ifdef NONSPEC
  sort_kind sort;
#endif
  int type;
  stamp st;
  char *name;
};

typedef struct term_rollback_info_ { /* extends banshee_rollback_info */
  banshee_time time;
  sort_kind kind;
  hash_table added_edges; 	/* a mapping from bounds to gen_e's added */
} * term_rollback_info; 

typedef struct term_constant_ *term_constant_;

region term_sort_region;
term_hash term_sort_hash;
bool flag_occurs_check = FALSE;

term_rollback_info term_current_rollback_info = NULL;

struct term_stats term_stats;

stamp term_get_stamp(gen_e e)
{
  if ( ((gen_term)e)->type == VAR_TYPE )
    return ((gen_term)term_get_ecr(e))->st;
  else 
    return ((gen_term)e)->st;
}

gen_e term_fresh(const char *name)
{
  term_stats.fresh++;
  return (gen_e)tv_fresh(term_sort_region,name);
}

gen_e term_fresh_large(const char *name)
{
  term_stats.fresh_large++;
  return (gen_e)tv_fresh_large(term_sort_region,name);
}

gen_e term_fresh_small(const char *name)
{
  term_stats.fresh_small++;
  return (gen_e)tv_fresh_small(term_sort_region,name);
}


#ifdef NONSPEC
static struct gen_term zero = {ZERO_TYPE,term_sort,ZERO_TYPE};
static struct gen_term one  = {ONE_TYPE,term_sort,ONE_TYPE};
#else
static struct gen_term zero = {ZERO_TYPE,ZERO_TYPE};
static struct gen_term one  = {ONE_TYPE,ONE_TYPE};
#endif /* NONSPEC */

gen_e term_zero(void)
{
  return (gen_e)&zero;
}

gen_e term_one(void)
{
  return (gen_e)&one;
}


gen_e term_constant(const char *str)
{
  stamp st[2];
  gen_e result;
  char *name = rstrdup(term_sort_region,str);

  assert (str != NULL);
  
  st[0] = CONSTANT_TYPE;
  st[1] = stamp_string(name); 

  if ( (result = term_hash_find(term_sort_hash,st,2)) == NULL)
    {
      term_constant_ c = ralloc(term_sort_region, struct term_constant_);
      c->type = CONSTANT_TYPE;
      c->st = stamp_fresh();
      c->name = name;

      result = (gen_e) c;
      term_hash_insert(term_sort_hash,result,st,2);
      
      return result;
    }
  
  else
    {
      return result;
    }

}

static bool term_is_bottom(gen_e e)
{
  return (term_is_zero(e) || term_is_var(e));
}

bool term_is_zero(gen_e e)
{
  return ( ((gen_term)term_get_ecr(e))->type == ZERO_TYPE);
}

bool term_is_one(gen_e e)
{
  return ( ((gen_term)term_get_ecr(e))->type == ONE_TYPE);
}

bool term_is_var(gen_e e)
{
  return ( ((gen_term)term_get_ecr(e))->type == VAR_TYPE);
}

bool term_is_constant(gen_e e)
{
  return ( ((gen_term)term_get_ecr(e))->type == CONSTANT_TYPE);
}

char *term_get_constant_name(gen_e e)
{
  gen_e ecr = term_get_ecr(e);
  if(! term_is_constant(ecr))
    return NULL;
  else
    return ((term_constant_)ecr)->name;
}

gen_e term_get_ecr(gen_e e)
{
  if (((gen_term)e)->type == VAR_TYPE)
    return tv_get_ecr((term_var)e);
  else return e;
}

static void fire_pending(term_var v, gen_e e, 
			 con_match_fn_ptr con_match, 
			 occurs_check_fn_ptr occurs)
{
  bounds_scanner scan;
  gen_e temp;

  bounds_scan(tv_get_pending(v),&scan);
  while (bounds_next(&scan,&temp))
    {
      term_unify(con_match,occurs,temp,e);
    }
}

static bool eq(gen_e e1, gen_e e2)
{
  return term_get_ecr(e1) == term_get_ecr(e2);
}

bool term_eq(gen_e e1, gen_e e2)
{
  return eq(e1,e2);
}


static void term_register_rollback(void) 
{
#ifdef BANSHEE_ROLLBACK
  term_current_rollback_info = 
    ralloc(banshee_rollback_region, struct term_rollback_info_); 
  banshee_set_time((banshee_rollback_info)term_current_rollback_info);
  term_current_rollback_info->kind = term_sort;
  term_current_rollback_info->added_edges = 
    make_hash_table(banshee_rollback_region,
		    4, ptr_hash, ptr_eq);
  
  banshee_register_rollback((banshee_rollback_info)term_current_rollback_info);
#endif /* BANSHEE_ROLLBACK */
}

static void term_register_edge(const bounds b, stamp st) {
#ifdef BANSHEE_ROLLBACK
  stamp_list sl = NULL;
  assert(term_current_rollback_info);
  
  /* The current rollback info already has an edge list associated
   * with this bounds */
  if (hash_table_lookup(term_current_rollback_info->added_edges,
			(hash_key)b,
			(hash_data *)&sl)) {
    assert(sl);
    stamp_list_cons(st,sl);
  }
  else {
    sl = new_stamp_list(banshee_rollback_region);
    stamp_list_cons(st,sl);
    hash_table_insert(term_current_rollback_info->added_edges,
		      (hash_key)b,
		      (hash_data)sl);
  }
#endif /* BANSHEE_ROLLBACK */ 
}

void term_unify(con_match_fn_ptr con_match, occurs_check_fn_ptr occurs,
		gen_e a, gen_e b)
{
  gen_e e1 = term_get_ecr(a),
    e2 = term_get_ecr(b);

  if (!banshee_check_rollback(term_sort)) {
    term_register_rollback();
  }

  if ( term_eq(e1,e2) )
    {
      return;
    }
  if (term_is_constant(e1) && term_is_constant(e2))
    { 
      handle_error(e1,e2,bek_cons_mismatch);
    }
  else if (term_is_var(e1))
    {
      term_var v = (term_var)e1;
   

      if (! term_is_bottom(e2))
	fire_pending(v,e2,con_match,occurs);

      if (term_is_var(e2)) {
	bounds_scanner scan;
	gen_e temp;
	const bounds pending1 = tv_get_pending((term_var)e2),
	  pending2 = tv_get_pending(v);
	tv_unify_vars(v,(term_var)e2);
	
	bounds_scan(pending1,&scan);

	while(bounds_next(&scan,&temp)) {
	  if (! tv_add_pending(v,temp,term_get_stamp(temp))) {
	    term_register_edge(tv_get_pending(v),term_get_stamp(temp));
	  }
	}
	
	bounds_scan(pending2, &scan);
	
	while(bounds_next(&scan,&temp)) {
	  if (! tv_add_pending(v,temp,term_get_stamp(temp))) {
	    term_register_edge(tv_get_pending(v),term_get_stamp(temp));
	  }
	}

      }
      else /* v = e2, e2 is not a var */
	{ 
	  if (flag_occurs_check && occurs(v,e2))
	    handle_error(e1,e2,bek_occurs_check);
	  tv_unify(v,e2); 
	}
    }
  else if (term_is_var(e2))
    {
      term_var v = (term_var)e2;

      if (! term_is_bottom(e2))
	fire_pending(v,e1,con_match,occurs);
      
      /* v = e1, e1 is not a var */
      if (flag_occurs_check && occurs(v,e1))
	handle_error(e1,e2,bek_occurs_check);
      tv_unify(v,e1); 
      
    }
  else con_match(e1,e2);
}

void term_cunify(con_match_fn_ptr con_match, occurs_check_fn_ptr occurs,
		 gen_e e1, gen_e e2)
{

  if (!banshee_check_rollback(term_sort)) {
    term_register_rollback();
  }

  if (term_is_bottom(e1) && term_is_var(e1))
    {
      term_var v1 = (term_var)e1;
      
      if (!tv_add_pending(v1,e2, term_get_stamp(e2))) {
	term_register_edge(tv_get_pending(v1),term_get_stamp(e2));
      }
    }
  else 
    {
      term_unify(con_match,occurs,e1,e2);
    }
}

static void term_reset_stats(void)
{
  term_stats.fresh = 0;
  term_stats.fresh_small = 0;
  term_stats.fresh_large = 0;
}

void term_print_stats(FILE *f)
{
  fprintf(f,"\n========== Term Var Stats ==========\n");
  fprintf(f,"Fresh : %d\n",term_stats.fresh); 
  fprintf(f,"Fresh Small : %d\n",term_stats.fresh_small);
  fprintf(f,"Fresh Large : %d\n",term_stats.fresh_large);
  fprintf(f,"=====================================\n");
}

/* TODO */
void term_print_constraint_graph(FILE *f)
{
}

void term_init(void)
{
  term_sort_region = newregion();
  term_sort_hash = make_term_hash(term_sort_region);
}

void term_reset(void)
{
  term_hash_delete(term_sort_hash);
  deleteregion_ptr(&term_sort_region);
 
  term_reset_stats();
 
  term_sort_region = newregion();
  term_sort_hash = make_term_hash(term_sort_region);
}

/* For term rollbacks, we just remove conditional unifications. The uf
   rollback will undo any actual unifications */
void term_rollback(banshee_rollback_info info)
{
  hash_table_scanner hash_scan;
  stamp_list_scanner stamp_scan;
  bounds next_bounds;
  stamp_list next_edges;
  stamp next_stamp;

  term_rollback_info tinfo = (term_rollback_info)info;
  
  assert(tinfo->kind = term_sort);
  
  hash_table_scan(tinfo->added_edges, &hash_scan);
  while(hash_table_next(&hash_scan,(hash_key *)&next_bounds,
			(hash_data *) &next_edges)) {
    stamp_list_scan(next_edges, &stamp_scan);
    while(stamp_list_next(&stamp_scan,&next_stamp)) {
      bounds_remove(next_bounds,next_stamp);
    }
  }
						     
}


