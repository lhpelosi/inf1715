

lexico: lex.yy.c
	gcc -o lexico lex.yy.c

lex.yy.c: lexico.l
	flex lexico.l

clean:
	rm -f lexico *.o lex.yy.c

test: lexico
	@echo "Teste sem erros:"
	./lexico < teste_ok.txt
	@echo "Teste com erros:"
	./lexico < teste_erro.txt
