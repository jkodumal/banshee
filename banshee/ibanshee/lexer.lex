%{
  /* IBanshee lexer */
  #include <stdio.h>
%}
WHITESPACE ['\t' '\n' '\r' ' ']
UPPERCASE [A-Z]
LOWERCASE [a-z]
DIGIT [0-9]
DIGITS ({DIGIT}+)
IDENT [A-Za-z0-9_]*
TICK [\']
%%
{WHITESPACE}
"setIF"
"setST"
"term" 
"flow"
"row"
"help"
"tlb"
"undo"
"time"
"trace"
"proj"
"pat"
"("
")"
"<"
">"
"|"
"+"
"-"
"="
"#"
","
"_"
"&&"
"||"
"<="
"=="
{UPPERCASE}{IDENT}
{LOWERCASE}{IDENT}
{TICK}{IDENT}

%%

main()
{
  yylex();
}


