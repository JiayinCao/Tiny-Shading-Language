all:
	$(MAKE) lex
    
	echo    "Compiling generated C ..."
	gcc lex.yy.c -ll

	echo    "Executing binary ..."
	./a.out < shader.tsl

lex:
	echo    "Laxer parsing ..."
	flex    lex.l

clean:
	rm a.out lex.yy.c