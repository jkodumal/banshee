%{
  /* IBanshee lexer */
  #include <stdio.h>
  #include <string.h>
  #include "nonspec.h"
  #include "regions.h"
  #include "parser.tab.h"
  #include "hash.h"
  
%}

%option always-interactive
%option noyywrap

WHITESPACE ['\t' ' ']
LINE ['\n''\r']
UPPERCASE [A-Z]
LOWERCASE [a-z]
DIGIT [0-9]
DIGITS ({DIGIT}+)
IDENT [A-Za-z][A-Za-z0-9_]*
TICK [\']
%%
<<EOF>>     return TOK_EOF;
{LINE}      return TOK_LINE;
"setIF"     return TOK_SETIF;
"term"      return TOK_TERM;
"flow"      return TOK_FLOW;
"row"       return TOK_ROW;
"proj"      return TOK_PROJ;
"pat"       return TOK_PAT;
"("         return TOK_LPAREN;
")"         return TOK_RPAREN;
"<"         return TOK_LANGLE;
">"         return TOK_RANGLE;
"|"         return TOK_REST;
"+"         return TOK_POS;
"-"         return TOK_NEG;
"="         return TOK_EQ;
"#"         return TOK_DECL;
","         return TOK_COMMA;
"_"         return TOK_WILD;
":"         return TOK_COLON;
"!"         return TOK_CMD;
"^"         return TOK_CARET;
"&&"        return TOK_INTER;
"||"        return TOK_UNION;
"<="        return TOK_LEQ;
"=="        return TOK_DEQ;
{TICK}{IDENT} {yylval.str = strdup(yytext); return TOK_VAR; }
{IDENT} { yylval.str = strdup(yytext); return TOK_IDENT; }
{DIGITS} { yylval.num = atoi(yytext); return TOK_INTEGER; }
[ \t]+				/* ignore whitespace */
.
%%

yyerror( char *msg ) { fprintf( stderr, "%s\n", msg ); }

