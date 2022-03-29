#include "purecalc.tab.h"
#include "purecalc.lex.h"
#include "ast.h"

#include <stdio.h>

int main(int argc, char* argv[]){
	struct pcdata p = { NULL, 0, NULL };
	YY_BUFFER_STATE bp;

	if(build_ast(&p, &bp, argv[1])){
		fprintf(stderr, "failed in building ast\n");
		return -1;
	}
	printf( "%s = %f\n", argv[1], eval(&p, p.ast));
	free_ast(&p, bp);

/*
	if(yylex_init_extra(&p, &p.scaninfo)) {
		perror("init alloc failed");
		return 1;
	}
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

	for(;;) {
		printf("> "); 
		yyparse(&p);
		if(p.ast) {
			printf("= %4.4g\n", eval(&p, p.ast));
			treefree(&p, p.ast);
			p.ast = 0;
		} else {
			printf("no ast\n");
		}
	}

	yy_delete_buffer(bp,  p.scaninfo);
	if( f != NULL) { fclose(f); }
*/
	return 0;
}
