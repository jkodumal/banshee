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

#include <assert.h>
#include "list.h"
#include "utils.h"

struct list_node_
{  
  void *data;
  struct list_node_ *sameregion next;
};

#define scan_node(b,var) for (var = b; var; var = var->next)

struct list 
{
  region sameregion r;
  int length;
  list_node sameregion head;
  list_node sameregion tail;
};

struct list *new_list(region r)
{
  struct list *result;

  assert(r);
  
  result = ralloc(r,struct list);
  result->r = r;
  result->length = 0;
  result->head = NULL;
  result->tail = NULL;

  return result;
}

int list_length(struct list *l)
{
  return l->length;
}

struct list *list_cons(void *data, struct list *l)
{
  list_node newnode = ralloc(l->r, struct list_node_);
  newnode->next = l->head;
  newnode->data = data;

  if (l->head == NULL) {
    assert(list_length(l) == 0);
    l->tail = newnode;
  }
  
  l->head = newnode;
  l->length++;

  return l;
}

struct list *list_append_tail(void *data, struct list *l)
{

  if (l->tail == NULL) {
    assert(l->head == NULL);
    assert(list_length(l) == 0);
    list_cons(data, l);
  }
  else {
    list_node newnode = ralloc(l->r, struct list_node_);
    assert(l->tail->next == NULL);
    newnode->next = NULL;
    newnode->data = data;
    l->tail->next = newnode;
    l->tail = newnode;
    l->length++;
  }

  return l;
}

struct list *list_reverse(struct list *l)
{

  if (list_empty(l))
    return l;

  else
    {
      list_node temp,reversed = NULL; 

      l->tail = l->head;

      while (l->head)
	{
	  temp = l->head->next;
	  
	  l->head->next = reversed;
	  
	  reversed = l->head;

	  l->head = temp;
	}

      l->head = reversed;
      return l;
    }

}

bool list_empty(struct list *l)
{
  assert( (l->head == NULL) == (list_length(l) == 0) ); 
  return (l->head == NULL);
}

static list_node tail(struct list *l)
{
  return l->tail;
}

static list_node fetch_tail(list_node n)
{
  if (n == NULL)
    return NULL;
  else
    {
      list_node temp = NULL,
	tail = NULL;
      
      scan_node(n,temp)
	tail = temp;
      
      assert(tail && tail->next == NULL);
      
      return tail;
    }
}

struct list *list_append(struct list *a, struct list *b)
{
  list_node tl;

  assert( a && b );
  assert( a != b);
  assert( ptr_eq(a->r,b->r) );

  tl = tail(a);

  
  if (! tl)
    {
      a->head = b->head;
      a->length = b->length;
      a->tail = tail(b);
    } 
  
  else
    {
      tl->next = b->head;
      a->length += b->length;
      a->tail = tail(b);
    }
  return a;
}

struct list *list_app(struct list *l,app_fn app)
{
  list_node n = NULL;

  
  assert(l);
  
  scan_node(l->head,n)
    {
      app(n->data);
    }
  return l;
}

void *list_find(struct list *l,eq_fn eq)
{
  list_node n = NULL;
  assert(l);
  
  scan_node(l->head,n)
    {
      if (eq(n->data))
	return n->data;
    }
  
  return NULL;
}

struct list *list_tail(struct list *l)
{
  l->length--;
  l->head = l->head->next;
  return l;
}

void *list_head(struct list *l)
{
  assert(l->head);
  return l->head->data;
}

void *list_last(struct list *l) 
{
  assert(l->tail);
  return l->tail->data;
}


/* Drop the first list element s{atisfying eq */
void list_drop(struct list *l, eq_fn eq)
{
  list_node prev, n;
  assert(l);
  
  if (l->head == NULL) return;
  else if (eq(l->head->data)) {
    l->head = l->head->next;
    l->length--;
    return;
  }
  else {
    prev = l->head;
    
    scan_node(l->head->next,n) 
      {
	if (eq(n->data)) {
	  if (n == l->tail) {
	    l->tail = prev;
	    assert(n->next == NULL);
	  }
	  prev->next = n->next;
	  l->length--;
	  return;
	}
	else prev = n;
      }
    
  }
  
  
}

struct list *list_filter(region r,struct list *l,eq_fn eq)
{
  struct list *result;
  list_node n = NULL;
  assert(l);

  result = new_list(r);
  
  scan_node(l->head,n)
    {
      if (eq(n->data)) {
	list_cons(n->data,result);
      }
    }
  
  assert( (list_length(result) == 0) || result->tail);
  
  return result;
}

struct list *list_keep(struct list *l, eq_fn eq)
{
  list_node prev, n;
  assert(l);

  while (l->head && !eq(l->head->data))
    {
      l->head = l->head->next;
      l->length--;
    }

  prev = l->head;
  scan_node(l->head->next,n)
    {
      if (!eq(n->data)) {
	  prev->next = n->next;
	  l->length--;
      }
      else prev = n;
    }

  l->tail = prev;

  assert( (list_length(l) == 0) || l->tail);

  return l;
}

struct list *list_filter2(struct list *l,eq_fn eq)
{
  return list_filter(l->r,l,eq);
}

struct list *list_copy(region r, struct list *l)
{

  struct list *result;
  list_node n = NULL;
#ifndef NDEBUG
  int count = 0;
#endif  
  assert(l);

  result = new_list(r);
  
  scan_node(l->head,n)
    {
      list_cons(n->data,result);
      assert(++count <= l->length);
    }
 
  return list_reverse(result);
}
/* A Linked-List Memory Sort
   by Philip J. Erdelsky
   pje@acm.org
   http://www.alumni.caltech.edu/~pje/
*/

#include <stdio.h>

void *sort_linked_list(void *p, unsigned index,
  int (*compare)(const void *,const void *), unsigned long *pcount)
{
  unsigned base;
  unsigned long block_size;

  struct record
  {
    struct record *next[1];
    /* other members not directly accessed by this function */
  };

  struct tape
  {
    struct record *first, *last;
    unsigned long count;
  } tape[4];

  /* Distribute the records alternately to tape[0] and tape[1]. */

  tape[0].count = tape[1].count = 0L;
  tape[0].first = NULL;
  base = 0;
  while (p != NULL)
  {
    struct record *next = ((struct record *)p)->next[index];
    ((struct record *)p)->next[index] = tape[base].first;
    tape[base].first = ((struct record *)p);
    tape[base].count++;
    p = next;
    base ^= 1;
  }

  /* If the list is empty or contains only a single record, then */
  /* tape[1].count == 0L and this part is vacuous.               */

  for (base = 0, block_size = 1L; tape[base+1].count != 0L;
    base ^= 2, block_size <<= 1)
  {
    int dest;
    struct tape *tape0, *tape1;
    tape0 = tape + base;
    tape1 = tape + base + 1;
    dest = base ^ 2;
    tape[dest].count = tape[dest+1].count = 0;
    for (; tape0->count != 0; dest ^= 1)
    {
      unsigned long n0, n1;
      struct tape *output_tape = tape + dest;
      n0 = n1 = block_size;
      while (1)
      {
        struct record *chosen_record;
        struct tape *chosen_tape;
        if (n0 == 0 || tape0->count == 0)
        {
          if (n1 == 0 || tape1->count == 0)
            break;
          chosen_tape = tape1;
          n1--;
        }
        else if (n1 == 0 || tape1->count == 0)
        {
          chosen_tape = tape0;
          n0--;
        }
        else if ((*compare)(tape0->first, tape1->first) > 0)
        {
          chosen_tape = tape1;
          n1--;
        }
        else
        {
          chosen_tape = tape0;
          n0--;
        }
        chosen_tape->count--;
        chosen_record = chosen_tape->first;
        chosen_tape->first = chosen_record->next[index];
        if (output_tape->count == 0)
          output_tape->first = chosen_record;
        else
          output_tape->last->next[index] = chosen_record;
        output_tape->last = chosen_record;
        output_tape->count++;
      }
    }
  }

  if (tape[base].count > 1L)
    tape[base].last->next[index] = NULL;
  if (pcount != NULL)
    *pcount = tape[base].count;
  return tape[base].first;
}



struct list *list_sort(struct list *l, comparator_fn cmp)
{
  int compare(const void *node1, const void *node2)
    {
      return cmp(((struct list_node_ *)node1)->data,
		 ((struct list_node_ *)node2)->data);
    }
  
  long pcount;
  l->head = sort_linked_list(l->head,1,compare,&pcount);
  l->tail = fetch_tail(l->head);
  assert(pcount == l->length);
  return l;
}

struct list *list_merge(struct list *a,struct list *b, comparator_fn cmp)
{
  return list_sort( list_append(a,b),cmp);
}

void list_scan(struct list *a,struct list_scanner *scan)
{ 
  scan->l = a;
  scan->cur = a->head;
}

bool list_next(struct list_scanner *scan, void **data)
{
  if (!scan->cur)
    return FALSE;
  else
    {
      if (data)
	*data = scan->cur->data;
      scan->cur = scan->cur->next;
      return TRUE;
    }
}

void list_clear(struct list *l)
{ 
  l->head = NULL;
  l->tail = NULL;
  l->length = 0;
}

bool list_member(struct list *l,void *data)
{
  list_node n = NULL;
  scan_node(l->head,n)
    {
      if (n->data == data)
	return TRUE;
    }
  return FALSE;
}


struct list *list_from_array(region r, void **data, int length)
{
  struct list *result = new_list(r);
  int i;
  
  for (i = length - 1; i >= 0; i--)
    {
      list_cons(data[i],result);
    }
  return result;
}

void **array_from_list(region r, struct list *l) 
{
  int i = 0;
  void **result = rarrayalloc(r,list_length(l),void *);
  list_node n = NULL;

  scan_node(l->head,n)
    {
      result[i] = n->data;
      i++;
    }
  return result;
}







