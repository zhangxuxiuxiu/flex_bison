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
}

%{
# include "purecalc.lex.h"
# include "ast.h"
#define YYLEX_PARAM pp->scaninfo
%}

/* declare tokens */
%token <d> NUMBER
%token <s> NAME
%token <fn> FUNC
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
 printf("%d: Defined %s\n", yyget_lineno(pp->scaninfo),
 $2->name);
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

int main(int argc, char* argv[]){
	struct pcdata p = { NULL, 0, NULL };
	/* set up scanner */
	if(yylex_init_extra(&p, &p.scaninfo)) {
		perror("init alloc failed");
		return 1;
	}
	/* allocate and zero out the symbol table */
	if(!(p.symtab = calloc(NHASH, sizeof(struct symbol)))) {
		perror("sym alloc failed");
		return 1;
	}

	YY_BUFFER_STATE bp;
	FILE* f=NULL;
	if( argc==1 || (argc==2 && (0==strcmp(argv[1],"-f"))) || (argc==3 && (0==strcmp(argv[1],"-f"))  && (0==strcmp(argv[2],"-"))) ){	
		bp = yy_create_buffer(stdin, YY_BUF_SIZE,  p.scaninfo); 
	} else if(argc != 3 || 0!=strcmp(argv[1],"-f") && 0!=strcmp(argv[1],"-s")) {
		fprintf(stderr, "Usage: %s (-s str) (-f (file|-))", argv[0]);
		return -1;
	} else if( argv[1][1] == 'f' ){
		f = fopen( argv[2], "r");
		bp = yy_create_buffer(f, YY_BUF_SIZE,  p.scaninfo); 
	} else {
		bp = yy_scan_string(argv[2],  p.scaninfo);
	}
	yy_switch_to_buffer(bp, p.scaninfo);

//	for(;;) {
		printf("> "); 
		yyparse(&p);
		if(p.ast) {
			printf("= %4.4g\n", eval(&p, p.ast));
			treefree(&p, p.ast);
			p.ast = 0;
		} else {
			printf("no ast\n");
		}
//	}

	yy_delete_buffer(bp,  p.scaninfo);
	if( f != NULL) { fclose(f); }

	return 0;
}

void yyerror(struct pcdata *pp, char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "%d: error: ", yyget_lineno(pp->scaninfo));
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
}