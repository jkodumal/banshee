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


#include "linkage.h"
#include "regions.h"

EXTERN_C_BEGIN

#define NUM_REGIONS 5

#define NUM_UPDATES 4 


/*****************************************************************************
 *                                                                           *
 *   Initialization                                                          *
 *                                                                           *
 *****************************************************************************/

void banshee_region_persistence_init();

/*****************************************************************************
 *                                                                           *
 *   Region declarations                                                     *
 *                                                                           *
 *****************************************************************************/

/* A region that won't be serialized to disk. */
extern region banshee_ephemeral_region;

/* A region containing pointers. The update function will call
   update_pointer on adjacent word values */
extern region banshee_ptr_region;

/* A region containing non pointers. The update function won't touch
   any of these values */
extern region banshee_nonptr_region;

/* Regions for hash.c */
extern region bucket_region;
extern region table_region;

/* Regions for list.c */
extern region list_header_region;
extern region list_node_region;
extern region list_strnode_region;

/* Update functions for various data types */

/* Update functions for banshee_region_persist_kinds.c */
int update_nonptr_data(translation t, void *m);
int update_ptr_data(translation t, void *m);

/* Update functions for hash.c */
int update_hash_table(translation t, void *m);
int update_bucket(translation t, void *m);
int update_strbucket(translation t, void *m);

/* Update functions for list.c */
int update_list_header(translation t, void *m);
int update_list_node(translation t, void *m);
int update_list_strnode(translation t, void *m);

EXTERN_C_END
