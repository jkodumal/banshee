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

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include "banshee.h"

typedef struct annotation_ *annotation;

/* function that returns true if a is the empty annotation */
typedef bool (*empty_annotation_fn) (annotation);

/* function that returns true if a and a' are equal annotations  */
typedef bool (*eq_annotation_fn) (annotation, annotation);

/* function that computes a new annotation given the current
   annotations and the lower and upper bounds:
   
   C ^ (e <=_a1 <= 'x <=_a2 e') => C ^ (e <=_transition(e,a1,a2,e') e')
 */
typedef annotation (*transition_fn) (gen_e, annotation, annotation, gen_e);

/* function that determines whether annother annotated constraint
   subsumes this one. If true, there is no need to add this
   constraint:

   C ^ (e <=_a e') => C if subsumed(e,a,e')
                   => C ^ (e <=_a e') otherwise

   Note that the constraint must be atomic, so one or both of e and e'
   must be a variable
 */
typedef bool (*subsumption_fn) (gen_e, annotation, gen_e);

#endif /* ANNOTATIONS_H */
