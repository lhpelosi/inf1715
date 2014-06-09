TEST = ./mini0 tests

mini0: lex.yy.c y.tab.c
	gcc -o mini0 lex.yy.c y.tab.c ast.c sym.c

lex.yy.c: lex.l
	lex lex.l

y.tab.c: yacc.y
	yacc -d yacc.y

clean:
	rm -f mini0 *.o lex.yy.c y.tab.c y.tab.h y.output

test: mini0
	$(TEST)/ok_decl.txt
	$(TEST)/ok_cmd.txt
	$(TEST)/ok_exp.txt
	! $(TEST)/error_decl.txt
	! $(TEST)/error_cmd.txt
	! $(TEST)/error_exp.txt

