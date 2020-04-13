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
%token EOL              ";"
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

/* the start token */
%start PROGRAM

%%
PROGRAM:
    SHADER_FUNC_ID FUNCTION_DEF {
        printf("Found a shader!\n");
    };

FUNCTION_DEF:
	INDENTIFIER "(" ")" COMPOUNDSTMT {
		printf("Found a shader definition.\n");
	};

COMPOUNDSTMT:
	"{" "}" {
		printf("Empty compound statement.\n");
	}
	|
	"{" STATEMENTS "}" {
		printf("Statements.\n");
	};

STATEMENTS:
	STATEMENT {
		printf("Found a statement.\n");
	}
	|
	STATEMENTS STATEMENT {
		printf("Found multiple statements.\n" );
	};

STATEMENT:
	INDENTIFIER ";" {
		printf("Place holder for now.\n");
	};

%%

void yyerror(char const * p){
    printf( "Error: %s\n" , p );
}