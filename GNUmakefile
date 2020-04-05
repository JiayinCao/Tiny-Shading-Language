MAKEFLAGS += --silent

all:
	rm -rf tmp
	mkdir tmp

	$(MAKE) grammer
	$(MAKE) lex
    
	echo    "Compiling generated C ..."
	cd tmp;ls;gcc -m64 -c compiled_lex.c compiled_grammer.c;\
	ar rvs compiled_grammer.a compiled_grammer.o;\
	ar rvs compiled_lex.a compiled_lex.o;

	rm -rf bin;mkdir bin;cd bin;\
	g++ -m64 -std=c++11 ../src/main.cpp ../tmp/compiled_grammer.a ../tmp/compiled_lex.a

	echo    "Executing binary ..."
	./bin/a.out < example/shader_first.tsl

grammer:
	echo    "Bison parsing ..."
	bison   -d src/grammer.y -o tmp/compiled_grammer.c

lex:
	echo    "Laxer parsing ..."
	flex    src/lex.l

clean:
	rm -rf bin tmp