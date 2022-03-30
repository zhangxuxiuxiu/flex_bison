eval: grammar.y grammar.l ast.c ast.h main.c
	bison -d grammar.y
	flex -ogrammar.lex.c grammar.l  
	cc grammar.lex.c ast.c grammar.tab.c main.c -ll -DYYERROR_VERBOSE=1 -o eval

clean: 
	-rm grammar.tab.{c,h} grammar.lex.{c,h} eval
