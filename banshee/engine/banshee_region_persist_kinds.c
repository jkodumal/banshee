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

#include "banshee_region_persist_kinds.h"

/* A region containing pointers. The update function will call
   update_pointer on adjacent word values */
region banshee_ptr_region = NULL;

/* A region containing non pointers. The update function won't touch
   any of these values */
region banshee_nonptr_region = NULL;

const int num_persistent_regions = NUM_REGIONS;

void banshee_region_persistence_init()
{
  region_init();

  banshee_ptr_region = newregion();
  banshee_nonptr_region = newregion();
}

int update_nonptr_data(translation t, void *m)
{
  return sizeof(void *);
}

int update_ptr_data(translation t, void *m)
{
  update_pointer(t, &m);
  return sizeof(void *);
}

region *get_persistent_regions()
{
#ifndef NONSPEC
  return NULL;
#else
  region *result = rarrayalloc(permanent, NUM_REGIONS+1, region);
  result[0] = banshee_nonptr_region;
  result[1] = banshee_ptr_region;
  result[2] = bucket_region;
  result[3] = table_region;	
  result[4] = strbucket_region;

  result[5] = list_header_region; 
  result[6] = list_node_region;  
  result[7] = list_strnode_region;
  result[8] = uf_element_region; 
  result[9] = ustack_element_region;

  result[10] = bounds_region;
  result[11] = added_edge_info_region;
  result[12] = setif_var_region;	
  result[13] = sv_info_region;
  result[14] = hash_entry_region;

  result[15] = term_bucket_region;
  result[16] = term_hash_region;
  result[17] = setif_term_region;
  result[18] = added_ub_proj_info_region;  
  result[19] = setif_rollback_info_region; 

  result[20] = term_var_region;	
  result[21] = flowrow_rollback_info_region; 
  result[22] = flowrow_region;	   
  result[23] = flowrow_field_region;
  result[24] = contour_region;	    

  result[25] = flow_var_region;	
  result[26] = proj_pat_region;	
  result[27] = gproj_pat_region;	
  result[28] = constructor_region; 
  result[29] = cons_group_region;  

  result[30] = cons_expr_region;	
  result[31] = term_constant_region; 
  result[32] = NULL;
  return result;
#endif	/* NONSPEC */
}

Updater *get_updater_functions()
{
#ifndef NONSPEC
  return NULL;
#else 
  Updater *result = rarrayalloc(permanent, NUM_REGIONS, Updater);

  result[0] = update_nonptr_data;
  result[1] = update_ptr_data;
  result[2] = update_bucket;
  result[3] = update_hash_table;
  result[4] = update_strbucket;
 
  result[5] = update_list_header;
  result[6] = update_list_node;
  result[7] = update_list_strnode;
  result[8] = update_uf_element;
  result[9] = update_ustack_element;
  
  result[10] = update_bounds;
  result[11] = update_added_edge_info;
  result[12] = update_setif_var;
  result[13] = update_sv_info;
  result[14] = update_hash_entry_region; 
  
  result[15] = update_term_bucket;
  result[16] = update_term_hash;
  result[17] = update_setif_term;
  result[18] = update_added_ub_proj_info;
  result[19] = update_setif_rollback_info;
  
  result[20] = update_term_var;
  result[21] = update_flowrow_rollback_info;
  result[22] = update_flowrow;
  result[23] = update_flowrow_field;
  result[24] = update_contour;
  
  result[25] = update_flow_var;
  result[26] = update_proj_pat;
  result[27] = update_gproj_pat;
  result[28] = update_constructor;
  result[29] = update_cons_group;

  result[30] = update_cons_expr;
  result[31] = update_term_constant;

  return result;
#endif  /* NONSPEC  */
}
