bison -d purecalc.y
flex -opurecalc.lex.c purecalc.l  
cc purecalc.lex.c purecalc.c purecalc.tab.c  -ll -o purecalc
