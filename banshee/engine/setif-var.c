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
#include "regions.h"
#include "setif-var.h"
#include "ufind.h"
#include "bounds.h"
#include "banshee_persist_kinds.h"

struct sv_info
{
  stamp st;
  bounds sameregion lbs;
  bounds sameregion ubs;
  jcoll tlb_cache;
  gen_e_list ub_projs;
  uf_element component;
  char *name;
};

typedef struct sv_info *sv_info;

DECLARE_UFIND(sv_elt,sv_info);
 
DEFINE_UFIND(sv_elt,sv_info);

DEFINE_LIST(setif_var_list,setif_var);

#define get_info(v) (sv_elt_get_info((v)->elt))

struct setif_var /* extends gen_e */
{
#ifdef NONSPEC
  sort_kind sort;
#endif
  int type;
  sv_elt elt;
};

static region sv_region = NULL;

void sv_init(void)
{
  sv_region = newregion();
}

void sv_reset(void)
{
  deleteregion(sv_region);
  sv_region = newregion();
}
    
bool sv_lt(setif_var v1, setif_var v2)
{
  return ( sv_get_stamp(v1) < sv_get_stamp(v2) );
}

bool sv_eq(setif_var v1, setif_var v2)
{
  return ( sv_get_stamp(v1) == sv_get_stamp(v2) );
}

static setif_var make_var(region r, const char *name, stamp st)
{
  setif_var result = ralloc(r, struct setif_var);
  sv_info info = ralloc(r, struct sv_info);
 
  info->st = st;
  info->lbs = bounds_create(r);
  info->ubs = bounds_create(r);
  info->tlb_cache = NULL;
  info->ub_projs = new_gen_e_list(r);
  info->name = name ? rstrdup(r,name) : "fv";
  info->component = new_uf_element(r, NULL, BANSHEE_PERSIST_KIND_null);

  result->type = VAR_TYPE;
  result->elt = new_sv_elt(r,info); 
  
#ifdef NONSPEC
  result->sort = setif_sort;
#endif  
  
  return result;
}

setif_var sv_fresh(region r, const char *name)
{
  return make_var(r,name,stamp_fresh());
}

setif_var sv_fresh_large(region r, const char *name)
{
  return make_var(r,name,stamp_fresh_large());
}

setif_var sv_fresh_small(region r, const char *name)
{
  return make_var(r,name,stamp_fresh_small());
}

stamp sv_get_stamp(setif_var v)
{
  return get_info(v)->st;
}

char *sv_get_name(setif_var v)
{
  return get_info(v)->name;
}

static sv_info combine(const sv_info i1, const sv_info i2)
{
  if (i1->st < i2->st)
    return i1;
  else return i2;
}

void sv_unify(setif_var v,setif_var_list vars)
{
  setif_var temp;
  setif_var_list_scanner scan;
  
  setif_var_list_scan(vars,&scan);

  while (setif_var_list_next(&scan,&temp))
    {
      sv_elt_unify(combine,v->elt,temp->elt);
    }
}

bounds sv_get_lbs(setif_var v)
{
  return get_info(v)->lbs;
}

bounds sv_get_ubs(setif_var v)
{
  return get_info(v)->ubs;
}

bool sv_add_ub(setif_var v, gen_e e, stamp s)
{
  return bounds_add(get_info(v)->ubs,e,s);
}

bool sv_add_lb(setif_var v, gen_e e, stamp s)
{
  return bounds_add(get_info(v)->lbs,e,s);
}

bool sv_is_ub(setif_var v, stamp s)
{
  bool self_edge = sv_get_stamp(v) == s,
    in_bounds = bounds_query(get_info(v)->ubs,s);
  
  return (self_edge || in_bounds);
}

bool sv_is_lb(setif_var v, stamp s)
{

  bool self_edge = sv_get_stamp(v) == s,
    in_bounds = bounds_query(get_info(v)->lbs,s);
  
  return (self_edge || in_bounds);
}

void sv_add_ub_proj(setif_var v, gen_e e)
{
  gen_e_list_cons(e,get_info(v)->ub_projs);
}

gen_e sv_get_ub_proj(setif_var v, get_proj_fn_ptr get_proj)
{
  return get_proj(get_info(v)->ub_projs);
}

gen_e_list sv_get_ub_projs(setif_var v)
{
  return get_info(v)->ub_projs;
}

bool sv_union_component(setif_var v1, setif_var v2)
{
  if (uf_eq(get_info(v1)->component,get_info(v2)->component))
    return FALSE;
  
  else
    {
      uf_union(get_info(v1)->component,get_info(v2)->component);
      return TRUE;
    }
}

void sv_set_tlb_cache(setif_var v, jcoll j)
{
  get_info(v)->tlb_cache = j;
}

jcoll sv_get_tlb_cache(setif_var v)
{
  return get_info(v)->tlb_cache;
}

void sv_clear_tlb_cache(setif_var v)
{
  get_info(v)->tlb_cache = NULL;
}

/* Persistence support for setif_var */
bool sv_serialize(FILE *f, void *obj)
{
  setif_var var;
  assert(f);
  assert(obj);

  var = (setif_var)obj;

#ifdef NONSPEC
  fwrite((void *)&var->sort, sizeof(int), 1, f);
#endif /* NONSPEC */

  /* No need to serialize the type, since it must be VAR_TYPE */
  
  /* Serialize the sv_elt's id */
  fwrite((void *)&var->elt, sizeof(void *), 1, f);

  /* Mark the uf element for serialization */
  serialize_banshee_object(uf_element, var->elt);

  return TRUE;
}

void *sv_deserialize(FILE *f)
{
  setif_var var;
  assert(f);

  var = ralloc(sv_region, struct setif_var);

#ifdef NONSPEC
  fread((void*)&var->sort, sizeof(int), 1, f);
#endif /* NONSPEC */
  
  var->type = VAR_TYPE;
  fread((void*)&var->elt, sizeof(void *), 1, f);

  return var;
}

bool sv_set_fields(void *obj)
{
  setif_var var = (setif_var)obj;
  assert(obj);

  var->elt = deserialize_get_obj(var->elt);

  assert(var->elt);

  return TRUE;
}

/* Persistence support for sv_info */
bool sv_info_serialize(FILE *f, void *obj)
{
  sv_info info = (sv_info) obj;

  assert(info);
  assert(f);

  fwrite((void *)&info->st, sizeof(stamp), 1, f);
  fwrite((void *)&info->lbs, sizeof(bounds), 1, f);
  fwrite((void *)&info->ubs, sizeof(bounds), 1, f);
  fwrite((void *)&info->tlb_cache, sizeof(jcoll), 1, f);
  fwrite((void *)&info->ub_projs, sizeof(gen_e_list), 1, f);
  fwrite((void *)&info->component, sizeof(uf_element), 1, f);
  fprintf(f, "%s",info->name);

  serialize_banshee_object(bounds, info->lbs);
  serialize_banshee_object(bounds, info->ubs);
  serialize_banshee_object(jcoll, info->tlb_cache);
  serialize_banshee_object(list, info->ub_projs);
  serialize_banshee_object(uf_element, info->component);

  return TRUE;
}

void *sv_info_deserialize(FILE *f)
{
  char buf[512];
  assert(f);
  assert(sv_region);

  sv_info info = ralloc(sv_region, struct sv_info);
  
  fread((void *)&info->st, sizeof(stamp), 1, f);
  fread((void *)&info->lbs, sizeof(bounds), 1, f);
  fread((void *)&info->ubs, sizeof(bounds), 1, f);
  fread((void *)&info->tlb_cache, sizeof(jcoll), 1, f);
  fread((void *)&info->ub_projs, sizeof(gen_e_list), 1, f);
  fread((void *)&info->component, sizeof(uf_element), 1, f);
  fscanf(f,"%s",buf);

  info->name = rstrdup(sv_region,buf);

  return info;  
}
