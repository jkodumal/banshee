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

/* Hashtable based bounds implementation */

#include <stdlib.h>
#include <assert.h>
#include "hash.h"
#include "bounds.h"
#include "stamp.h"

struct bounds_ {
  hash_table table;
};

/* From here: http://www.concentric.net/~Ttwang/tech/inthash.htm  */
unsigned long stamp_hash(hash_key key)
{
  unsigned long keyval = (unsigned long)key;
  keyval += (keyval << 12);
  keyval ^= (keyval >> 22);
  keyval += (keyval << 4);
  keyval ^= (keyval >> 9);
  keyval += (keyval << 10);
  keyval ^= (keyval >> 2);
  keyval += (keyval << 7);
  keyval ^= (keyval >> 12);
  return keyval;
}

static bool stamp_eq(hash_key s1, hash_key s2)
{
  return s1 == s2;
}

bounds bounds_create(region r)
{
  bounds result;
  result = ralloc(r, struct bounds_);
  result->table = make_hash_table(r, 8, stamp_hash, stamp_eq);
  
  return result;
}

/* TODO */
gen_e_list bounds_exprs(bounds b)
{
  assert(0);
  return NULL;
}

void bounds_scan(bounds b, bounds_scanner *scan)
{
  hash_table_scan(b->table,&scan->hs);
}

bool bounds_next(bounds_scanner *scan, gen_e *e)
{
  return hash_table_next(&scan->hs,NULL,(hash_data *) e);
}

bool bounds_add(bounds b, gen_e e, stamp st)
{
  return !hash_table_insert(b->table, (hash_key)st, (hash_data) e);
}

/* Returns TRUE if e was in the bounds */
bool bounds_remove(bounds b, stamp st)
{
  return hash_table_remove(b->table,(hash_key)st);
}

bool bounds_query(bounds b, stamp st)
{
  bool result = hash_table_lookup(b->table, (hash_key)st, NULL);
  return result;
}

bool bounds_empty(bounds b)
{
  return (hash_table_size(b->table) == 0);
}

void bounds_delete(bounds b)
{
  hash_table_reset(b->table);
}

/* TODO */
void bounds_set(bounds b, gen_e_list el)
{
  assert(0);
}

int bounds_size(bounds b)
{
  return hash_table_size(b->table);
}
