all: ibanshee.exe

ibanshee.exe: parser.tab.c lex.yy.c
	gcc -o ibanshee.exe lex.yy.c parser.tab.c -I../engine -I../libcompat -L../engine -L../libcompat -lnsengine -lregions -lfl -g -pg
	cp ibanshee.exe ../bin

parser.tab.c: parser.y
	bison -d parser.y

lex.yy.c: lexer.lex
	flex lexer.lex

clean: 
	rm -rf *.o parser.tab.h parser.tab.c lex.yy.c gmon.out ../bin/ibanshee.exe ibanshee.exe