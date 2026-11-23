all:
	bison -d parser.y
	flex lexer.l
	gcc -o parser parser.tab.c lex.yy.c main.c

run: all
	./parser test.c
	python3 out.py

clean:
	rm -f parser parser.tab.c parser.tab.h lex.yy.c out.py ast.html
