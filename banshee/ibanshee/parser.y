/* Parser for iBanshee */
%{
#include "nonspec.h"
#include "regions.h"
#include "hash.h"


static region ibanshee_region;
static hash_table constructor_env;
static hash_table named_env;
static hash_table var_env;

static void ibanshee_error_handler(gen_e e1, gen_e e2,banshee_error_kind bek) 
{
  fprintf(stderr,"Warning: ");
  switch(bek) {
  case bek_cons_mismatch:         // c(...) <= d(...)
    fprintf(stderr, "constructor mismatch ");
    break;
  case bek_occurs_check:	// occurs check failed (cyclic unification)
    fprintf(stderr, "occurs check failure ");
    break;
  default:
    fprintf(stderr, "unknown error ");
  }
  fprintf(stderr,"between expressions: ");
  expr_print(stderr,e1);
  fprintf(stderr,", ");
  expr_print(stderr,e2);
  fprintf(stderr,"\n");
}

static void print_tlb(gen_e e) 
{
  gen_e_list sol = setif_tlb(e);
	       
  if (gen_e_list_length(sol) == 0) {
    printf("{}");
  }
  else {
    gen_e next;
    gen_e_list_scanner scan;
    
    gen_e_list_scan(sol,&scan);
    
    gen_e_list_next(&scan,&next);
    printf("{");
    expr_print(stdout,next);
    
    while(gen_e_list_next(&scan,&next)) {
      printf(", ");
      expr_print(stdout,next);
    }
    printf("}");
  }
}

static void ibanshee_init(void) {
  region_init();
  nonspec_init();
  register_error_handler(ibanshee_error_handler);

  ibanshee_region = newregion();
  constructor_env = make_string_hash_table(ibanshee_region,32);
  named_env = make_string_hash_table(ibanshee_region,32);
  var_env = make_string_hash_table(ibanshee_region,32);
}

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
%left TOK_INTER "&&"
%left TOK_UNION "||"
%token TOK_LEQ "<="
%token TOK_DEQ "=="
%token TOK_LINE
%token TOK_ERROR
%token TOK_EOF

/* Keywords */
/* Commands (help,tlb,undo,time,trace,quit) are treated as
   identifiers,and interpreted after lexing. */

%token TOK_SETIF "setIF"
%token TOK_TERM "term" 
%token TOK_FLOW "flow"
%token TOK_ROW "row"
%token TOK_PROJ "proj"
%token TOK_PAT "pat"

%type <expr> expr
%type <exprs> expr_list
%type <esort> esort
%type <sort> basesort
%type <sort> sort 
%type <sig_elt_ptr> sig_elt
%type <pat> pattern
%type <rowmap> rowmap
%type <expr> row
%type <sig> signature

%union {
  int num;
  char *str;
  gen_e expr;
  gen_e_list exprs;
  e_sort esort;
  sort_kind sort;
  sig_elt* sig_elt_ptr;
  flowrow_map rowmap;
  sig_elt_list sig;
  pattern pat;
}
			      
 
%%

// TODO
line:      TOK_LINE
           { YYACCEPT; }
         | toplev TOK_LINE
           { YYACCEPT; }
         | TOK_EOF
           { exit(0); }
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
	       yyerror("Attempted to redefine existing variable");
	     }

	     switch($4) {
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
	       yyerror("Attempted to redefine existing constructor\n");
	     }
	     else {
	       hash_table_insert(named_env,$2,(hash_data)$4);
	     }
           }
         | TOK_DECL TOK_IDENT TOK_COLON basesort
           {
	     if (hash_table_lookup(constructor_env,$2,NULL)) {
               yyerror("Attempted to redefine existing constructor\n");
	     }
             else if (hash_table_lookup(named_env,$2,NULL)) {
	       yyerror("Attempted to redefine existing expression\n");
 	     }
	     else {
               constructor c = make_constructor($2,$4,NULL,0);
	       hash_table_insert(constructor_env,$2,(hash_data)c);
	     }
           }
         | TOK_DECL TOK_IDENT TOK_LPAREN signature TOK_RPAREN TOK_COLON basesort
           { 
	     if (hash_table_lookup(constructor_env,$2,NULL)) {
               yyerror("Attempted to redefine existing constructor\n");
	     }
             else if (hash_table_lookup(named_env,$2,NULL)) {
	       yyerror("Attempted to redefine existing expression\n");
 	     }
	     else {
               constructor c = make_constructor_from_list($2,$7,$4);
	       hash_table_insert(constructor_env,$2,c);
	     }
           }
;

signature: sig_elt
           { 
             sig_elt_list sig = new_sig_elt_list(ibanshee_region);
             sig_elt_list_cons($1,sig);
             $$ = sig;
           }
         | signature TOK_COMMA sig_elt
           {
             sig_elt_list_cons($3,$1);
             $$ = $1; 
           }
;

sig_elt:   TOK_POS sort
           {
	     sig_elt *eltptr = ralloc(ibanshee_region,sig_elt);
	     eltptr->variance = vnc_pos;
	     eltptr->sort = $2;
	     $$ = eltptr;
           }
         | TOK_NEG sort
            {
	     sig_elt *eltptr = ralloc(ibanshee_region,sig_elt);
	     eltptr->variance = vnc_neg;
	     eltptr->sort = $2;
	     $$ = eltptr;
           }
         | TOK_EQ sort
             {
	     sig_elt *eltptr = ralloc(ibanshee_region,sig_elt);
	     eltptr->variance = vnc_non;
	     eltptr->sort = $2;
	     $$ = eltptr;
           }
;
 
sort:       basesort
           { $$ = $1}
         | TOK_ROW TOK_LPAREN basesort TOK_RPAREN
           { $$ = flowrow_sort }
;

esort:      basesort
           {
             switch($1) {
 	       case setif_sort: $$ = e_setif_sort; break;
               case term_sort: $$ = e_term_sort; break;
	     default: yyerror("Bad base sort\n");
	     } 
           }
         | TOK_ROW TOK_LPAREN basesort TOK_RPAREN
           { 
             switch($3) {
	       case setif_sort: $$ = e_flowrow_setif_sort; break;
               case term_sort: $$ = e_flowrow_term_sort; break;
	     default: yyerror("Bad base sort\n");
	     }
           } 
;

basesort:  TOK_SETIF
           { $$ = setif_sort; }
         | TOK_TERM
           { $$ = term_sort; }
;

constraint: expr TOK_LEQ expr
            { call_sort_inclusion($1,$3); }
         |  expr TOK_DEQ expr
            { call_sort_unify($1,$3); }
;

expr:    TOK_VAR
           { 
	     gen_e v = NULL;
	     
	     if (hash_table_lookup(var_env,$1,(hash_data*)&v)) {
	       $$ = v;
	     }
	     else {
	       yyerror("Could not find variable\n",$1);
	     }
           }
         | TOK_IDENT /* constant or named expression */
           {
	     constructor c = NULL;
	     gen_e n = NULL;
             if (hash_table_lookup(constructor_env,$1,(hash_data *)&c)) {
	       $$ = constructor_expr(c,NULL,0);
	     }
	     else if (hash_table_lookup(named_env,$1,(hash_data*)&n)) {
	       $$ = n;
	     }
	     else {
	       yyerror("Could not find constant or named expression\n");
	     }
           }
         | TOK_IDENT TOK_LPAREN expr_list TOK_RPAREN /* a constructed term */
           {
	     constructor c = NULL;

	     if (hash_table_lookup(constructor_env,$1,(hash_data *)&c)) {
	       $$ = 
		 constructor_expr(c,
				  gen_e_list_array_from_list(ibanshee_region,$3),
				  gen_e_list_length($3));
	     }
	     else {
	       yyerror("Could not find constructor\n");
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
	       default: yyerror("Invalid sort for zero expression\n");
              } 

             }
             else if ($1 == 0) {
              switch($3) {
	       case e_setif_sort: $$ = setif_zero(); break;
	       case e_term_sort: $$ = term_zero(); break;
	       case e_flowrow_setif_sort: $$ = flowrow_zero(setif_sort); break;
	       case e_flowrow_term_sort: $$ = flowrow_zero(term_sort); break;
	       default: yyerror("Invalid sort for one expression\n");
	      } 
	     }
             else {
               yyerror("Invalid expression\n");
	     }
           }
         | TOK_WILD TOK_COLON esort	/* wildcard */
           { 
             switch($3) {
	     case e_flowrow_setif_sort: $$ = flowrow_wild(setif_sort); break;
	     case e_flowrow_term_sort: $$ = flowrow_wild(term_sort); break;
	     default: yyerror("Invalid sort for wildcard expression\n");
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
             flowrow_field field = flowrow_make_field($3,$5);
             flowrow_map_cons(field,$1);
             $$ = $1;
           }
;

pattern:   TOK_LPAREN TOK_IDENT TOK_COMMA TOK_INTEGER TOK_COMMA expr TOK_RPAREN
           { 
	     constructor c = NULL;
	     if (hash_table_lookup(constructor_env,$2,(hash_data *)&c)) {
	       $$ = (pattern){c,$4,$6}; 
	     }
	     else {
	       yyerror("Could not find constructor\n",$2);
	     }
	   }
;


// TODO
cmd:       TOK_CMD TOK_IDENT
           {
	     if (!strcmp($2,"quit")) {
	       exit(0);
	     }
	     if (!strcmp($2,"exit")) {
               exit(0);
	     }
	   }
        |  TOK_CMD TOK_IDENT TOK_INTEGER
           { }  
        |  TOK_CMD TOK_IDENT expr
           { 
	         if (!strcmp($2,"tlb")) {
	       		print_tlb($3);
	       	 }
	       	 else if (!strcmp($2,"ecr")) {
				expr_print(stdout,term_get_ecr($3));
	       	 }
	     
           }
;

%%
int main() {
  ibanshee_init();
  printf("iBanshee version 0.1");
  do {
    printf("\n>");
    fflush(stdout);
    yyparse();
  }
  while (1);
}

