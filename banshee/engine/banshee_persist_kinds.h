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

/* Defines a kind for each persistent object type used in Banshee, as
   well as pointers to the functions needed to
   serialize/deserialize/restore pointer-valued fields for each object
   type
*/
#ifndef BANSHEE_PERSIST_KINDS_H
#define BANSHEE_PERSIST_KINDS_H

#include "persist.h"

/* Take each type that should support serialization and prefix it with
   BANSHEE_PERSIST_KIND. Then serialize_banshee_object(object, type)
   will take care of choosing the right kind.
*/
typedef enum banshee_persist_kind_ {
  BANSHEE_PERSIST_KIND_none = 0,  /* for non-persistent data: KEEP THIS 0 */
  BANSHEE_PERSIST_KIND_uf_element, 
  BANSHEE_PERSIST_KIND_hash_bounds, /* TODO */
  BANSHEE_PERSIST_KIND_setif_var,  
  BANSHEE_PERSIST_KIND_sv_info,	 
  BANSHEE_PERSIST_KIND_list,	/* TODO */
  BANSHEE_PERSIST_KIND_bounds,	/* TODO */
  BANSHEE_PERSIST_KIND_contour,	/* TODO */
  BANSHEE_PERSIST_KIND_st_info,	/* TODO */
  BANSHEE_PERSIST_KIND_gen_e,	/* TODO */
  BANSHEE_PERSIST_KIND_banshee_rollback_info, /* TODO */
  BANSHEE_PERSIST_KIND_flow_var,	      /* TODO */
  BANSHEE_PERSIST_KIND_flowrow_field,	      /* TODO */
  BANSHEE_PERSIST_KIND_cons_group,	      /* TODO */
  BANSHEE_PERSIST_KIND_sig_elt_ptr,	      /* TODO */
  BANSHEE_PERSIST_KIND_setst_var,	      /* TODO */
  BANSHEE_PERSIST_KIND_term_var,	      /* TODO */
  BANSHEE_PERSIST_KIND_ustack_elt,	      /* TODO */
} banshee_persist_kind;

#define serialize_banshee_object(object, type)			\
  serialize_object(BANSHEE_PERSIST_KIND_ ## type, object)




#endif /* BANSHEE_PERSIST_KINDS_H */
