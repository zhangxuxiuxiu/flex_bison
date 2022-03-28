bison -d calc.y
flex calc.l
cc  -o calc calc.tab.c lex.yy.c -ll  
