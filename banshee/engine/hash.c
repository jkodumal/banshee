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
#include "hash.h"
#include "regions.h"
#include "utils.h"

struct bucket
{
  hash_key key;
  hash_data data;
  struct bucket *next;
};

#define scan_bucket(b, var) for (var = b; var; var = var->next)

struct Hash_table
{
  region r;         		/* Region for this table */
  hash_fn hash;     		/* Function for hashing keys */
  keyeq_fn cmp;     		/* Function for comparing keys */

  unsigned long size;		/* Number of buckets */
  unsigned long log2size;	/* log2 of size */
  unsigned long used;		/* Number of elements */
  bucket *table;		/* Array of (size) buckets */
};

static void rehash(hash_table ht);

/* Make a new hash table, with size buckets initially.  Hash table
   elements are allocated in region rhash. */
hash_table make_hash_table(region r, unsigned long size, hash_fn hash,
			   keyeq_fn cmp)
{
  hash_table result;

  assert(size > 0);
  result = ralloc(r, struct Hash_table);
  result->r = r;
  result->hash = hash;
  result->cmp = cmp;
  result->size = 1;
  result->log2size = 0;
  /* Force size to a power of 2 */
  while (result->size < size)
    {
      result->size *= 2;
      result->log2size++;
    }
  result->used = 0;
  result->table = rarrayalloc(result->r, result->size, bucket);

  return result;
}

/* Make a hash table for strings. */
hash_table make_string_hash_table(region rhash, unsigned long size)
{
  return make_hash_table(rhash, size, (hash_fn) string_hash,
			 (keyeq_fn) string_eq);
}

/* Zero out ht.  Doesn't reclaim bucket space. */
void hash_table_reset(hash_table ht)
{
  unsigned long i;

  ht->used = 0;
  for (i = 0; i < ht->size; i++)
    ht->table[i] = NULL;
}

/* Return the number of entries in ht */
unsigned long hash_table_size(hash_table ht)
{
  return ht->used;
}

#define MAGIC 2*0.6180339987

#define LLMAGIC ((unsigned long long)(MAGIC * (1ULL << (8 * sizeof(unsigned long) - 1))))

/* Return the bucket corresponding to k in ht */
static inline bucket *find_bucket(hash_table ht, hash_key k)
{
  unsigned long hash;

  hash = ht->hash(k);
  hash = hash * LLMAGIC;
  hash = hash >> (8 * sizeof(unsigned long) - ht->log2size);
  if (ht->size == 1)
    hash = 0;
  /* hash = hash % ht->size; */
  assert(hash < ht->size);
  return &ht->table[hash];
}

/* Given a comparison function which agrees with our hash_function,
   search for the given element. */
bool hash_table_hash_search(hash_table ht, keyeq_fn cmp, 
			    hash_key k, hash_data *d)
{
  bucket cur;

  cur = *find_bucket(ht, k);
  while (cur)
    {
      if (cmp(k, cur->key))
	{
	  if (d)
	    *d = cur->data;
	  return TRUE;
	}
      cur = cur->next;
    }
  return FALSE;
}

/* Lookup k in ht.  Returns corresponding data in *d, and function
   result is TRUE if the k was in ht, false otherwise. */
bool hash_table_lookup(hash_table ht, hash_key k, hash_data *d)
{
  return hash_table_hash_search(ht, ht->cmp, k, d);
}

/* Add k:d to ht.  If k was already in ht, replace old entry by k:d.
   Rehash if necessary.  Returns TRUE if k was not already in ht. */
bool hash_table_insert(hash_table ht, hash_key k, hash_data d)
{
  bucket *cur;

  if (ht->used > 3 * ht->size / 4)
    rehash(ht);
  cur = find_bucket(ht, k);
  while (*cur)
    {
      if (ht->cmp(k, (*cur)->key))
	{
	  (*cur)->data = d;
	  return FALSE; /* Replace */
	}
      cur = &(*cur)->next;
    }
  *cur = ralloc(ht->r, struct bucket);
  (*cur)->key = k;
  (*cur)->data = d;
  (*cur)->next = NULL;
  ht->used++;
  return TRUE; /* New key */
}

/* Remove mapping for k in ht.  Returns TRUE if k was in ht. */
bool hash_table_remove(hash_table ht, hash_key k) 
{
  bucket *cur;
  bucket prev = NULL;

  cur = find_bucket(ht, k);
  while (*cur)
    {
      if (ht->cmp(k, (*cur)->key))
	{
	  if (prev)
	    prev->next = (*cur)->next;
	  else
	    *cur = (*cur)->next;
	  ht->used--;
	  return TRUE;
	}
      prev = *cur;
      cur = &(*cur)->next;
    }
  return FALSE;
}

/* Return a copy of ht */
hash_table hash_table_copy(region r, hash_table ht)
{
  unsigned long i;
  hash_table result;
  bucket cur, newbucket, *prev;

  result = make_hash_table(r, ht->size, ht->hash, ht->cmp);
  result->used = ht->used;

  for (i = 0; i < ht->size; i++)
    {
      prev = &result->table[i];
      scan_bucket(ht->table[i], cur)
	{
	  newbucket = ralloc(result->r, struct bucket);
	  newbucket->key = cur->key;
	  newbucket->data = cur->data;
	  newbucket->next = NULL;
	  assert(!*prev);
	  *prev = newbucket;
	  prev = &newbucket->next;
	}
    }
  return result;
}

/* Increase size of ht (double it) and reinsert all the elements */
static void rehash(hash_table ht)
{
  unsigned long old_table_size, i;
  bucket *old_table, cur;

#ifdef DEBUG
  printf("Rehash table size=%ld, used=%ld\n", ht->size, ht->used);
#endif

  old_table_size = ht->size;
  old_table = ht->table;

  ht->size = ht->size*2;
  ht->log2size = ht->log2size + 1;
  ht->used = 0;
  ht->table = rarrayalloc(ht->r, ht->size, bucket);

  for (i = 0; i < old_table_size; i++)
    scan_bucket(old_table[i], cur)
      hash_table_insert(ht, cur->key, cur->data);
}

/* Begin scanning ht */
void hash_table_scan(hash_table ht, hash_table_scanner *hts)
{
  hts->ht = ht;
  hts->i = 0;
  hts->cur = hts->ht->table[0];
}

/* Get next elt in table, storing the elt in *k and *d if k and d are
   non-NULL, respectively.  Returns TRUE if there is a next elt, FALSE
   otherwise. */
bool hash_table_next(hash_table_scanner *hts, hash_key *k, hash_data *d)
{
  while (hts->cur == NULL)
    {
      hts->i++;
      if (hts->i < hts->ht->size)
	hts->cur = hts->ht->table[hts->i];
      else
	break;
    }

  if (hts->i == hts->ht->size)
    {
      return FALSE;
    }
  else
    {
      if (k)
	*k = hts->cur->key;
      if (d)
	*d = hts->cur->data;
      hts->cur = hts->cur->next;
    }
  return TRUE;
}

/* Apply f to all elements of ht, in some arbitrary order */
void hash_table_apply(hash_table ht, hash_apply_fn f, void *arg)
{
  unsigned long i;
  bucket cur;

  for (i = 0; i < ht->size; i++)
    scan_bucket(ht->table[i], cur)
      f(cur->key, cur->data, arg);
}

/* Map f to all elements on ht, creating a new hash table in region r */
hash_table hash_table_map(region r, hash_table ht, hash_map_fn f, void *arg)
{
  unsigned long i;
  hash_table result;
  bucket cur, newbucket, *prev;

  result = make_hash_table(r, ht->size, ht->hash, ht->cmp);
  result->used = ht->used;
  
  for (i = 0; i < ht->size; i++)
    {
      prev = &result->table[i];
      scan_bucket(ht->table[i], cur)
	{
	  newbucket = ralloc(result->r, struct bucket);
	  newbucket->key = cur->key;
	  newbucket->data = f(cur->key, cur->data, arg);
	  newbucket->next = NULL;
	  assert(!*prev);
	  *prev = newbucket;
	  prev = &newbucket->next;
	}
    }
  return result;
}

static keycmp_fn cur_cmp = NULL;

int entry_cmp(const void *a, const void *b)
{
  struct sorted_entry *ae = (struct sorted_entry *) a;
  struct sorted_entry *be = (struct sorted_entry *) b;
  return cur_cmp(ae->k, be->k);
}

/* Begin scanning ht in sorted order according to f */
void hash_table_scan_sorted(hash_table ht, keycmp_fn f,
			    hash_table_scanner_sorted *htss)
{
  hash_table_scanner hts;
  unsigned long i;

  htss->r = newregion();
  htss->size = hash_table_size(ht);
  htss->entries = rarrayalloc(htss->r, htss->size, struct sorted_entry);
  htss->i = 0;

  hash_table_scan(ht, &hts);
  i = 0;
  while (hash_table_next(&hts, &htss->entries[i].k,
			 &htss->entries[i].d))
    i++;
  assert(i == htss->size);
  cur_cmp = f;
  qsort(htss->entries, htss->size, sizeof(struct sorted_entry), entry_cmp);
  cur_cmp = NULL;
}

/* Just like hash_table_next, but scans in sorted order */
bool hash_table_next_sorted(hash_table_scanner_sorted *htss, hash_key *k,
			    hash_data *d)
{
  if (htss->i < htss->size)
    {
      if (k)
	*k = htss->entries[htss->i].k;
      if (d)
	*d = htss->entries[htss->i].d;
      htss->i++;
      return TRUE;
    }
  else
    {
      deleteregion(htss->r);
      htss->r = NULL;
      return FALSE;
    }
}
