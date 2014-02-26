

lex: lex.yy.c
	gcc -o lex lex.yy.c

lex.yy.c: lex.l
	flex lex.l

clean:
	rm -f lex *.o lex.yy.c

test: lex
	./lex < testLex.txt
