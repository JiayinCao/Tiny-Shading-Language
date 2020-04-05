all:
	$(MAKE) grammer
	$(MAKE) lex
    
	echo    "Compiling generated C ..."
	gcc -m64 -c lex.yy.c grammer.tab.c
	ar rvs grammer.tab.a grammer.tab.o
	ar rvs lex.yy.a lex.yy.o
	g++ -m64 -std=c++11 main.cpp grammer.tab.a lex.yy.a -ll

	rm -rf tmp
	mkdir tmp
	mv lex.yy.* ./tmp/
	mv grammer.tab.* ./tmp/
	echo    "Executing binary ..."

	rm -rf bin
	mkdir bin
	mv a.out ./bin
	./bin/a.out < shader_first.tsl

grammer:
	echo    "Bison parsing ..."
	bison   -d grammer.y

lex:
	echo    "Laxer parsing ..."
	flex    lex.l

clean:
	rm -rf bin tmp