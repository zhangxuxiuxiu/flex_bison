%{
#include "calc.tab.h"
%}
%%
"+" 	{ return ADD; }
"-" 	{ return SUB; }
"*" 	{ return MUL; }
"/" 	{ return DIV; }
"|" 	{ return ABS; }
"(" 	{ return LP; }
")" 	{ return RP; }
[0-9]+ 	{ yylval = atoi(yytext);  return NUMBER; }
\n 	{ return EOL; }
; 	{ return EOL; }
[ \t] 	{ /* ignore whitespace */ }
"//".*	/* ignore comments, somehow not work */
. 	{ printf("Mystery character %c\n", *yytext); }
%%
