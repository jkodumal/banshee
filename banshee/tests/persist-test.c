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

/* Test persistence functionality */

#include <string.h>
#include <stdio.h>
#include "regions.h"
#include "persist.h"
#include "assert.h"


#define DEFAULT_FILENAME "persist-test.out"

/* Types */
struct node_ {
  int data;
  struct node_ *left;
  struct node_ *right;
};

typedef struct node_ *node;

/* Globals */
static region test_rgn;

/* Persistence support */
bool node_serialize(FILE *f, void *obj)
{
  node n = (node) obj;
  
  assert(f);
  assert(obj);

  fwrite((void *)&n->data, sizeof(int), 1, f);
  fwrite((void *)&n->left, sizeof(void *), 1, f);
  fwrite((void *)&n->right, sizeof(void *), 1, f);

  serialize_object(0, n->left);
  serialize_object(0, n->right);


  return TRUE;
}

void *node_deserialize(FILE *f)
{
  node n = ralloc(test_rgn, struct node_);
  
  fread((void *)&n->data, sizeof(int), 1, f);
  fread((void *)&n->left, sizeof(void *), 1, f);
  fread((void *)&n->right, sizeof(void *), 1, f);

  return n;
}

bool set_fields(void *obj)
{
  node n = (node) obj;
  n->left = (node) deserialize_get_obj( (void *) n->left);
  n->right = (node) deserialize_get_obj( (void *) n->right);

  return TRUE;
}


/* Main */
static void show_usage(char *progname)
{
  printf("Usage: %s [--out --in]\n", progname);
}

static void serialize()
{
  node n1;
  /* Build a small graph */
  { 				
    node n2,n3,n4;
    n1 = ralloc(test_rgn, struct node_);
    n2 = ralloc(test_rgn, struct node_);
    n3 = ralloc(test_rgn, struct node_);
    n4 = ralloc(test_rgn, struct node_);
    
    n1->data = 1;
    n2->data = 2;
    n3->data = 3;
    n4->data = 4;
    
    n1->left = n2;
    n1->right = n3;
    
    n2->left = n4;
    n2->right = n4;
    
    n3->left = n4;
    n3->right = n4;
    
    n4->left = n1;
    n4->right = n1;
  }

  /* Serialize it */
  {  
    FILE *outfile;
    serialize_fn_ptr serialize_fns[1] = {node_serialize};

    outfile = fopen(DEFAULT_FILENAME, "wb");

    /* Write down the old address of n1 */
    fwrite((void *)&n1, sizeof(void *), 1, outfile);
    
    serialize_start(outfile, serialize_fns, 1);
    serialize_object(0, n1);
    serialize_end();

    fclose(outfile);
  }
}

static void deserialize()
{
}

int main(int argc, char **argv)
{ 
  assert(argc > 0);

  region_init();

  test_rgn = newregion();


  if (argc == 2) {
    /* Serialize */
    if (!strcmp(argv[1], "--out")) {
      serialize();
    }
    /* Deserialize */
    else if (!strcmp(argv[1], "--in")) {
      deserialize();
    }
    else {
      show_usage(argv[0]);
    }
  }
  else {
    show_usage(argv[0]);
  }

  return 0;
}
