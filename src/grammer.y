%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();
    void yyerror(char const* );
%}

%token INDENTIFIER
%token INT_NUM
%token FLT_NUM
%token SHADER_FUNC_ID
%token EOL
%token TO_BE_IGNORED
%token L_CBRACKET       "{"
%token R_CBRACKET       "}"
%token L_RBRACKET       "("
%token R_RBRACKET       ")"
%token COLON            ":"
%token OP_ADD           "+"
%token OP_MINUS         "-"
%token OP_MULT          "*"
%token OP_DIV           "/"
%token COMMA            ","
%token EQUAL            "="

%start input

%%
input:
    SHADER_FUNC_ID  INDENTIFIER "(" ")" "{" "}"
%%

void yyerror(char const * p){
    printf( "Error: %s\n" , p );
}