%{
  /* IBanshee lexer */
  #include <stdio.h>
%}
WHITESPACE ['\t' '\n' '\r' ' ']
UPPERCASE [A-Z]
LOWERCASE [a-z]
DIGIT [0-9]
DIGITS ({DIGIT}+)
IDENT [A-Za-z][A-Za-z0-9_]*
TICK [\']
%%
{WHITESPACE}
"setIF"     return TOK_SETIF;
"setST"     return TOK_SETST;
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
"&&"        return TOK_INTER;
"||"        return TOK_UNION;
"<="        return TOK_LEQ;
"=="        return TOK_DEQ;
{TICK}{IDENT} return TOK_VAR;
{IDENT} return TOK_IDENT;
{DIGITS} return TOK_INTEGER;

%%

main()
{
  yylex();
}


