/*
    This file is a part of Tiny-Shading-Language or TSL, an open-source cross
    platform programming shading language.

    Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.

    TSL is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

%{
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();
    void yyerror(char const* );

	int g_verbose = 0;	// somehow bool is not working here.

	extern int yylineno;

	#include "ast.h"

	/* global variables which can be used in other .c .h */
	struct Program *g_program = c_nullptr;
%}

/* definitions of tokens and types passed by FLEX */
%union {
	/* pointers for the AST struct nodes */
    struct Program 			*Program_Ptr;
}

%locations

%token ID
%token INT_NUM
%token FLT_NUM
%token INC_OP			"++"
%token DEC_OP			"--"
%token SHADER_FUNC_ID
%token TYPE_INT			"int"
%token TYPE_FLOAT		"float"
%token TYPE_VOID		"void"
%token EOL              ";"
%token L_CBRACKET       "{"
%token R_CBRACKET       "}"
%token L_RBRACKET       "("
%token R_RBRACKET       ")"
%token L_SBRACKET       "["
%token R_SBRACKET       "]"
%token OP_ADD           "+"
%token OP_MINUS         "-"
%token OP_MULT          "*"
%token OP_DIV           "/"
%token OP_MOD			"%"
%token OP_AND			"&"
%token OP_OR			"|"
%token OP_XOR			"^"
%token OP_LOGIC_AND     "&&"
%token OP_LOGIC_OR		"||"
%token OP_EQ			"=="
%token OP_NE			"!="
%token OP_GE			">="
%token OP_G				">"
%token OP_LE			"<="
%token OP_L				"<"
%token OP_SHL			"<<"
%token OP_SHR			">>"
%token OP_ADD_ASSIGN    "+="
%token OP_MINUS_ASSIGN  "-="
%token OP_MULT_ASSIGN   "*="
%token OP_DIV_ASSIGN    "/="
%token OP_MOD_ASSIGN    "%="
%token OP_ASSIGN        "="
%token OP_AND_ASSIGN	"&="
%token OP_OR_ASSIGN		"|="
%token OP_XOR_ASSIGN	"^="
%token OP_SHL_ASSIGN	"<<="
%token OP_SHR_ASSIGN	">>="
%token OP_NOT			"!"
%token OP_COMP			"~"
%token DOT				"."
%token COMMA            ","
%token COLON            ":"
%token METADATA_START   "<<<"
%token METADATA_END     ">>>"
%token RETURN		    "return"
%token QUESTION_MARK	"?"
%token IF				"if"
%token ELSE				"else"
%token FOR				"for"
%token WHILE			"while"
%token DO				"do"

%type <Program_Ptr> PROGRAM

%nonassoc IF_THEN
%nonassoc ELSE

%left ","
%right "=" "+=" "-=" "*=" "/=" "%=" "<<=" ">>=" "&=" "|=" "^="
%right "?" ":"
%left "||"
%left "&&"
%left "|"
%left "^"
%left "&"
%left "==" "!="
%left ">" ">=" "<" "<=" 
%left "<<" ">>"
%left "+" "-"
%left "*" "/" "%"
%right UMINUS_PREC "!" "~"
%left "++" "--"
%left "(" ")"
%left "[" "]"
%left "<<<" ">>>"

/* the start token */
%start PROGRAM

%%
// A programm has a bunch of global statement.
PROGRAM:
	// empty shader
	{
        $$ = c_nullptr;
	}
	|
	GLOBAL_STATEMENTS {
		struct Program *p = (struct Program*)malloc(sizeof(struct Program));	// create a new node and allocate the space
        p->tmp = 123;
        g_program = p;
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
	"{" STATEMENTS "}" {
	};

STATEMENTS:
	STATEMENT STATEMENTS {}
	|
	/* empty */ {};

STATEMENT:
	STATEMENT_SCOPED{
	}
	|
	STATEMENT_RETURN{
	}
	|
	STATEMENT_VARIABLES_DECLARATIONS {
	}
	|
	STATEMENT_CONDITIONAL {
	}
	|
	STATEMENT_LOOP {
	}
	|
	STATEMENT_COMPOUND_EXPRESSION {
	};

STATEMENT_SCOPED:
	"{" 
		{ /* push a new scope here */ }
	STATEMENTS "}"
		{ /* pop the scope from here */ }
	;

STATEMENT_RETURN:
	"return" STATEMENT_EXPRESSION_OPT ";"
	{
	};

STATEMENT_EXPRESSION_OPT:
	COMPOUND_EXPRESSION {
	}
	|
	/* empty */ {
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

STATEMENT_CONDITIONAL:
	"if" "(" COMPOUND_EXPRESSION ")" STATEMENT %prec IF_THEN {
	}
	|
	"if" "(" COMPOUND_EXPRESSION ")" STATEMENT "else" STATEMENT {
	};
	
STATEMENT_LOOP:
	"while" "(" COMPOUND_EXPRESSION ")" STATEMENT{
	}
	|
	"do" STATEMENT "while" "(" COMPOUND_EXPRESSION ")" ";"{
	}
	|
	"for" "(" FOR_INIT_STATEMENT COMPOUND_EXPRESSION_OPT ";" COMPOUND_EXPRESSION_OPT ")" STATEMENT {
	};
	
FOR_INIT_STATEMENT:
	";"{
	}
	|
	COMPOUND_EXPRESSION ";"{
	}
	|
	STATEMENT_VARIABLES_DECLARATIONS {
	};
	
STATEMENT_COMPOUND_EXPRESSION:
	COMPOUND_EXPRESSION ";" {
	}

COMPOUND_EXPRESSION_OPT:
	/* empty */ {}
	|
	COMPOUND_EXPRESSION {
	};

COMPOUND_EXPRESSION:
	EXPRESSION {
	}
	| 
	EXPRESSION "," COMPOUND_EXPRESSION {
	};

// Exrpession always carries a value so that it can be used as input for anything needs a value,
// like if condition, function parameter, etc.
EXPRESSION:
	EXPRESSION_UNARY {
	}
	|
	EXPRESSION_BINARY {
	}
	|
	EXPRESSION_TERNARY {
	}
	|
	EXPRESSION_ASSIGN {
	}
	|
	EXPRESSION_FUNCTION_CALL {
	}
	|
	EXPRESSION_CONST {
	}
	|
	EXPRESSION_SCOPED {
	}
	|
	EXPRESSION_TYPECAST {
	}
	|
	EXPRESSION_VARIABLE {
	};

EXPRESSION_UNARY:
	OP_UNARY EXPRESSION %prec UMINUS_PREC {
	};
	
OP_UNARY:
	"-" {
	}
	|
	"+" {
	}
	|
	"!" {
	}
	|
	"~" {
	};

EXPRESSION_BINARY:
	EXPRESSION "&&" EXPRESSION {
	}
	|
	EXPRESSION "||" EXPRESSION {
	}
	|
	EXPRESSION "&" EXPRESSION {
	}
	|
	EXPRESSION "|" EXPRESSION {
	}
	|
	EXPRESSION "^" EXPRESSION {
	}
	|
	EXPRESSION "==" EXPRESSION {
	}
	|
	EXPRESSION "!=" EXPRESSION {
	}
	|
	EXPRESSION ">" EXPRESSION {
	}
	|
	EXPRESSION "<" EXPRESSION {
	}
	|
	EXPRESSION ">=" EXPRESSION {
	}
	|
	EXPRESSION "<=" EXPRESSION {
	}
	|
	EXPRESSION "<<" EXPRESSION {
	}
	|
	EXPRESSION ">>" EXPRESSION {
	}
	|
	EXPRESSION "+" EXPRESSION {
	}
	|
	EXPRESSION "-" EXPRESSION {
	}
	|
	EXPRESSION "*" EXPRESSION {
	}
	|
	EXPRESSION "/" EXPRESSION{
	}
	|
	EXPRESSION "%" EXPRESSION{
	};

// Ternary operation support
EXPRESSION_TERNARY:
	EXPRESSION "?" EXPRESSION ":" EXPRESSION {
	};

// Assign an expression to a reference
EXPRESSION_ASSIGN:
	VARIABLE_LVALUE "=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "+=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "-=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "*=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "/=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "%=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "&=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "|=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "^=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE "<<=" EXPRESSION {
	}
	|
	VARIABLE_LVALUE ">>=" EXPRESSION {
	};

// Function call, this is only non-shader function. TSL doesn't allow calling shader function.
EXPRESSION_FUNCTION_CALL:
	ID "(" FUNCTION_ARGUMENTS ")" {
	};

// None-shader function arguments
FUNCTION_ARGUMENTS:
	{}
	|
	EXPRESSION{
	}
	|
	FUNCTION_ARGUMENTS "," EXPRESSION {
	};


// Const literal
EXPRESSION_CONST:
	INT_NUM {
	}
	|
	FLT_NUM {
	};

// Scopped expression
EXPRESSION_SCOPED:
	"(" COMPOUND_EXPRESSION ")" {
	};

// This is for type casting
EXPRESSION_TYPECAST:
	"(" TYPE ")" EXPRESSION {
	};

EXPRESSION_VARIABLE:
	VARIABLE_LVALUE{
	}
	|
	VARIABLE_LVALUE REC_OR_DEC {
	}
	|
	REC_OR_DEC VARIABLE_LVALUE {
	};

REC_OR_DEC:
	"++" {
	}
	|
	"--" {
	};

// No up to two dimensional array supported for now.
VARIABLE_LVALUE:
	ID_OR_FIELD {
	}
	|
	ID_OR_FIELD "[" EXPRESSION "]" {
	};

ID_OR_FIELD:
	ID{
	}
	|
	VARIABLE_LVALUE "." ID {
	};

TYPE:
	"int" {
	}
	|
	"float" {
	}
	|
	"void" {
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
