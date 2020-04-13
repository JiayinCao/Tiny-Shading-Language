%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();
    void yyerror(char const* );
%}

%token ID
%token INT_NUM
%token FLT_NUM
%token SHADER_FUNC_ID
%token EOL              ";"
%token L_CBRACKET       "{"
%token R_CBRACKET       "}"
%token L_RBRACKET       "("
%token R_RBRACKET       ")"
%token L_SBRACKET       "["
%token R_SBRACKET       "]"
%token COLON            ":"
%token OP_ADD           "+"
%token OP_MINUS         "-"
%token OP_MULT          "*"
%token OP_DIV           "/"
%token COMMA            ","
%token EQUAL            "="
%token DOT				"."

/* the start token */
%start PROGRAM

%%
PROGRAM:
    SHADER_FUNC_ID FUNCTION_DEF {
        printf("Found a shader!\n");
    };

FUNCTION_DEF:
	ID "(" ")" COMPOUNDSTMT {
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
	STATEMENT_EXPRESSION ";" {
		printf("Place holder for now.\n");
	};

STATEMENT_EXPRESSION:
	EXPRESSION_CONST {
	    printf("Useless expression?\n" );
	}
	|
	EXPRESSION_ASSIGN {
		printf("Expression statement.\n");
	}
	|
	EXPRESSION_OP {
	};

EXPRESSION_ASSIGN:
	EXPRESSION_REF "=" STATEMENT_EXPRESSION {
	};

EXPRESSION_CONST:
	INT_NUM {
	}
	|
	FLT_NUM {
	};

EXPRESSION_REF:
	EXPRESSION_REF "." IDENTIFIER {
	}
	|
	IDENTIFIER {
	};

EXPRESSION_OP:
	EXPRESSION_ADD {
	}
	|
	EXPRESSION_MINUS {
	}
	|
	EXPRESSION_MULT {
	}
	|
	EXPRESSION_DIV{
	};

EXPRESSION_ADD:
	STATEMENT_EXPRESSION "+" STATEMENT_EXPRESSION {
	};

EXPRESSION_MINUS:
	STATEMENT_EXPRESSION "-" STATEMENT_EXPRESSION {
	};

EXPRESSION_MULT:
	STATEMENT_EXPRESSION "*" STATEMENT_EXPRESSION {
	};

EXPRESSION_DIV:
	STATEMENT_EXPRESSION "/" STATEMENT_EXPRESSION {
	};

IDENTIFIER:
	ID {
	}
	|
	ID "[" INT_NUM "]"{
	};
%%

void yyerror(char const * p){
    printf( "Error: %s\n" , p );
}