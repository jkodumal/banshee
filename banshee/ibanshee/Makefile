ibanshee: parser.tab.c lex.yy.c
	gcc -o ../bin/ibanshee.exe lex.yy.c parser.tab.c -I../engine -I../libcompat -L../engine -L../libcompat -lnsengine -lregions -lfl

parser.tab.c: parser.y
	bison -d parser.y

lex.yy.c: lexer.lex
	flex lexer.lex

clean: 
	rm -rf *.o parser.tab.h parser.tab.c lex.yy.c ../bin/ibanshee.exe