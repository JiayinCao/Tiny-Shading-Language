echo "Lexing..."
flex laxer.lex
echo "Compiling Lex.yy.c"
g++ lex.yy.c -ll
echo "Parsing shader"
./a.out < shader.tsl
