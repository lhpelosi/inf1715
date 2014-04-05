TEST = ./mini0 tests

mini0: lex.yy.c y.tab.c
	gcc -o mini0 lex.yy.c y.tab.c

lex.yy.c: lex.l
	lex lex.l

y.tab.c: yacc.y
	yacc -d yacc.y

clean:
	rm -f mini0 *.o lex.yy.c y.tab.c y.tab.h y.output

test: mini0
	$(TEST)/testok_simple.txt
	$(TEST)/testok_multi.txt
	$(TEST)/testok_decl.txt
	$(TEST)/testok_fun.txt
	$(TEST)/testok_cmd.txt
	$(TEST)/testok_exp.txt
	! $(TEST)/testerror_simple.txt
	! $(TEST)/testerror_multi.txt
	! $(TEST)/testerror_blockorder.txt
	! $(TEST)/testerror_ifelse.txt
	! $(TEST)/testerror_fechamento.txt
	! $(TEST)/testerror_cmdexp.txt

