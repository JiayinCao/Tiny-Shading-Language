%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();
    void yyerror(char const* );
%}

%token INDENTIFIER L_CBRACKET R_CBRACKET L_RBRACKET R_RBRACKET COLON INT_NUM FLT_NUM OP_ADD OP_MINUS OP_MULT OP_DIV SHADER_FUNC_ID EOL COMMA EQUAL TO_BE_IGNORED

%start input

%%
input:
    SHADER_FUNC_ID  INDENTIFIER L_RBRACKET R_RBRACKET L_CBRACKET R_CBRACKET
%%

void yyerror(char const * p){
    printf( "Error: %s\n" , p );
}