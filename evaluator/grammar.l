/* recognize tokens for the calculator */
/* pure scanner and parser version */
/* $Header: /usr/home/johnl/flnb/RCS/ch09.tr,v 1.4 2009/05/19 18:28:27 johnl Exp $ */
%option noyywrap nodefault yylineno reentrant bison-bridge
%option header-file="grammar.lex.h"
%{
#include "grammar.tab.h"
#include "ast.h"
%}
/* float exponent */
EXP ([Ee][-+]?[0-9]+)
%%
%{
 struct pcdata *pp = yyextra;
%}
 /* single character ops */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")" { return yytext[0]; }
 /* comparison ops */
">" { yylval->fn = 1; return CMP; }
"<" { yylval->fn = 2; return CMP; }
"<>" { yylval->fn = 3; return CMP; }
"==" { yylval->fn = 4; return CMP; }
">=" { yylval->fn = 5; return CMP; }
"<=" { yylval->fn = 6; return CMP; }
 /* keywords */
"if" { return IF; }
"then" { return THEN; }
"else" { return ELSE; }
"while" { return WHILE; }
"do" { return DO; }
"let" { return LET;}
 /* built-in functions */
"sqrt" { yylval->fn = B_sqrt; return FUNC; }
"exp" { yylval->fn = B_exp; return FUNC; }
"log" { yylval->fn = B_log; return FUNC; }
"print" { yylval->fn = B_print; return FUNC; }
 /*"like" { yylval->fp = &eval_like; return FPTR; }*/
 /*"comment" { yylval->fp = &eval_comment; return FPTR; }*/
 /*"follow" { yylval->fp = &eval_follow; return FPTR; }*/
 /* names */
[a-zA-Z][a-zA-Z0-9]* { yylval->s = lookup(pp, yytext); return NAME; }
[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}? { yylval->d = atof(yytext); return NUMBER; }
"//".* 
[ \t] /* ignore whitespace */ 
\\n { return EOL; } /*printf("c> ");  ignore line continuation */
"\n" { return EOL; }
. { yyerror(pp, "Mystery character %c\n", *yytext); }
 /*<<EOF>> { printf("meet eof\n"); exit(0); }*/
%%
