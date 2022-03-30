/* calculator with AST */
/*%define "api.pure" full */
%pure-parser
%parse-param { struct pcdata *pp }
%{
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
%}
%union {
 struct ast *a;
 double d;
 struct symbol *s; /* which symbol */
 struct symlist *sl;
 int fn; /* which function */
 void* fp;
}

%{
# include "grammar.lex.h"
# include "ast.h"
#define YYLEX_PARAM pp->scaninfo
%}

/* declare tokens */
%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
%token <fp> FPTR
%token EOL
%token IF THEN ELSE WHILE DO LET
%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS
%type <a> exp stmt list explist
%type <sl> symlist
%start calc
%%

calc: /* nothing */ EOL { pp->ast = NULL; YYACCEPT; }
 | stmt  { pp->ast = $1; YYACCEPT; }
 | LET NAME '(' symlist ')' '=' list EOL {
 dodef(pp, $2, $4, $7);
 printf("%d: Defined %s\n", yyget_lineno(pp->scaninfo),$2->name);
 pp->ast = NULL; YYACCEPT; }
 ;

stmt: IF exp THEN list { $$ = newflow(pp, 'I', $2, $4, NULL); }
 | IF exp THEN list ELSE list { $$ = newflow(pp, 'I', $2, $4, $6); }
 | WHILE exp DO list { $$ = newflow(pp, 'W', $2, $4, NULL); }
 | exp 
;

list: /* nothing */ { $$ = NULL; }
 | stmt ';' list { if ($3 == NULL)
 	$$ = $1;
 	else
 	$$ = newast(pp, 'L', $1, $3);
 }
 ;

exp: exp CMP exp { $$ = newcmp(pp, $2, $1, $3); }
 | exp '+' exp { $$ = newast(pp, '+', $1,$3); }
 | exp '-' exp { $$ = newast(pp, '-', $1,$3);}
 | exp '*' exp { $$ = newast(pp, '*', $1,$3); }
 | exp '/' exp { $$ = newast(pp, '/', $1,$3); }
 | '|' exp { $$ = newast(pp, '|', $2, NULL); }
 | '(' exp ')' { $$ = $2; }
 | '-' exp %prec UMINUS { $$ = newast(pp, 'M', $2, NULL); }
 | NUMBER { $$ = newnum(pp, $1); }
 | FPTR { $$ = newfptr(pp, $1); }
 | FUNC '(' explist ')' { $$ = newfunc(pp, $1, $3); }
 | NAME { $$ = newref(pp, $1); }
 | NAME '=' exp { $$ = newasgn(pp, $1, $3); }
 | NAME '(' explist ')' { $$ = newcall(pp, $1, $3); }
;

explist: exp
 | exp ',' explist { $$ = newast(pp, 'L', $1, $3); }
;

symlist: NAME { $$ = newsymlist(pp, $1, NULL); }
 | NAME ',' symlist { $$ = newsymlist(pp, $1, $3); }
;

%%

int init_grammar(struct pcdata* p){
	if(yylex_init_extra(p, &(p->scaninfo))) {
		perror("init alloc failed");
		return 1;
	}
	if(!(p->symtab = calloc(NHASH, sizeof(struct symbol)))) {
		perror("sym alloc failed");
		return 2;
	}
	return 0;
}

struct ast* build_ast(struct pcdata* p,  const char* stmt){
	YY_BUFFER_STATE bp = yy_scan_string(stmt,  p->scaninfo);
	yy_switch_to_buffer(bp, p->scaninfo);
	int r = yyparse(p);
	yy_delete_buffer(bp,  p->scaninfo);
	if(r) return NULL;
	struct ast* a = p->ast;
	p->ast=NULL;
	return a;
}

void free_grammar(struct pcdata* p){
	for(int i=0; i<NHASH; ++i){
		free_symbol(p, p->symtab+i);
	}
	free(p->symtab);
}

void yyerror(struct pcdata *pp, char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "%d: error: ", yyget_lineno(pp->scaninfo));
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
}
