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
#include <stdio.h>
#include "utils.h"
#include "pta.h"
#include "regions.h"

#include "steensgaard_terms.h"

typedef A contents_type;

DECLARE_LIST(contents_type_list, A);
DEFINE_NONPTR_LIST(contents_type_list, A);

/* Implements the interface defined in pta.h */

void pta_init() {
  steensgaard_terms_init();
}

void pta_reset() {
  steensgaard_terms_reset();
}

/* ref(\loc_{id},\alpha_{id}) */
T pta_make_ref(const char *id) {
  alabel_t loc, tag;
  A contents;
  T lvalue;

  tag = alabel_t_fresh(id);
  loc = alabel_t_constant(id);
  contents = T_fresh(id);

  alabel_t_inclusion(loc, tag);
  return ref(tag, contents);  
}

/* If t is ref(...), return its contents, otherwise, build a ref
   constructor and unify it with t, returning its contents */
static A decompose_ref_or_fresh(T t) {
  if (T_is_ref(t)) {
    struct ref_decon decon = ref_decon(t);
    
    return decon.f1;
  }
  else {
    alabel_t tag = alabel_t_fresh("'fv");
    A contents = A_fresh("'fv");
    T_cunify(t, ref(tag, contents));
    return contents;
  }
  assert(0);
  return NULL;
}

/* The no information case  */
T pta_bottom(void) {
  return T_zero();
}

/* Join t1 and t2's contents, but not their tags  */
T pta_join(T t1, T t2) {
  A a1, a2, a;
  a = A_fresh("join");
  a1 = decompose_ref_or_fresh(t1);
  a2 = decompose_ref_or_fresh(t2);
  
  T_cunify(a1, a);
  T_cunify(a2, a);

  return ref(alabel_t_fresh("join_tag"), a);
}

/* ref(_,alpha(ptr,fun)) --> ptr */
T pta_deref(T t) {
  if (T_is_ref(t)) {
    struct ref_decon r_decon = ref_decon(t);
    
    if (A_is_alpha(r_decon.f1)) {
      return (alpha_decon(r_decon.f1)).f0;
    }
    else {
      T ptr = T_fresh("'ptr");
      L fun = L_fresh("'fun");
      A_unify(r_decon.f1, alpha(ptr,fun));
      return ptr;
    }
  }
  else {
    T ptr = T_fresh("'ptr");
    L fun = L_fresh("'fun");
    T_unify(t, ref(alabel_t_fresh("'fv"), alpha(ptr,fun)));
    
    return ptr;
  }
  
  assert(0);
  return NULL;
}

T pta_rvalue(T) {
  if (T_is_ref(t)) {
    struct ref_decon r_decon = ref_decon(t);
    
    if (A_is_alpha(r_decon.f1)) {
      return (alpha_decon(r_decon.f1)).f0;
    }
    else {
      T ptr = T_fresh("'ptr");
      L fun = L_fresh("'fun");
      A_unify(r_decon.f1, alpha(ptr,fun));
      return ptr;
    }
  }

  else {
    T ptr = T_fresh("'ptr");
    L fun = L_fresh("'fun");
    T_cunify(t, ref(alabel_t_fresh("'fv"), alpha(ptr,fun)));
    
    return ptr;
  }
  
  assert(0);
  return NULL;
}

T pta_address(T t1) {
  return ref(alabel_t_fresh("wild"), alpha(t1,L_fresh("wild")));
}

void pta_assignment(T t1, T t2) {
  T_cunify(t2, pta_deref(t1));
}

T pta_make_fun(const char *name, T ret, T_list args) {

}

/* TODO */
T pta_application(T, T_list) {

}

contents_type pta_get_contents(T t) {
  
}

/* TODO */
void pta_pr_ptset(contents_type c) {

}

/* TODO */
int pta_get_ptsize(contents_type) {

}

void pta_serialize(FILE *f, hash_table *entry_points, unsigned long sz)
{ 
  assert(f);
  steensgaard_terms_serialize(f,entry_points,sz);
}

hash_table *pta_deserialize(FILE *f)
{
  assert(f);
  return steensgaard_terms_deserialize(f);
}
