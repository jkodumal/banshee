/* Parser for iBanshee */
%{
#include "nonspec.h"
#include "regions.h"

DECLARE_LIST(signature, sig_elt);
DEFINE_LIST(signature, sig_elt); 

struct pattern {
  constructor c;
  int index;
  gen_e e;
};

// Fully expanded sort kinds. Translate to sort_kinds as needed
// These are needed when we have row(base) and need to know the base
// which occurs when we have 0,1, or _ row expressions
enum e_sort {
  e_setif_sort,
  e_term_sort,
  e_flowrow_setif_sort;
  e_flowrow_term_sort;
};

static region ibanshee_region;
static hash_table constructor_env;
static hash_table named_env;
static hash_table var_env;

%}			

/* Things with multiple spellings */
%token <num> TOK_INTEGER
%token <str> TOK_IDENT
%token <str> TOK_VAR

/* Punctuators */
%token TOK_LPAREN "("
%token TOK_RPAREN ")"
%token TOK_LANGLE "<"
%token TOK_RANGLE ">"
%token TOK_REST "|"
%token TOK_POS "+"
%token TOK_NEG "-"
%token TOK_EQ "="
%token TOK_DECL "#"
%token TOK_COMMA ","
%token TOK_WILD "_"
%token TOK_COLON ":"
%token TOK_CMD "!"
%token TOK_INTER "&&"
%token TOK_UNION "||"
%token TOK_LEQ "<="
%token TOK_DEQ "=="
%token TOK_LINE

/* Keywords */
/* Commands (help,tlb,undo,time,trace,quit) are treated as
   identifiers,and interpreted after lexing. */

%token TOK_SETIF "setIF"
%token TOK_TERM "term" 
%token TOK_FLOW "flow"
%token TOK_ROW "row"
%token TOK_PROJ "proj"
%token TOK_PAT "pat"

%type <gen_e> expr
%type <gen_e_list> expr_list
%type <e_sort> esort
%type <sort_kind> basesort
%type <sort> sort 
%type <sig_elt> sig_elt
%type <pattern> pattern
%type <flowrow_map> rowmap
%type <gen_e> row
%type <signature> signature


%union {
  int num;
  char *str;
}
			      
 
%%

// TODO
program:   /* empty */ 
           { }
         | program line
           { }
;

// TODO
line:      TOK_LINE
           { }
         | toplev TOK_LINE
           { }
;

// TODO
toplev:    decl
           { }
         | constraint
           { }
         | cmd
           { }
;

decl:      TOK_DECL TOK_VAR TOK_COLON sort
           { 
	     gen_e fresh_var;

	     if (hash_table_lookup(var_env,$2,NULL)) {
	       fail("A variable named %s already exists.\n",$2);
	     }

	     switch(sort) {
	     case setif_sort:
	       fresh_var = setif_fresh($2);
	       break;
	     case term_sort:
	       fresh_var = term_fresh($2);
	       break;
	     case flowrow_sort:
	       fresh_var = flowrow_fresh($2);
	       break;
	     }	     
	     hash_table_insert(var_env,$2,fresh_var);
	   }
         | TOK_DECL TOK_IDENT TOK_EQ expr
           {
	     if (hash_table_lookup(constructor_env,$2,NULL)) {
	       fail("A constructor named %s already exists.\n",$2);
	     }
	     else {
	       hash_table_insert(named_env,$2,(hash_data)$4);
	     }
           }
         | TOK_IDENT TOK_COLON basesort
           {
	     if (hash_table_lookup(constructor_env,$1,NULL)) {
               fail("Constructor %s already defined.\n",$1);
	     }
             else if (hash_table_lookup(named_env,$1,NULL)) {
	       fail("An expression named %s already exists.\n",$1);
 	     }
	     else {
               constructor c = make_constructor($1,$3,NULL,0);
	       hash_table_insert(constructor_env,$1,(hash_data)c);
	     }
           }
         | TOK_IDENT TOK_LPAREN signature TOK_RPAREN TOK_COLON basesort
           { 
	     if (hash_table_lookup(constructor_env,$1,NULL)) {
               fail("Constructor %s already defined.\n",$1);
	     }
             else if (hash_table_lookup(named_env,$1,NULL)) {
	       fail("An expression named %s already exists.\n",$1);
 	     }
	     else {
               constructor c = 
		 make_constructor($1,$6,
				  signature_array_from_list(ibanshee_region,
							    $3),
				  signature_length($3));
	       hash_table_insert(constructor_env,$1,c);
	     }
           }
;

signature: sig_elt
           { 
             signature sig = new_signature(ibanshee_region);
             signature_cons($1,sig);
             $$ = sig;
           }
         | signature TOK_COMMA sig_elt
           {
             signature_cons($3,$1);
             $$ = $1; 
           }
;

sig_elt:   TOK_POS sort
           { $$ = (sig_elt){vnc_pos,$2}; }
         | TOK_NEG sort
           { $$ = (sig_elt){vnc_neg,$2}; }
         | TOK_EQ sort
           { $$ = (sig_elt){vnc_non,$2}; }
;
 
sort:       basesort
           { $$ = $1}
         | TOK_ROW TOK_LPAREN basesort TOK_RPAREN
           { $$ = flowrow_sort }

esort:      basesort
           {
             switch($1) {
 	       case setif_sort: $$ = e_setif_sort; break;
               case term_sort: $$ = e_term_sort; break;
               case flow_sort: $$ = e_flow_sort; break;
	       default: fail("Bad base sort\n");
	     } 
           }
         | TOK_ROW TOK_LPAREN basesort TOK_RPAREN
           { 
             switch($3) {
	       case setif_sort: $$ = e_flowrow_setif_sort; break;
               case term_sort: $$ = e_flowrow_term_sort; break;
               case flow_sort: $$ = e_flowrow_flow_sort; break;
	       default: fail("Bad base sort\n");
	     }
           } 
;

basesort:  TOK_SETIF
           { $$ = setif_sort; }
         | TOK_TERM
           { $$ = term_sort; }
;

constraint: expr TOK_DEQ expr
            { call_sort_inclusion($1,$3); }
         |  expr TOK_LEQ expr
            { call_sort_unify($1,$3); }
;

expr:    TOK_VAR
           { 
	     gen_e v = NULL;
	     
	     if (hash_table_lookup(var_env,$1,&v)) {
	       $$ = v;
	     }
	     else {
	       fail("Could not find variable named %s.\n",$1);
	     }
           }
         | TOK_IDENT /* constant or named expression */
           {
	     constructor c = NULL;
	     gen_e n = NULL;
             if (hash_table_lookup(constructor_env,$1,&c)) {
	       $$ = constructor_expr(c,NULL,0);
	     }
	     else if (hash_table_lookup(named_env,$1,&n)) {
	       $$ = n;
	     }
	     else {
	       fail("Could not find constant or expression named %s.\n",$1);
	     }
           }
         | TOK_IDENT TOK_LPAREN expr_list TOK_RPAREN /* a constructed term */
           {
	     constructor c = NULL;

	     if (hash_table_lookup(constructor_env,$1,&c)) {
	       $$ = 
		 constructor_expr(c,
				  gen_e_array_from_list(ibanshee_region,$3),
				  gen_e_list_length($3));
	     }
	     else {
	       fail("Could not find constructor named %s.\n",$1);
	     }
           }
         | expr TOK_UNION expr	/* the union of e1 and e2 */
           {
             gen_e_list exprs = new_gen_e_list(ibanshee_region); 
             gen_e_list_cons($1, exprs);
             gen_e_list_cons($3, exprs); 
             $$ = setif_union(exprs);
           }
         | expr TOK_INTER expr	/* the intersection of e1 and e2 */
           { 
             gen_e_list exprs = new_gen_e_list(ibanshee_region); 
             gen_e_list_cons($1, exprs);
             gen_e_list_cons($3, exprs); 
             $$ = setif_inter(exprs);
           }
         | TOK_LANGLE row TOK_RANGLE /* a row */
           {
             $$ = $2;
           }
         | TOK_INTEGER TOK_COLON esort /* only 0,1 though */
           { 
             if ($1 == 1) {
              switch($3) {
	       case e_setif_sort: $$ = setif_one(); break;
	       case e_term_sort: $$ = term_one(); break;
	       case e_flowrow_setif_sort: $$ = flowrow_one(setif_sort); break;
	       case e_flowrow_term_sort: $$ = flowrow_one(term_sort); break;
	       default: fail("Invalid sort for zero expression\n");
              } 

             }
             else if ($1 == 0) {
              switch($3) {
	       case e_setif_sort: $$ = setif_zero(); break;
	       case e_term_sort: $$ = term_zero(); break;
	       case e_flowrow_setif_sort: $$ = flowrow_zero(setif_sort); break;
	       case e_flowrow_term_sort: $$ = flowrow_zero(term_sort); break;
	       default: fail("Invalid sort for one expression\n");
	      } 
	     }
             else {
               fail("Invalid expression %d\n",$1);
	     }
           }
         | TOK_WILD TOK_COLON esort	/* wildcard */
           { 
             switch($3) {
	       case e_setif_sort: $$ = setif_wild(); break;
  	       case e_flowrow_setif_sort: $$ = flowrow_wild(setif_sort); break;
               case e_flowrow_term_sort: $$ = flowrow_wild(term_sort); break;
               default: fail("Invalid sort for wildcard expression\n");
             }  
           }
         | TOK_PAT pattern	/* a projection pattern */
           { 
             $$ = setif_proj_pat($2.c,$2.i,$2.e); 
           }
         | TOK_PROJ pattern	/* a projection */
           { 
             $$ = setif_proj($2.c,$2.i,$2.e); 
           }	  
         | TOK_LPAREN expr TOK_RPAREN /* parenthesized expression */
           { $$ = $2; }
;

expr_list: expr 
           {
             gen_e_list exprs = new_gen_e_list(ibanshee_region);
             gen_e_list_cons($1,exprs);
             $$ = exprs; 
           }
         | expr_list TOK_COMMA expr
           {
             gen_e_list_cons($3,$1);
             $$ = $1;
           }
;
 
row:       rowmap
           { 
             $$ = flowrow_make_row($1, flowrow_fresh("rest"));
           }
         | rowmap TOK_REST expr
           { 
             $$ = flowrow_make_row($1, $3);
           }
;

rowmap:    TOK_IDENT TOK_EQ expr 
           {
             flowrow_map map = new_flowrow_map(ibanshee_region);
             flowrow_field field = flowrow_make_field($1,$3);
             flowrow_map_cons(field,map);
             $$ = map;
           }
         | rowmap TOK_COMMA TOK_IDENT TOK_EQ expr
           {
             flowrow_field field = flowrow_make_field($1,$3);
             flowrow_map_cons(field,$1);
             $$ = $1;
           }
;

pattern:   TOK_LPAREN TOK_IDENT TOK_COMMA TOK_INTEGER TOK_COMMA expr TOK_RPAREN
           { 
	     constructor c = NULL;
	     if (hash_table_lookup(constructor_env,$2,&c)) {
	       $$ = (pattern){c,$4,$6}; 
	     }
	     else {
	       fail("Could not find constructor named %s.\n",$2);
	     }
	   }
;


// TODO
cmd:       TOK_CMD TOK_IDENT
           {
	     if (!strcmp($2,"quit")) {
	       exit(0);
	     }
	   }
        |  TOK_CMD TOK_IDENT TOK_INTEGER
           { }  
        |  TOK_CMD TOK_IDENT expr
           { }
;
