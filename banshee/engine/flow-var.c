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

#include <stdio.h>
#include <assert.h>
#include "banshee.h"
#include "flow-var.h"
#include "ufind.h"
#include "bounds.h"

DECLARE_UFIND(contour_elt,contour);

struct flow_var /* extends gen_e */
{
#ifdef NONSPEC
  sort_kind sort;
#endif
  int type; /* alias or var */
  stamp st;
  gen_e alias;
  bounds sameregion ubs;
  bounds sameregion lbs;
  contour_elt elt; 
  char *name;
  void *extra_info;
  int extra_persist_kind;
};

DEFINE_UFIND(contour_elt,contour);
DEFINE_LIST(flow_var_list, flow_var);

#define get_contour(x) (contour_elt_get_info((x)->elt))

static flow_var make_var(region r,const char *name, stamp st)
{
  flow_var result = ralloc(r,struct flow_var);

  result->type = VAR_TYPE;
  result->st = st;
  result->alias = NULL;
  result->ubs = bounds_create(r);
  result->lbs = bounds_create(r);
  result->elt = new_contour_elt(r,NULL);
  result->name = name ? rstrdup(r,name) : "fv";
  result->extra_info = NULL;
  result->extra_persist_kind = NULL;

#ifdef NONSPEC
  result->sort = flowrow_sort;
#endif

  return result;
}

flow_var fv_fresh(region r, const char *name)
{
  return make_var(r,name,stamp_fresh());
}

flow_var fv_fresh_large(region r, const char *name)
{
  return make_var(r,name,stamp_fresh_large());
}

flow_var fv_fresh_small(region r, const char *name)
{
  return make_var(r,name,stamp_fresh_small());
}

char * fv_get_name(flow_var v)
{
  return v->name;
}

bounds fv_get_lbs(flow_var v)
{
  return v->lbs;
}

bounds fv_get_ubs(flow_var v)
{
  return v->ubs;
}

bool fv_add_ub(flow_var v, gen_e e, stamp st)
{
  return bounds_add(v->ubs,e,st);
}

bool fv_add_lb(flow_var v, gen_e e, stamp st)
{
  return bounds_add(v->lbs,e,st);
}

bool fv_is_ub(flow_var v, stamp st)
{
  bool self_edge = v->st == st,
    in_bounds = bounds_query(v->ubs,st);

  return (self_edge || in_bounds);
}

bool fv_is_lb(flow_var v, stamp st)
{
  bool self_edge = v->st == st,
    in_bounds = bounds_query(v->lbs,st);

  return (self_edge || in_bounds);
}

void fv_set_alias(flow_var v, gen_e e)
{
  assert(v->type == VAR_TYPE);

  v->type = ALIAS_TYPE;
  v->alias = e;
}

void fv_unset_alias(flow_var v)
{
  assert(v->type == ALIAS_TYPE);
  v->type = VAR_TYPE;
  v->alias = NULL;
}

gen_e fv_get_alias(flow_var v)
{
  assert(v->type == ALIAS_TYPE);
  return v->alias;
}

bool fv_has_contour(flow_var v)
{
  return (get_contour(v) != NULL);
}

void fv_set_contour(flow_var v, contour c)
{
  contour_elt_update(v->elt,c);
}

static contour combine_contour(const contour c1, const contour c2)
{
  if (c1 == NULL)
    return c2;
  else if (c2 == NULL)
    return c1;

  else 
    {
      fail("Attempt to unify two distinct contours\n");
      return NULL;
    }

}
void fv_unify_contour(flow_var v1, flow_var v2)
{
  contour_elt_unify(combine_contour,v1->elt,v2->elt);
}


gen_e fv_instantiate_contour(flow_var v) deletes
{
  contour c = get_contour(v);
  return c->instantiate(c->fresh,c->get_stamp,c->shape);
}

void *fv_get_extra_info(flow_var v)
{
  return v->extra_info;
}

void fv_set_extra_info(flow_var v, void *extra_info, int persist_kind)
{
  v->extra_info = extra_info;
  v->extra_persist_kind = persist_kind;
}


/* Persistence */

bool flow_var_serialize(FILE *f, void *obj)
{
  flow_var var = (flow_var)obj;

  assert(f);
  assert(obj);

  fwrite((void *)&var->st, sizeof(stamp), 1, f);
  fwrite((void *)&var->alias, sizeof(gen_e), 1, f);
  fwrite((void *)&var->ubs, sizeof(bounds), 1, f);
  fwrite((void *)&var->lbs, sizeof(bounds), 1, f);
  fwrite((void *)&var->elt, sizeof(contour_elt), 1, f);
  fwrite((void *)&var->extra_info, sizeof(void *), 1, f);
  string_data_serialize(f,var->name);
  
  serialize_banshee_object(var->ubs, bounds);
  serialize_banshee_object(var->lbs, bounds);
  serialize_banshee_object(var->elt, uf_element);
  serialize_object(var->extra_info, var->extra_persist_kind);
  
  return TRUE;
}

void *flow_var_deserialize(FILE *f)
{
  flow_var var = NULL;

  var = ralloc(permanent, struct flow_var);

  fread((void *)&var->st, sizeof(stamp), 1, f);
  fread((void *)&var->alias, sizeof(gen_e), 1, f);
  fread((void *)&var->ubs, sizeof(bounds), 1, f);
  fread((void *)&var->lbs, sizeof(bounds), 1, f);
  fread((void *)&var->elt, sizeof(contour_elt), 1, f);
  fread((void *)&var->extra_info, sizeof(void *), 1, f);
  
  var->name = (char *)string_data_deserialize(f);

  return var;
}

bool flow_var_set_fields(void *obj)
{
  flow_var var = (flow_var)obj;
  assert(var);

  deserialize_set_obj((void **)&var->ubs);
  deserialize_set_obj((void **)&var->lbs);
  deserialize_set_obj((void **)&var->elt);
  if (var->extra_persist_kind != BANSHEE_PERSIST_KIND_nonptr) {
    deserialize_set_obj((void **)&var->extra_info);
  }
  
  return TRUE;
}

bool contour_serialize(FILE *f, void *obj)
{
  contour c = (contour)obj;

  assert(f);
  assert(c);

  fwrite((void *)&c->shape, sizeof(gen_e), 1, f);
  fwrite((void *)&c->fresh, sizeof(fresh_fn_ptr), 1, f);
  fwrite((void *)&c->get_stamp, sizeof(get_stamp_fn_ptr), 1, f);
  fwrite((void *)&c->instantiate, sizeof(contour_inst_fn_ptr), 1, f);

  serialize_banshee_object(c->shape, gen_e);
  serialize_banshee_object(c->fresh, funptr);
  serialize_banshee_object(c->get_stamp, funptr);
  serialize_banshee_object(c->instantiate, funptr);

  return TRUE;
}

void *contour_deserialize(FILE *f)
{
  contour c = ralloc(permanent, struct contour);

  assert(f);

  fread((void *)&c->shape, sizeof(gen_e), 1, f);
  fread((void *)&c->fresh, sizeof(fresh_fn_ptr), 1, f);
  fread((void *)&c->get_stamp, sizeof(get_stamp_fn_ptr), 1, f);
  fread((void *)&c->instantiate, sizeof(contour_inst_fn_ptr), 1, f);

  return c;
}

bool contour_set_fields(void *obj)
{
  contour c = (contour)obj;
  deserialize_set_obj((void **)&c->shape);
  deserialize_set_obj((void **)&c->fresh);
  deserialize_set_obj((void **)&c->get_stamp);
  deserialize_set_obj((void **)&c->instantiate);

  return TRUE;
}
