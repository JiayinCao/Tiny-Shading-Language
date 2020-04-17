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

	int g_verbose = 0;	// somehow bool is not working here.

	extern int yylineno;
%}

%locations

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
// A programm has a bunch of global statement.
PROGRAM:
	// empty shader
	{}
	|
	GLOBAL_STATEMENTS {
	};

// One or multiple of blobal statements
GLOBAL_STATEMENTS:
	GLOBAL_STATEMENT{
	}
	|
	GLOBAL_STATEMENT GLOBAL_STATEMENTS {
	};

// Global statement could be one of the followings
//  - Global variable decleration.
//  - Global function definition.
//  - Global data structure definition.
//  - Shader function definition.
GLOBAL_STATEMENT:
	STATEMENT_VARIABLES_DECLARATIONS{
	}
	|
    SHADER_DEF {
        DEBUG_INFO("Found a shader!\n");
    }
	|
	FUNCTION_DEF {
	};

// Shader is the only unit that can be exposed in the group.
SHADER_DEF:
	SHADER_FUNC_ID FUNCTION_DEF {
	};

// Standard function definition
FUNCTION_DEF:
	ID "(" ")" FUNCTION_BODY {
		DEBUG_INFO("Found a shader definition.\n");
	};

FUNCTION_BODY:
	"{" "}" {
		DEBUG_INFO("Empty compound statement.\n");
	}
	|
	"{" STATEMENTS "}" {
		DEBUG_INFO("Statements.\n");
	};

STATEMENTS:
	STATEMENT_PROXY {
		DEBUG_INFO("Found a statement.\n");
	}
	|
	STATEMENTS STATEMENT_PROXY {
		DEBUG_INFO("Found multiple statements.\n" );
	};

STATEMENT_PROXY:
	"{" "}"{
	}
	|
	"{" STATEMENTS "}"{
	}
	|
	STATEMENT {
	};

STATEMENT:
	STATEMENT_EXPRESSION {
		DEBUG_INFO("Place holder for now.\n");
	}
	|
	STATEMENT_VARIABLES_DECLARATIONS {
	};

STATEMENT_VARIABLES_DECLARATIONS:
	PARAMS_DECLARATION {
	}
	|
	PARAMS_DECLARATION STATEMENT_VARIABLES_DECLARATIONS {
	};

PARAMS_DECLARATION:
	TYPE VARIABLE_DECLARATIONS ";" {
	};

VARIABLE_DECLARATIONS:
	VARIABLE_DECLARATION {
	}
	|
	VARIABLE_DECLARATION "," VARIABLE_DECLARATIONS {
	};

VARIABLE_DECLARATION:
	ID {
	}
	|
	ID "=" EXPRESSION {
	};

STATEMENT_EXPRESSION:
	EXPRESSION ";" {
	};

EXPRESSION:
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
	EXPRESSION_REF "=" EXPRESSION {
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
	EXPRESSION "+" EXPRESSION {
	};

EXPRESSION_MINUS:
	EXPRESSION "-" EXPRESSION {
	};

EXPRESSION_MULT:
	EXPRESSION "*" EXPRESSION {
	};

EXPRESSION_DIV:
	EXPRESSION "/" EXPRESSION {
	};

TYPE:
	TYPE_INT {
	}
	|
	TYPE_FLOAT {
	}
	|
	ID {
		// custom data structure
	};
	
IDENTIFIER:
	ID {
	}
	|
	ID "[" INT_NUM "]"{
	};
%%

void yyerror(char const * str){
	if(!g_verbose)
		return;

	printf( "line(%d), error: %s\n", yylineno, str);
}

void makeVerbose(int verbose){
	 g_verbose = verbose;
}