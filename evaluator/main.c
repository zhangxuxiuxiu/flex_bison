#include "grammar.tab.h"
#include "grammar.lex.h"
#include "ast.h"

#include <stdio.h>
#include <time.h>

#define CLK_TCKCLOCKS_PER_SEC 1000

double raw_fn(struct user_score* u){
	return u->like * u->follow - u-> comment;
}

int main(int argc, char* argv[]){
//	if(argc==1){
//		fprintf(stderr, "Usage: %s \"like+follow/comment\" \"2*like+3*comment+4*follow-0.5\" \n", argv[0]);
//		return -1;
//	}

	// 1) init grammar and define keywords
	struct pcdata p = { NULL, 0, NULL };
	init_grammar(&p);
	addsym(&p, "like", &eval_like);
	addsym(&p, "comment", &eval_comment);
	addsym(&p, "follow", &eval_follow);

	// 2) prepare user data
	struct user_score u1={2,3,4};
	struct user_score u2={3,4,5};
	struct user_score u3={6,7,8};
	printf("u1: like->%f, comment->%f, follow->%f\n", u1.like, u1.comment, u1.follow);
	printf("u1: like->%f, comment->%f, follow->%f\n", u2.like, u2.comment, u2.follow);
	printf("u1: like->%f, comment->%f, follow->%f\n", u3.like, u3.comment, u3.follow);

	// 3) build ast for each statement and eval ast on each user datum
	struct ast* a = build_ast(&p, "like*follow-comment");
	if(!a){
		fprintf(stderr, "failed in building ast\n");
		return -1;
	}

	clock_t start_ts = clock();
	double r =0.f;
	for(int j=0; j<100000; ++j){
	//	printf( "%s = %f\n", argv[i+1], eval(&p, asts[i], &u1));
		r+=eval(&p, a, &u1);
		r+=eval(&p, a, &u2);
		r+=eval(&p, a, &u3);
	}
	printf("ast cost:%f, r=%f\n", (double)(clock()-start_ts)/CLK_TCKCLOCKS_PER_SEC, r);

	start_ts = clock();
	r = 0.f;
	for(int j=0; j<100000; ++j){
		r+=raw_fn(&u1);
		r+=raw_fn(&u2);
		r+=raw_fn(&u3);
	}
	printf("raw cost:%f, r=%f\n", (double)(clock()-start_ts)/CLK_TCKCLOCKS_PER_SEC, r);

	// 4) destruction
	free_grammar(&p);
	treefree(&p, a);



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
