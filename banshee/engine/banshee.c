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
#include "regions.h"
#include "banshee.h"
#include "setif-sort.h"
#include "setst-sort.h"
#include "flowrow-sort.h"
#include "setif-var.h"
#include "term-sort.h"
#include "ufind.h"
#include "list.h"

DECLARE_LIST(banshee_rollback_stack, banshee_rollback_info);
DEFINE_LIST(banshee_rollback_stack, banshee_rollback_info);
DEFINE_LIST(gen_e_list ,gen_e);

static int banshee_clock = 0;
static banshee_rollback_stack rb_stack;
static region engineregion;

void engine_init(void)
{
  region_init(); 
  stamp_init();
  uf_init();

  engineregion = newregion();
  rb_stack = new_banshee_rollback_stack(engineregion);
}

void engine_reset(void) deletes
{
  stamp_reset();
  banshee_clock = 0;
  deleteregion(engineregion);
  engineregion = newregion();
  rb_stack = new_banshee_rollback_stack(engineregion);
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

void banshee_clock_tick()
{
  banshee_clock++;
  uf_tick();
}

/* Return TRUE if there is already a rollback entry for this sort at
   the current time */
bool banshee_check_rollback(sort_kind k)
{
  banshee_rollback_stack_scanner scan;
  banshee_rollback_info info;

  banshee_rollback_stack_scan(rb_stack,&scan);

  while(banshee_rollback_stack_next(&scan,&info)) {
    if (info->time.time != banshee_clock) return FALSE;
    else if (info->kind == k) return TRUE;
  }
  return FALSE;
}

void banshee_register_rollback(banshee_rollback_info info)
{
  /* This rollback must not be present already */
  assert(!banshee_check_rollback(info->kind));
  
  banshee_rollback_stack_cons(info, rb_stack);
}

static void banshee_rollback_dispatch(banshee_rollback_info info) {
  switch(info->kind) {
  case flowrow_sort:
    flowrow_rollback(info);
    break;
  case setif_sort:
    setif_rollback(info);
    break;
  case setst_sort:
    setst_rollback(info);
    break;
  case flowterm_sort:
    assert(0);
    break;
  case term_sort:
    term_rollback(info);
    break;
  default:
    fail("Unknown sort in banshee_rollback_dispatch.\n");
  }
}

void banshee_backtrack()
{
  banshee_rollback_stack_scanner scan;
  banshee_rollback_info info;
  
  uf_rollback();
  
  banshee_rollback_stack_scan(rb_stack,&scan);
  
  while(banshee_rollback_stack_next(&scan,&info)) {
    if (info->time.time < banshee_clock) break;
    banshee_rollback_dispatch(info);
  }
  banshee_clock--;
}

banshee_error_handler_fn handle_error = default_error_handler;

