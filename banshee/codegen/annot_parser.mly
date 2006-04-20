/*
 * Copyright (c) 2000-2006
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

%{
open Fsm
let reverse x = List.rev x
%}

%token EOF
%token <string> IDENT
%token LBRACKET
%token RBRACKET
%token ARROW
%token WILDCARD
%token OR
%token SEMICOLON
%token COLON
%token STATE START ACCEPT
%start state_machine
%type <Fsm.state> state
%type <Fsm.fsm> state_machine
%type <Fsm.symbol> symbol
%type <Fsm.transition> transition
%type <Fsm.state_opt> options

%%

state_machine : state_list { reverse($1) }
;

state_list : state { [$1] }
		   | state_list state { $2 :: $1  }
;

state : options STATE IDENT COLON transition_list SEMICOLON { {name = $3; delta = reverse($5); opts = $1}   }
       | options STATE IDENT SEMICOLON { {name = $3; delta = []; opts = $1} }
;

options : START { Start }
       | ACCEPT { Accept }
       | START ACCEPT { StartAndAccept }
       | ACCEPT START { StartAndAccept }
       | { NoOpt }
;

transition_list : transition { [$1] }
				| transition_list transition { $2 :: $1 }
;

transition : OR symbol ARROW IDENT { ($2,$4) }
;

symbol : IDENT { Letter($1) }
	   | IDENT LBRACKET IDENT RBRACKET { Param($1,$3) }
	   | WILDCARD { Wildcard}
;
