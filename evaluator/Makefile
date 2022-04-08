eval: grammar.y grammar.l ast.c ast.h main.cpp
	bison -d grammar.y
	flex -ogrammar.lex.c grammar.l  
	g++ -std=c++11 grammar.lex.c ast.c grammar.tab.c main.cpp  -DYYERROR_VERBOSE=1 -o eval -Wno-deprecated-register

clean: 
	-rm grammar.tab.{c,h} grammar.lex.{c,h} eval
