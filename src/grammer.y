%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();
    void yyerror(char const* );

	#ifdef DEBUG_TOKENS
  		#define DEBUG_INFO( ... )  printf( __VA_ARGS__ );
	#else
  		#define DEBUG_INFO( ... )  ;
	#endif
%}

%token ID
%token INT_NUM
%token FLT_NUM
%token SHADER_FUNC_ID
%token TYPE_INT
%token TYPE_FLOAT
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
        DEBUG_INFO("Found a shader!\n");
    };

FUNCTION_DEF:
	ID "(" ")" COMPOUNDSTMT {
		DEBUG_INFO("Found a shader definition.\n");
	};

COMPOUNDSTMT:
	"{" "}" {
		DEBUG_INFO("Empty compound statement.\n");
	}
	|
	"{" STATEMENTS "}" {
		DEBUG_INFO("Statements.\n");
	};

STATEMENTS:
	STATEMENT {
		DEBUG_INFO("Found a statement.\n");
	}
	|
	STATEMENTS STATEMENT {
		DEBUG_INFO("Found multiple statements.\n" );
	};

STATEMENT:
	STATEMENT_EXPRESSION ";" {
		DEBUG_INFO("Place holder for now.\n");
	}
	|
	STATEMENT_DECLARATION ";" {
	};

STATEMENT_DECLARATION:
	PARAM{
	};

STATEMENT_EXPRESSION:
	EXPRESSION_CONST {
	    DEBUG_INFO("Useless expression?\n" );
	}
	|
	EXPRESSION_ASSIGN {
		DEBUG_INFO("Expression statement.\n");
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

PARAM:
	TYPE ID {
	}
	|
	TYPE ID "=" STATEMENT_EXPRESSION{
	};
	
TYPE:
	TYPE_INT {
	}
	|
	TYPE_FLOAT {
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