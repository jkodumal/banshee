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

hash_table table;

int update_node(translation t, void *m) {
  update_pointer(t, (void **) &((struct node_ *) m)->next);
  return(sizeof(struct node_));
}

int update_string(translation t, void *m) {
  return((1 + strlen((char *)m)) * sizeof(char));
}

void update()
{
  Updater u[5];
  translation t;
  region temp = newregion();
  
  u[4] = update_string;
  u[3] = update_bucket;
  u[2] = update_hash_table;
  u[1] = update_bucketptr;
  u[0] = update_node;

  t = deserialize("data", "offsets", u, temp);
  table = (hash_table) translate_pointer(t, (void *)table);
}

int verify()
{
  int j = 0;
  char str[512];
  node next_node;
  
  for (j = 0; j < NUM_NODES; j++) {
    snprintf(str, 512, "node(%d)", j);
    hash_table_lookup(table, str, (hash_data *)&next_node);
    if (next_node->data != j) {
      return 0;
    }
  }
  
  return 1;
}

void seed_fn_ptr_table(region r);

int main(int argc, char *argv[])
{
  region r[6];
  int i = 0;
  node n = NULL;
  region node_region, string_region;
  
  region_init();
  hash_table_init();
  seed_fn_ptr_table(newregion());

  node_region = newregion();
  string_region = newregion();

  r[5] = NULL;
  r[4] = string_region;
  r[3] = bucket_region;
  r[2] = table_region;
  r[1] = bucketptr_region;
  r[0] = node_region;

  table = make_persistent_string_hash_table(table_region, 8, 1);

  for(i = 0; i < NUM_NODES; i++) {
    char str[512];
    node prev = n;
    n = ralloc(node_region, struct node_);
    n->data = i;
    n->next = prev;

    snprintf(str, 512,"node(%d)", i);

    hash_table_insert(table, (hash_key)rstrdup(string_region, str), (hash_data)n);
  }

  if (!verify()) {
    printf("Failed region persist test before serialization\n");
    exit(1);
  }

  serialize(r, "data", "offsets");

  update();
  if (!verify()) {
    printf("Failed region persist test\n");
    exit(1);
  }
  else {
    printf("Passed region persist test\n");
    exit(0);
  }

}
