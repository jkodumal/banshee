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

/*
  call_s_inclusion, call_s_unify : called only from the top-level, must do a 
  setjmp, returns an error code (handles exceptions)

  call_ denotes a sort check (i.e. inclusion between e1 and e2 where the sorts
  of e1 and e2 are unknown or untrusted)

  _ind denotes induced, i.e. a constraint not introduced at the top level
  for induced inclusions/equations, no sort checks or setjmps are necessary.
  the invariant is that the non-induced relations are ONLY called from the
  toplevel. induced functions must have the signature
  gen_e * gen_e -> void 
  which means they can be passed as function pointers (incl_fn_ptr)
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "banshee.h"
#include "flowrow-sort.h"
#include "flow-var.h"
#include "setif-sort.h"
#include "setif-var.h"
#include "setst-sort.h"
#include "setst-var.h"
#include "term-sort.h"
#include "term-var.h"
#include "utils.h"

/* Types defined here MUST be larger than LARGEST_BUILTIN_TYPE (banshee.h). */
#define GROUP_PROJ_PAT_TYPE 11

/* Update this whenever a new term type is defined. It MUST be even. */
#define NUM_EXTRA_TYPES 2

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

struct cons_group_ 
{
  int arity;
  char *name;
  sig_elt *sig;
  int gid;
};

typedef struct cons_group_ *cons_group;

DECLARE_LIST(cons_group_list,cons_group);
DEFINE_LIST(cons_group_list,cons_group);

struct constructor_
{
  sort_kind sort;
  int type;
  int arity;
  char *name;
  sig_elt *sig;
  cons_group_list groups;
};

typedef struct constructor_ *constructor;

typedef struct proj_pat_
{
  sort_kind sort;
  int type;
  stamp st;
  int i;
  gen_e exp;
  vnc_kind variance;
  constructor c;
} *proj_pat;

typedef struct gproj_pat_
{
  sort_kind sort;
  int type;
  stamp st;
  int i;
  gen_e exp;
  //  vnc_kind variance;
  cons_group g;
} *gproj_pat;

typedef struct cons_expr_  
{
  sort_kind sort;
  int type;
  stamp st;
  int arity;
  char *name;
  sig_elt *sig;
  gen_e *exps;
  constructor c;
} * cons_expr;

struct decon
{
  char *name;
  int arity;
  gen_e *elems;
};

struct nonspec_stats_
{
  int dispatches;
} nonspec_stats_;

static int smallest_special_type = LARGEST_BUILTIN_TYPE + NUM_EXTRA_TYPES;

static int new_type()
{
  static int next_type = LARGEST_BUILTIN_TYPE + NUM_EXTRA_TYPES;

  assert(next_type %2 == 0);

  int ret = next_type;
//   if (next_type > 2000)
//     {
//       fprintf(stderr, "Exceeded maximum number of constructors\n");
//       assert(0);
//     }
  next_type += 2;
  return ret;
}

static bool fixed_sort(sort_kind s)
{
  return (s == flowrow_sort);
}

static void term_con_match(gen_e e1, gen_e e2);
static bool term_occurs(term_var v, gen_e e2);
static void setif_con_match(gen_e e1, gen_e e2);
static void setst_con_match(gen_e e1, gen_e e2);
static bool setif_res_proj(setif_var v1,gen_e e2);
static void call_inclusion_ind(gen_e e1, gen_e e2);
static void call_unify_ind(gen_e e1, gen_e e2);
int call_term_unify(gen_e e1, gen_e e2);
void expr_print(FILE *f, gen_e e);

void term_unify_ind(gen_e e1, gen_e e2)
{
  term_unify(term_con_match,term_occurs,e1,e2);
}

static bool setif_is_gpat(gen_e e)
{
  int type = ((setif_term)e)->type;
  return type == GROUP_PROJ_PAT_TYPE;
}

/* 
   Convention : constructor types are even, pats are odd.
   The smallest specialized type is smallest_special_type.
*/
static bool setif_is_pat(gen_e e)
{
  int type = ((setif_term)e)->type;
  return ( (type % 2) && (type > 11) );
}

static bool setst_is_pat(gen_e e)
{
  int type = ((setst_term)e)->type;
  return ( (type % 2) && (type > smallest_special_type) );
}

static bool setif_is_cons_expr(gen_e e)
{
  int type = ((setif_term)e)->type;

  return ( !(type % 2) && (type >= smallest_special_type) );
}

static bool setst_is_cons_expr(gen_e e)
{
  int type = ((setst_term)e)->type;

  return ( !(type % 2) && (type >= smallest_special_type) );
}

static bool term_is_cons_expr(gen_e e)
{
  int type = ((gen_term) term_get_ecr(e))->type;

  return ( !(type % 2) && (type >= smallest_special_type) );
}

static get_stamp_fn_ptr get_sort_stamp(sort_kind s)
{
switch (s)
    {
    case setif_sort:
      return setif_get_stamp;
    case setst_sort:
      return setst_get_stamp;
    case term_sort:
      return term_get_stamp;
    case flowrow_sort:
      return flowrow_get_stamp;
    default:
      fail("Unmatched sort in get_sort_zero\n");
      return NULL;
    }
  return NULL;
}

static gen_e get_sort_zero(sort_kind s)
{
switch (s)
    {
    case setif_sort:
      return setif_zero();
    case setst_sort:
      return setst_zero();
    case term_sort:
      return term_zero();
    case flowrow_sort:
    default:
      fail("Unmatched sort in get_sort_zero\n");
      return NULL;
    }
  return NULL;
}


static region get_sort_region(sort_kind s)
{
  switch (s)
    {
    case setif_sort:
      return setif_region;
    case setst_sort:
      return setst_region;
    case flowrow_sort:
      return flowrow_region;
    case term_sort:
      return term_sort_region;
    default:
      {
	fail("Unmatched sort in get_sort_region\n");
	return NULL;
      }
    }
  return NULL;
}

static term_hash get_sort_hash(sort_kind s)
{
  switch (s)
    {
    case setif_sort:
      return setif_hash;
    case setst_sort:
      return setst_hash;
    case flowrow_sort:
      return flowrow_hash;
    case term_sort:
      return term_sort_hash;
    default:
      fail("Unmatched sort in get_sort_hash\n");
      return NULL;
    }
  return NULL;
}


static gen_e get_named_proj_var(sort_kind s, bool large, char *name) /* HACK */
{
  switch (s)
    {
    case setif_sort:
      {
	if (large)
	  return (gen_e)setif_fresh_large(name);
	else return (gen_e)setif_fresh(name);	  
      }
      break;
    case setst_sort:
      {
	if (large)
	  return (gen_e)setst_fresh_large(name);
	else return (gen_e)setst_fresh(name);	
      }
      break;
    case flowrow_sort:
      {
	if (large)
	  return (gen_e)flowrow_fresh_large(name,setif_sort);
	else return (gen_e)flowrow_fresh(name,setif_sort);
      }
      break;
    case term_sort:
      {
	if (large)
	  return (gen_e)term_fresh_large(name);
	else return (gen_e)term_fresh(name);
      }	
      break;
    default:
      {
	fail("Unmatched sort in get_proj_var\n");
	return NULL;
      }
      break;
    }

  return NULL;
}

static gen_e get_vinv_proj_var(sort_kind s, constructor c, int i,gen_e e)
{
  char temp[512];
  snprintf(temp,512,"%s-%d(%s)",c->name,i,sv_get_name((setif_var)e));
  return get_named_proj_var(s,FALSE,temp);
}


static gen_e get_cinv_proj_var(sort_kind s,constructor c, int i)
{
  char temp[512];
  snprintf(temp,512,"%s<-%d>",c->name,i);
  return get_named_proj_var(s,FALSE,temp);
}

static gen_e get_proj_var(sort_kind s, bool large)
{
  return get_named_proj_var(s,large,NULL);
}

static void setif_inclusion_ind(gen_e e1,gen_e e2)
{
  setif_inclusion(setif_con_match,setif_res_proj,e1,e2);
}

static void setst_inclusion_ind(gen_e e1, gen_e e2)
{
  setst_inclusion(setst_con_match,e1,e2);
}

static bool pat_match(int t1, int t2)
{
  return (t1 - 1 == t2);
}

static bool gpat_match(cons_expr ce, gproj_pat pat)
{
  assert(ce->c->groups);
  return cons_group_list_member(ce->c->groups,pat->g);
}

/*
static char * sort_to_string(sort_kind s)
{
  switch(s)
    {
    case setif_sort:
      return "setif";
    case setst_sort:
      return "setst";
    case term_sort:
      return "term";
    case flowrow_sort:
      return "flowrow";
    default:
      return NULL;
    }
}
*/

constructor make_constructor(const char *name,sort_kind sort, sig_elt s[],
			     int arity)
{
  constructor c = ralloc(get_sort_region(sort),struct constructor_);
  sig_elt *sig = rarrayalloc(get_sort_region(sort),arity,sig_elt);
  
  c->type = new_type();

  if (arity) {
    memcpy(sig,s,sizeof(sig_elt)*arity);
  }

  if ( fixed_sort(sort) )
    fail("Specified sort does not allow constructor types\n");
  
  c->sort = sort;
  c->arity = arity;
  c->name = rstrdup(get_sort_region(sort),name);
  c->sig = sig;

  if (sort == setif_sort) c->groups = new_cons_group_list(get_sort_region(sort));
  else c->groups = NULL;

  return c;
}

gen_e constructor_expr(constructor c, gen_e exps[], int arity)
{
  cons_expr result;
  int i;
  get_stamp_fn_ptr get_stamp;
  region sort_region = get_sort_region(c->sort);
  term_hash sort_hash = get_sort_hash(c->sort);
  
  stamp *st = rarrayalloc(sort_region,arity + 1,stamp);
  st[0] = c->type;
  
  // Dynamic arity check
  if(arity != c->arity)
    {
      fail("Signature mismatch\n");
      return NULL;
    }
  // Dynamic sort checks
  for (i = 0; i < arity; i++)
    {
      if ( c->sig[i].sort != exps[i]->sort)
	{
	  fail("Signature mismatch\n");
	  return NULL;
	}
      get_stamp = get_sort_stamp(c->sig[i].sort);
      st[i+1] = get_stamp(exps[i]);
    }

  // Hash-consing of terms
  if (!(result = (cons_expr)term_hash_find(sort_hash,st,arity+1)) 
      || arity == 0 )
    {
      gen_e *e = rarrayalloc(sort_region,arity,gen_e);
      
      if (arity)
	memcpy(e,exps,sizeof(gen_e)*arity);
      else 
	e = NULL;

      result = ralloc(sort_region,struct cons_expr_);  
      result->type = st[0];
      result->st = stamp_fresh();
      result->sort = c->sort;
      result->arity = c->arity;
      result->name = c->name;
      result->sig = c->sig;
      result->exps = e;
      result->c = c;
      
      term_hash_insert(sort_hash,(gen_e)result,st,arity+1);
    }

  return (gen_e)result;
}

static gen_e make_proj_pat(constructor c, int i, gen_e e)
{
  proj_pat pat;
  region sort_region = get_sort_region(e->sort);
  term_hash sort_hash = get_sort_hash(e->sort);
  get_stamp_fn_ptr get_stamp = get_sort_stamp(e->sort);
  
  stamp s[3];
  s[0] = c->type + 1;
  s[1] = get_stamp(e);
  s[2] = i;

  if (! (pat = (proj_pat)term_hash_find(sort_hash,s,3)) )
    {
      pat = ralloc(sort_region,struct proj_pat_);
      pat->type = s[0];
      pat->st = stamp_fresh();
      pat->sort = c->sort;
      pat->exp = e;
      pat->variance = c->sig[i].variance;
      pat->c = c;
      pat->i = i;
      term_hash_insert(sort_hash,(gen_e)pat,s,3);
    }
  
  return (gen_e)pat;
}

gen_e setif_proj_pat(constructor c,int i,gen_e e)
{
  if (c->sort != setif_sort)
    {
      fail("Sort check failed: proj_pat\n");
      return NULL;
    }
  return make_proj_pat(c,i,e);
}

gen_e setst_proj_pat(constructor c, int i, gen_e e)
{
  if (c->sort != setst_sort)
    {
      fail("Sort check failed: proj_pat\n");
      return NULL;
    }
  return make_proj_pat(c,i,e);
}

/* for proj, sort(e) must be setif */
gen_e setif_proj(constructor c, int i, gen_e e) 
{
  setif_var v;
  gen_e proj_var, proj;

  gen_e nonspec_get_proj(gen_e_list arg1)
    {
      proj_pat pat;
      gen_e_list_scanner scan;
      gen_e temp;
      
      gen_e_list_scan(arg1,&scan);
      while (gen_e_list_next(&scan,&temp))
	{
	  if (! setif_is_pat(temp) ) continue;
	  pat = (proj_pat)temp;
	  if ( pat_match(pat->type,c->type) && i == pat->i )
	    return pat->exp;
	}
      return NULL;
    }

    if (e->sort != setif_sort)
    {
      fail("Sort check failed: setif_proj\n");
      return NULL;
    }

  else if (i < 0 || i >= c->arity)
    {
      fail("Signature mismatch\n");
      return NULL;
    }

  else if (setif_is_zero(e))
    {
      return get_sort_zero(c->sig[i].sort);
    }
  
  else if ( ((setif_term)e)->type == c->type )
    {
      cons_expr constructed = (cons_expr)e;
      return constructed->exps[i];
    }
  
  else if (setif_is_var(e))
    {
      v = (setif_var)e;
      if ( (proj = sv_get_ub_proj(v,nonspec_get_proj)) )
	{
	  return proj;
	}
      else
	{
	  gen_e pat;
	  gen_e_list_scanner scan;
	  gen_e lb;
	  //	  proj_var = get_proj_var(c->sig[i].sort,FALSE);
	  proj_var = get_vinv_proj_var(c->sig[i].sort,c,i,e);
	  pat = setif_proj_pat(c,i,proj_var);
	  sv_add_ub_proj(v,pat);
	  
	  gen_e_list_scan(sv_get_lbs(v),&scan);
	  while (gen_e_list_next(&scan,&lb))
	    {
	      setif_inclusion_ind(lb,pat);
	    }
	  return proj_var;
	}
    }

  else if (setif_is_union(e))
    {
      if( (proj = nonspec_get_proj(setif_get_proj_cache(e))) )
	return proj;
      else
	{
	  gen_e pat;
	  // proj_var = get_proj_var(c->sig[i].sort,FALSE);
	  proj_var = get_cinv_proj_var(c->sig[i].sort,c,i);
	  pat = setif_proj_pat(c,i,proj_var);
	  
	  setif_set_proj_cache(e,pat);
	  
	  setif_inclusion_ind(e,pat);
	  return proj_var;
	}
    }
  else 
    {
      gen_e pat;
      //     proj_var = get_proj_var(c->sig[i].sort,FALSE);
      proj_var = get_cinv_proj_var(c->sig[i].sort,c,i);
      pat = setif_proj_pat(c,i,proj_var);
      setif_inclusion_ind(e,pat);
      return proj_var;
    }
}

gen_e setst_proj(constructor c, int i, gen_e e)
{
  /* TODO */
  fail("setst_proj\n");
  return NULL;
}

static void setif_con_match(gen_e e1, gen_e e2)
{
  // Case where e1 is a constructor expression and e2 is a gproj_pat
  if (setif_is_cons_expr(e1) && setif_is_gpat(e2) && 
      gpat_match((cons_expr)e1, (gproj_pat)e2) ) {
    cons_expr c = (cons_expr)e1;
    gproj_pat p = (gproj_pat)e2;
    int i = p->i;

    assert(i == -1 || i < c->arity);

    // Calling with i == -1 makes the projection operate over every subterm
    // FIX : check, if this term exists w/ i == -1, it subsumes other pats
    // should they be created?
    if (i == -1) {
      int j;
      
      for (j = 0; j < c->arity; j++) {
	if (c->sig[j].variance == vnc_pos)
	  call_inclusion_ind(c->exps[j],p->exp);
	else if (c->sig[i].variance == vnc_neg)
	  call_inclusion_ind(p->exp,c->exps[j]);
	else
	  call_unify_ind(c->exps[j],p->exp);
      }
    }
    else {
      if (c->sig[i].variance == vnc_pos)
	call_inclusion_ind(c->exps[i],p->exp);
      else if (c->sig[i].variance == vnc_neg)
	call_inclusion_ind(p->exp,c->exps[i]);
      else
	call_unify_ind(c->exps[i],p->exp);
    }
  }
  else if (setif_is_gpat(e2)) { // no match
    return;
  }
  // Case where e1 is a constructor expression and e2 is a proj_pat
  else if (pat_match(((setif_term)e2)->type,((setif_term)e1)->type))
    {
      cons_expr c = (cons_expr)e1;
      proj_pat p = (proj_pat)e2;
      int i = p->i;
      
      if (c->sig[i].variance == vnc_pos)
	call_inclusion_ind(c->exps[i],p->exp);
      else if (c->sig[i].variance == vnc_neg)
	call_inclusion_ind(p->exp,c->exps[i]);
      else
	call_unify_ind(c->exps[i],p->exp);
    }
  else if (setif_is_pat(e2)) 	//  no match
    {
      return;
    }
  
  // Case where e1 and e2 are constructor expressions
  else 
    {
      cons_expr c1 = (cons_expr)e1,
	c2 = (cons_expr)e2;
      
      if (c1->type != c2->type)
	{
	  handle_error(e1,e2,bek_cons_mismatch);
	}
      else
	{
	  int i;
	  for (i = 0; i < c1->arity; i++)
	    {
	      if (c1->sig[i].variance == vnc_pos)
		call_inclusion_ind(e1,e2);
	      else if (c1->sig[i].variance == vnc_neg)
		call_inclusion_ind(e2,e1);
	      else
		call_unify_ind(e1,e2);
	    }
	  
	}
    } 
}


static void setst_con_match(gen_e e1, gen_e e2)
{
  // Case where e1 is a constructor expression and e2 is a proj_pat
  if (pat_match(((setst_term)e2)->type,((setst_term)e1)->type))
    {
      cons_expr c = (cons_expr)e1;
      proj_pat p = (proj_pat)e2;
      int i = p->i;
      
      if (c->sig[i].variance == vnc_pos)
	call_inclusion_ind(c->exps[i],p->exp);
      else if (c->sig[i].variance == vnc_neg)
	call_inclusion_ind(p->exp,c->exps[i]);
      else
	call_unify_ind(c->exps[i],p->exp);
    }
  else if (setst_is_pat(e2)) 
    {
      return;
    }
  
  // Case where e1 and e2 are constructor expressions
  else 
    {
      cons_expr c1 = (cons_expr)e1,
	c2 = (cons_expr)e2;
      
      if (c1->type != c2->type)
	{
	  handle_error(e1,e2,bek_cons_mismatch);
	}
      else
	{
	  int i;
	  for (i = 0; i < c1->arity; i++)
	    {
	      if (c1->sig[i].variance == vnc_pos)
		call_inclusion_ind(e1,e2);
	      else if (c1->sig[i].variance == vnc_neg)
		call_inclusion_ind(e2,e1);
	      else
		call_unify_ind(e1,e2);
	    }
	  
	}
    } 
}

// given x <= proj(c,i,e)
// proj_merge(region,e,get_proj_i_arg,fresh_large_fn_ptr,
// sort_inclusion_fn_ptr,set_inclusion)
static bool setif_res_proj(setif_var v1,gen_e e2)
{
  if (setif_is_pat(e2) ) {
    proj_pat projection_pat;
    projection_pat = (proj_pat)e2;
    
    gen_e setif_get_proj(gen_e_list arg1)
      {
	gen_e_list_scanner scan;
	gen_e temp;
	proj_pat pat;
	
	gen_e_list_scan(arg1,&scan);
	while(gen_e_list_next(&scan,&temp))
	  {
	    if (!setif_is_pat(temp)) continue;
	    pat = (proj_pat)temp;
	    if ( pat->type == ((setif_term)e2)->type && 
		 pat->i == ((proj_pat)e2)->i)
	      return pat->exp;
	  }
	return NULL;
      }
    
    gen_e fresh_large(const char *name)
      {
	return get_proj_var( ((proj_pat)e2)->exp->sort,TRUE);
      }
    
    void sort_inclusion(gen_e e1, gen_e e2)
      {
	if ( projection_pat->variance == vnc_pos )
	  call_inclusion_ind(e1,e2);
	else if ( projection_pat->variance == vnc_neg)
	  call_inclusion_ind(e2,e1);
	else 
	  call_unify_ind(e1,e2);
      }
    
    gen_e proj_con(gen_e e)
      {
	return make_proj_pat( ((proj_pat)e2)->c, ((proj_pat)e2)->i,e);
      }
    
    return setif_proj_merge(v1,((proj_pat)e2)->exp,
			    setif_get_proj,proj_con,
			    fresh_large,sort_inclusion,
			    setif_inclusion_ind);
  }
  else return FALSE;
}

/* Add a field */
flowrow_field flowrow_make_field(const char *name, gen_e e)
{
  flowrow_field result = ralloc(flowrow_region,struct flowrow_field_);
  result->label = rstrdup(flowrow_region,name);
  result->expr = e;
  return result;
}

gen_e flowrow_make_row(flowrow_map fields, gen_e rest)
{
  get_stamp_fn_ptr get_stamp = get_sort_stamp(flowrow_base_sort(rest));
  
  return flowrow_row(get_stamp,fields,rest);
}

/* Does a sort check */
int call_setif_inclusion(gen_e e1,gen_e e2)
{
  
  if (! ( (e1->sort == e2->sort) && (e1->sort == setif_sort) ) )
    {
      fail("Sort check failed during setif inclusion\n");
    }

  setif_inclusion(setif_con_match,setif_res_proj,e1,e2);
  return 0;
}

/* Does a sort check */
int call_setif_unify(gen_e e1, gen_e e2)
{
  
  if (! ( (e1->sort == e2->sort) && (e1->sort == setif_sort) ) )
    {
      fail("Sort check failed during setif_unify\n");
    }

  setif_inclusion(setif_con_match,setif_res_proj,e1,e2);
  setif_inclusion(setif_con_match,setif_res_proj,e2,e1);
  return 0;
}

/* Does a sort check */
int call_setst_inclusion(gen_e e1, gen_e e2)
{
  
  if (! ( (e1->sort == e2->sort) && (e1->sort == setst_sort) ) )
    {
      fail("Sort check failed: setif_inclusion\n");
    }
  
  setst_inclusion(setst_con_match,e1,e2);
  return 0;
}

/* Does a sort check */
int call_setst_unify(gen_e e1, gen_e e2)
{
  
  if (! ( (e1->sort == e2->sort) && (e1->sort == setst_sort) ) )
    {
      fail("Sort check failed: setst_unify\n");
    }

  setst_inclusion(setst_con_match,e1,e2);
  setst_inclusion(setst_con_match,e2,e1);
  return 0;
}

static void flowrow_inclusion_ind(gen_e e1, gen_e e2)
{
  fresh_fn_ptr fresh;
  get_stamp_fn_ptr get_stamp;
  incl_fn_ptr field_incl;
  gen_e zero_elem;
  
  if (flowrow_base_sort(e1) != flowrow_base_sort(e2))
    fail("Row base sorts do not match\n");

   switch(flowrow_base_sort(e2))
    {
    case setif_sort:
      {
	fresh = setif_fresh;
	get_stamp = setif_get_stamp;
	field_incl = setif_inclusion_ind;
	zero_elem = setif_zero();
      }
      break;
    case setst_sort:
      {
	fresh = setst_fresh;
	get_stamp = setst_get_stamp;
	field_incl = setst_inclusion_ind;
	zero_elem = setst_zero();
      }
      break;
    case term_sort:
      {
	fresh = term_fresh;
	get_stamp = term_get_stamp;
	field_incl = term_unify_ind;
	zero_elem = term_zero();
      }
      break;
    default:
      {
	fresh = NULL;
	get_stamp = NULL;
	field_incl = NULL;
	zero_elem = NULL;
	fail("Flowrow inclusion: unmatched base sort\n");
      }
      break;
    }

  flowrow_inclusion(fresh,get_stamp,field_incl,zero_elem,e1,e2);
}

/* Does a sort check */
int call_flowrow_inclusion(gen_e e1,gen_e e2)
{
  
  if ( (e1->sort != flowrow_sort) || (e2->sort != flowrow_sort) )
    {
      fail("Sort check failed: flowrow_inclusion\n");
    }
  if ( flowrow_base_sort(e1) != flowrow_base_sort(e2))
    {
      fail("Base sort check failed: flowrow_inclusion\n");
    }

  
  flowrow_inclusion_ind(e1,e2);
  return 0;
}

/* Does a sort check */
int call_flowrow_unify(gen_e e1, gen_e e2)
{
  
  if ( (e1->sort != flowrow_sort) || (e2->sort != flowrow_sort) )
    {
      fail("Sort check failed: flowrow_inclusion\n");
    }
  if ( flowrow_base_sort(e1) != flowrow_base_sort(e2))
    {
      fail("Base sort check failed: flowrow_inclusion\n");
    }

  flowrow_inclusion_ind(e1,e2);
  flowrow_inclusion_ind(e2,e1);
  return 0;
}

static void term_con_match(gen_e e1, gen_e e2)
{
  cons_expr c1 = (cons_expr)e1,
    c2 = (cons_expr)e2;
  
  if (c1->type != c2->type)
    {
      handle_error(e1,e2,bek_cons_mismatch);
     }
  else
    {
      int i;
      for (i = 0; i < c1->arity; i++)
	{
	  call_unify_ind(c1->exps[i],c2->exps[i]);
	}
      
    }
}

static bool term_occurs(term_var v, gen_e e)
{
  gen_e ecr = term_get_ecr(e);
  
  if (((gen_term)ecr)->type == VAR_TYPE)
    return ( term_get_stamp((gen_e)v) == term_get_stamp(e) );

  else if (((gen_term)ecr)->type >= smallest_special_type)
    {
      cons_expr c_e = (cons_expr) e;
      int i;
      for (i = 0; i < c_e->arity; i++)
	{
	  if (term_occurs(v,c_e->exps[i]))
	    return TRUE;
	}
    }
  
  return FALSE;
}


int call_term_unify(gen_e e1, gen_e e2)
{
  
  if ( (e1->sort != term_sort) || (e2->sort != term_sort) )
    {
      fail("Sort check failed: term_unify\n");
    }

  term_unify(term_con_match,term_occurs,e1,e2);
  return 0;
}

int call_term_cunify(gen_e e1, gen_e e2)
{
  
  if ( (e1->sort != term_sort) || (e2->sort != term_sort) )
    {
      fail("Sort check failed: term_unify\n");
    }

  term_cunify(term_con_match,term_occurs,e1,e2);
  return 0;
}

static void call_inclusion_ind(gen_e e1, gen_e e2)
{
  nonspec_stats_.dispatches++;

  switch (e1->sort)
    {
    case setif_sort:
      {
	setif_inclusion(setif_con_match,setif_res_proj,e1,e2);
      }
      break;
    case setst_sort:
      {
	setst_inclusion(setst_con_match,e1,e2);
      }
      break;
    case term_sort:
      {
	term_unify(term_con_match,term_occurs,e1,e2);
      }    
      break;
    case flowrow_sort:
      {
	flowrow_inclusion_ind(e1,e2);
      }
      break;
    default :
      fail("Unmatched sort in call inclusion\n");
    }
  return;
}

static void call_unify_ind(gen_e e1, gen_e e2)
{
 nonspec_stats_.dispatches++;

  switch (e1->sort)
    {
    case setif_sort:
      {
	setif_inclusion(setif_con_match,setif_res_proj,e1,e2);
	setif_inclusion(setif_con_match,setif_res_proj,e2,e1);
      }
      break;
    case setst_sort:
      {
	setst_inclusion(setst_con_match,e1,e2);
	setst_inclusion(setst_con_match,e2,e1);
      }
      break;
    case term_sort:
      {
	term_unify(term_con_match,term_occurs,e1,e2);
      }    
      break;
    case flowrow_sort:
      {
	flowrow_inclusion_ind(e1,e2);
	flowrow_inclusion_ind(e2,e1);
      }
      break;
    default :
      fail("Unmatched sort in call inclusion\n");
    }
  return;
}

// Returns true if c is NULL, otherwise, checks to see if expr_type
// matches c's type
static bool check_cons_match(constructor c, int expr_type)
{
  if (c) return c->type == expr_type;
  else return TRUE;
}

static struct decon deconstruct_expr_aux(constructor c,gen_e e)
{
  switch (e->sort)
    {
    case setif_sort:
      {
	if ( setif_is_cons_expr(e) && check_cons_match(c,((setif_term)e)->type) )
	  {
	    cons_expr ce = (cons_expr)e;
	    gen_e *elems = rarrayalloc(get_sort_region(e->sort),ce->arity,
				       gen_e);
	    memcpy(elems,ce->exps,sizeof(gen_e)*ce->arity);
	    return (struct decon){ce->name,ce->arity,elems};
	  }
	else goto NONE;
      }
      break;
    case setst_sort:
      {
	if ( setst_is_cons_expr(e) && check_cons_match(c,((setst_term)e)->type) )
	  {
	    cons_expr ce = (cons_expr)e;
	    gen_e *elems = rarrayalloc(get_sort_region(e->sort),ce->arity,
				       gen_e);
	    memcpy(elems,ce->exps,sizeof(gen_e)*ce->arity);
	    return (struct decon){ce->name,ce->arity,elems};
	  }
	else goto NONE;
      }
      break;
    case term_sort:
      {
	if ( term_is_cons_expr(e) && 
	     check_cons_match(c, ((gen_term)term_get_ecr(e))->type) )
	  {
	    cons_expr ce = (cons_expr)term_get_ecr(e);
	    gen_e *elems = rarrayalloc(get_sort_region(e->sort),ce->arity,
				       gen_e);
	    memcpy(elems,ce->exps,sizeof(gen_e)*ce->arity);
	    return (struct decon){ce->name,ce->arity,elems};
	  }
	else goto NONE;
      }
      break;
    case flowrow_sort:
    default:
      {
	goto NONE;
      }
    }
  
 NONE:
  return (struct decon){NULL,-1,NULL}; // FIX : is it ok to have changed this to -1??
}

struct decon deconstruct_expr(constructor c,gen_e e)
{
  return deconstruct_expr_aux(c,e);
}

struct decon deconstruct_any_expr(gen_e e)
{
  return deconstruct_expr_aux(NULL,e);
}


void nonspec_init(void)
{
  engine_init();
  term_init();
  setif_init();
  setst_init();
  flowrow_init();
}

void nonspec_reset(void)
{
  flowrow_reset();
  setst_reset();
  setif_reset();
  term_reset();
  engine_reset();
}

static void cons_expr_print(FILE *f, cons_expr ce)
{
  if (ce->arity == 0)
    fprintf(f,"%s",ce->name);
  else
    {
      int i;
      fprintf(f,"%s(",ce->name);
      expr_print(f,ce->exps[0]);
      for (i = 1; i < ce->arity; i++)
	{
	  fprintf(f,",");
	  expr_print(f,ce->exps[i]);
	}
      fprintf(f,")");
    }
}

static void pat_print(FILE *f, proj_pat pp)
{
  fprintf(f,"Proj[%s,%d,",pp->c->name,pp->i);
  expr_print(f,pp->exp);
  fprintf(f,"]");
}

static void gpat_print(FILE *f, gproj_pat pp)
{
  fprintf(f,"GProj[%s,%d,",pp->g->name,pp->i);
  expr_print(f,pp->exp);
  fprintf(f,"]");
}

static void setif_union_print(FILE *f, gen_e e)
{
  gen_e_list_scanner scan;
  gen_e temp;
  int i = 0;

  gen_e_list_scan(setif_get_union(e),&scan);
  while (gen_e_list_next(&scan,&temp))
    {
      if (i++ == 0)
	expr_print(f,temp);
      else
	{
	  fprintf(f," || ");
	  expr_print(f,temp);
	}
    }
}

static void setst_union_print(FILE *f, gen_e e)
{
  gen_e_list_scanner scan;
  gen_e temp;
  int i = 0;

  gen_e_list_scan(setst_get_union(e),&scan);
  while (gen_e_list_next(&scan,&temp))
    {
      if (i++ == 0)
	expr_print(f,temp);
      else
	{
	  fprintf(f," || ");
	  expr_print(f,temp);
	}
    }
}

static void setif_inter_print(FILE *f, gen_e e)
{
  gen_e_list_scanner scan;
  gen_e temp;
  int i = 0;

  gen_e_list_scan(setif_get_inter(e),&scan);
  while (gen_e_list_next(&scan,&temp))
    {
      if (i++ == 0)
	expr_print(f,temp);
      else
	{
	  fprintf(f," && ");
	  expr_print(f,temp);
	}
    }
}

static void setst_inter_print(FILE *f, gen_e e)
{
  gen_e_list_scanner scan;
  gen_e temp;
  int i = 0;

  gen_e_list_scan(setst_get_inter(e),&scan);
  while (gen_e_list_next(&scan,&temp))
    {
      if (i++ == 0)
	expr_print(f,temp);
      else
	{
	  fprintf(f," && ");
	  expr_print(f,temp);
	}
    }
}


void expr_print(FILE *f,gen_e e)
{
  switch(e->sort)
    {
    case setif_sort:
      {
	if (setif_is_var(e))
	  {
	    //   fprintf(f,"%s[%d]", sv_get_name((setif_var)e),setif_get_stamp(e));
	    fprintf(f,"%s::%d", sv_get_name((setif_var)e),sv_get_stamp((setif_var)e));
	  }
	else if (setif_is_zero(e))
	  {
	    fprintf(f,"0");
	  }
	else if (setif_is_one(e))
	  {
	    fprintf(f,"1");
	  }
	else if (setif_is_pat(e))
	  {
	    pat_print(f,(proj_pat)e);
	  }
	else if (setif_is_gpat(e)) 
	  {
	    gpat_print(f,(gproj_pat)e);
	  }
	else if (setif_is_union(e))
	  {
	    setif_union_print(f,e);
	  }
	else if (setif_is_inter(e))
	  {
	    setif_inter_print(f,e);
	  }
	else if (setif_is_constant(e))
	  {
	    fprintf(f,"%s()::%d",setif_get_constant_name(e),setif_get_stamp(e));
	  }
	else 
	  {
	    assert(setif_is_cons_expr(e));
	    cons_expr_print(f,(cons_expr)e);
	  }
      }
      break;
 case setst_sort:
      {
	if (setst_is_var(e))
	  {
	    fprintf(f,"%s", st_get_name((setst_var)e));
	  }
	else if (setst_is_zero(e))
	  {
	    fprintf(f,"0");
	  }
	else if (setst_is_one(e))
	  {
	    fprintf(f,"1");
	  }
	else if (setst_is_pat(e))
	  {
	    pat_print(f,(proj_pat)e);
	  }
	else if (setst_is_union(e))
	  {
	    setst_union_print(f,e);
	  }
	else if (setst_is_inter(e))
	  {
	    setst_inter_print(f,e);
	  }
	else 
	  {
	    cons_expr_print(f,(cons_expr)e);
	  }
      }
      break;
    case term_sort:
      {
	gen_e ecr = term_get_ecr(e);
	if (term_is_var(ecr))
	  {
	    fprintf(f,"%s", tv_get_name((term_var)ecr));
	  }
	else if (term_is_zero(ecr))
	  {
	    fprintf(f,"0");
	  }
	else if (term_is_one(ecr))
	  {
	    fprintf(f,"1");
	  }
	else 
	  {
	    cons_expr_print(f,(cons_expr)ecr);
	  }
      }
      break;
    case flowrow_sort:
      {
	flowrow_print(f,get_sort_stamp(flowrow_base_sort(e)),expr_print,e);
      }
      break;
    default:
      {
	fail("Unmatched sort: expr_print\n");
      }
      break;
    }
}

static void print_nonspec_stats(FILE *f)
{
  
}

void nonspec_stats(FILE *f)
{
  print_nonspec_stats(f);
  engine_stats(f);
}

bool expr_eq(gen_e e1, gen_e e2)
{
  if (e1->sort != e2->sort) {
    return FALSE;
  }
  else {
    bool result = FALSE;
    get_stamp_fn_ptr get_stamp = get_sort_stamp(e1->sort);
    result = (get_stamp(e1) == get_stamp(e2));
    return result;
  }
}

bool expr_is_constant(gen_e e)
{
  if (e->sort != setif_sort) return FALSE;

  return setif_is_constant(e);
}

char *expr_constant_name(gen_e e)
{
  if (! expr_is_constant(e)) return NULL;

  return setif_get_constant_name(e);
}


int expr_stamp(gen_e e)
{
  get_stamp_fn_ptr get_stamp = get_sort_stamp(e->sort);
  return get_stamp(e);
}

void register_error_handler(banshee_error_handler_fn error_handler)
{
  handle_error = error_handler;
}

/* Groups */
cons_group make_cons_group(const char *name, sig_elt s[], int arity)
{
  static int next_gid = 0;
  cons_group g = ralloc(setif_region, struct cons_group_);
  sig_elt *sig = NULL;
  
  if (arity > 0) {
    sig = rarrayalloc(setif_region, arity, sig_elt);
    memcpy(sig,s,sizeof(sig_elt)*arity);
  }
  
  g->arity = arity;
  g->name = rstrdup(setif_region,name);
  g->sig = sig;
  g->gid = next_gid++;

  return g;
}

void cons_group_add(cons_group g, constructor c)
{
  int i;
  if (c->sort != setif_sort) {
    fail("Attempted to add %s to group %s, but %s is not a setif constructor", 
	 c->name, g->name); 
  }

  if (c->arity != g->arity && g->arity != -1) {
    fail("Attempted to add %s to group %s, but there was an arity mismatch",
	 c->name, g->name);
  }
  
  if (g->arity != -1) {
    for (i = 0; i < c->arity; i++) {
      if (g->sig[i].variance != c->sig[i].variance || g->sig[i].sort != c->sig[i].sort) {
	fail("Attempted to add %s to group %s, but there was a signature mismatch",
	     c->name, g->name);
      }
    }
  }

  assert(c->groups);
  cons_group_list_cons(g,c->groups);
}

static gen_e make_group_proj_pat(cons_group g, int i, gen_e e)
{
  gproj_pat pat;
  region sort_region = get_sort_region(e->sort);
  term_hash sort_hash = get_sort_hash(e->sort);
  get_stamp_fn_ptr get_stamp = get_sort_stamp(e->sort);
  
  stamp s[4];
  s[0] = GROUP_PROJ_PAT_TYPE;
  s[1] = g->gid;
  s[2] = get_stamp(e);
  s[3] = i;

  if (! (pat = (gproj_pat)term_hash_find(sort_hash,s,4)) )
    {
      pat = ralloc(sort_region,struct gproj_pat_);
      pat->sort = setif_sort;
      pat->type = s[0];
      pat->st = stamp_fresh();
      pat->exp = e;


      //pat->variance = g->sig[i].variance;
      pat->g = g;
      pat->i = i;
      term_hash_insert(sort_hash,(gen_e)pat,s,4);
    }
  
  return (gen_e)pat;
}

gen_e setif_group_proj_pat(cons_group g, int i, gen_e e)
{
  return make_group_proj_pat(g,i,e);
}

// FIX : can we take advantage of projection merging here?
// gen_e setif_group_proj(cons_group g, int i, gen_e e)
// {
// }



