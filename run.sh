echo "Lexing..."
flex laxer.l
echo "Compiling Lex.yy.c"
gcc lex.yy.c -ll
echo "Parsing shader"
./a.out < shader.tsl
