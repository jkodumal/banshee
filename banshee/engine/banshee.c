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
#include <regions.h>
#include "banshee.h"
#include "setif-sort.h"
#include "setst-sort.h"
#include "flowrow-sort.h"
#include "setif-var.h"
#include "ufind.h"


DEFINE_LIST(gen_e_list,gen_e);
// DEFINE_LIST(int_list,int);
// DEFINE_LIST(string_list,char*);

void engine_init(void)
{
  region_init(); 
  stamp_init();
  uf_init();
}

void engine_reset(void) deletes
{
  stamp_reset();
}

/* TODO */
void engine_update(void)
{
}

void engine_stats(FILE *f)
{
  setif_print_stats(f);
  setst_print_stats(f);
  flowrow_print_stats(f);
}

void print_constraint_graphs(FILE *f)
{
  setif_print_constraint_graph(f);
}

static void default_error_handler(gen_e e1, gen_e e2,banshee_error_kind k)
{
  fail("Unhandled banshee error: code %d\n",k);
}

banshee_error_handler_fn handle_error = default_error_handler;
