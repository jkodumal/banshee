/* Parser for iBanshee */
%{
#include "nonspec.h"
#include "regions.h"

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

				
/* TODO - static constructor lookup_constructor(char *name)  */

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
%type <row> gen_e


%union {
  int num;
  char *str;
}
			      
 
%%

program:   /* empty */ 
           { }
         | program line
           { }
;

line:      TOK_LINE
           { }
         | toplev TOK_LINE
           { }
;

toplev:    decl
           { }
         | constraint
           { }
         | cmd
           { }
;

decl:      TOK_DECL TOK_VAR TOK_COLON sort
           { }
         | TOK_DECL TOK_IDENT TOK_EQ expr
           { }
         | TOK_DECL cons_decl TOK_COLON basesort
           { }
;

cons_decl: TOK_IDENT 
           { }
         | TOK_IDENT TOK_LPAREN signature TOK_RPAREN
           { }
;

signature: sig_elt
           { }
         | signature TOK_COMMA sig_elt
           { }
;

sig_elt:   TOK_POS sort
           { $$ = (sig_elt){vnc_pos,$2}; }
         | TOK_NEG sort
           { $$ = (sig_elt){vnc_neg,$2}; }
         | TOK_EQ sort
           { $$ = (sig_elt){vnc_non,$2}; }
;
 
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
         | TOK_FLOW
           { $$ = flow_sort; }
;

constraint: expr TOK_DEQ expr
            { }
         |  expr TOK_LEQ expr
            { }
;

expr:    TOK_VAR
           { 

           }
         | TOK_IDENT /* constant or named expression */
           {
             
           }
         | TOK_IDENT LPAREN expr_list TOK_RPAREN /* a constructed term */
           {

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
         | TOK_INTEGER COLON esort /* only 0,1 though */
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
         | TOK_WILD COLON esort	/* wildcard */
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
           { $$ = (pattern){lookup_constructor($2),$4,$6}; }
;

cmd:       TOK_CMD TOK_IDENT
           { }
        |  TOK_CMD TOK_IDENT TOK_INTEGER
           { }  
        |  TOK_CMD TOK_IDENT TOK_EXPR 
           { }
;
