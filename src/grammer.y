%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();
    void yyerror(char const* );

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
%token TYPE_VOID		"void"
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
%token METADATA_START   "<<<"
%token METADATA_END     ">>>"

%left '(' ')' 

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
    }
	|
	FUNCTION_DEF {
	};

// Shader is the only unit that can be exposed in the group.
SHADER_DEF:
	SHADER_FUNC_ID ID "(" ")" FUNCTION_BODY {
	}
	|
	SHADER_FUNC_ID ID "(" SHADER_FUNCTION_ARGUMENT_DECLS ")" FUNCTION_BODY {
	};

SHADER_FUNCTION_ARGUMENT_DECLS:
	SHADER_FUNCTION_ARGUMENT_DECL{
	}
	|
	SHADER_FUNCTION_ARGUMENT_DECL "," SHADER_FUNCTION_ARGUMENT_DECLS {
	};

SHADER_FUNCTION_ARGUMENT_DECL:
	FUNCTION_ARGUMENT_DECL ARGUMENT_METADATA {
	};

ARGUMENT_METADATA:
	// no meta data
	{}
	|
	"<<<" ">>>"{
	};

// Standard function definition
FUNCTION_DEF:
	TYPE ID "(" FUNCTION_ARGUMENT_DECLS ")" FUNCTION_BODY {
	};

FUNCTION_ARGUMENT_DECLS:
	{}
	|
	FUNCTION_ARGUMENT_DECL{
	}
	|
	FUNCTION_ARGUMENT_DECL "," FUNCTION_ARGUMENT_DECLS{
	};

FUNCTION_ARGUMENT_DECL:
	TYPE ID {
	}
	|
	TYPE ID "=" EXPRESSION {
	};

FUNCTION_BODY:
	"{" "}" {
	}
	|
	"{" STATEMENTS "}" {
	};

STATEMENTS:
	STATEMENT_PROXY {
	}
	|
	STATEMENTS STATEMENT_PROXY {
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
	}
	|
	STATEMENT_VARIABLES_DECLARATIONS {
	};

STATEMENT_VARIABLES_DECLARATIONS:
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
	FUNCTION_CALL{
	}
	|
	EXPRESSION_CONST {
	}
	|
	EXPRESSION_ASSIGN {
	}
	|
	EXPRESSION_OP {
	}
	|
	IDENTIFIER{
	};

FUNCTION_CALL:
	ID "(" FUNCTION_ARGUMENTS ")" {
	};

FUNCTION_ARGUMENTS:
	{}
	|
	EXPRESSION{
	}
	|
	FUNCTION_ARGUMENTS "," EXPRESSION {
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
	TYPE_VOID{
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