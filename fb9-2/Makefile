purecalc: purecalc.y purecalc.l ast.c ast.h
	bison -d purecalc.y
	flex -opurecalc.lex.c purecalc.l  
	cc purecalc.lex.c ast.c purecalc.tab.c  -ll -o purecalc

clean: 
	-rm purecalc.tab.{c,h} purecalc.lex.{c,h} purecalc
