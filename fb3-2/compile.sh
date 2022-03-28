bison -d calc.y
flex calc.l 
cc lex.yy.c calc.c calc.tab.c  -ll -o calc
