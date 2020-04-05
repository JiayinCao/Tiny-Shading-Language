MAKEFLAGS += --silent

all:
	$(MAKE) grammer
	$(MAKE) lex
    
	echo    "Compiling generated C ..."
	gcc -m64 -c compiled_lex.c compiled_grammer.c
	ar rvs compiled_grammer.a compiled_grammer.o
	ar rvs compiled_lex.a compiled_lex.o
	g++ -m64 -std=c++11 main.cpp compiled_grammer.a compiled_lex.a

	rm -rf tmp
	mkdir tmp
	mv compiled_lex.* ./tmp/
	mv compiled_grammer.* ./tmp/
	echo    "Executing binary ..."

	rm -rf bin
	mkdir bin
	mv a.out ./bin
	./bin/a.out < shader_first.tsl

grammer:
	echo    "Bison parsing ..."
	bison   -d grammer.y -o compiled_grammer.c

lex:
	echo    "Laxer parsing ..."
	flex    lex.l

clean:
	rm -rf bin tmp