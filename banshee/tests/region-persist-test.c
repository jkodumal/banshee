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

/* Test persistence functionality at the region level */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "regions.h"
#include "persist.h"
#include "hash.h"
#include "utils.h"

#define NUM_NODES 100000

struct node_ {
  int data;
  struct node_ *next;
};

typedef struct node_ *node;

int update_node(translation t, void *m) {
  update_pointer(t, (void **) &((struct node_ *) m)->next);
  return(sizeof(struct node_));
}

int verify()
{
  int j = 0;
  char str[512];
  node next_node;
  Updater u[4];
  translation t;
  hash_table table; 
  region temp = newregion();

  u[0] = update_bucket;
  u[1] = update_hash_table;
  u[2] = update_bucketptr;
  u[3] = update_node;

  t = deserialize("data", "offsets", u, temp);
  table = (hash_table) translate_pointer(t, (void *)table);
  
  for (j = 0; j < NUM_NODES; j++) {
    snprintf(str, 512, "node(%d)", j);
    hash_table_lookup(table, str, (hash_data *)&next_node);
    if (next_node->data != j) {
      return 0;
    }
  }
  
  return 1;
}

int main(int argc, char *argv[])
{
  region r[5];
  int i = 0;
  node n = NULL;
  hash_table table;
  region node_region = newregion();
  
  region_init();
  hash_table_init();

  r[0] = bucket_region;
  r[1] = table_region;
  r[2] = bucketptr_region;
  r[3] = node_region;
  r[4] = NULL;

  table = make_persistent_string_hash_table(table_region, 8, 1);

  for(i = 0; i < NUM_NODES; i++) {
    char str[512];
    node prev = n;
    n = ralloc(node_region, struct node_);
    n->data = i;
    n->next = prev;

    snprintf(str, 512,"node(%d)", i);

    hash_table_insert(table, (hash_key)str, (hash_data)n);
  }

  serialize(r, "data", "offsets");
  if (!verify()) {
    printf("Failed region persist test\n");
    exit(1);
  }
  else {
    printf("Passed region persist test\n");
    exit(0);
  }
}
