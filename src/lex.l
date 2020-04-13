%{
  #include "compiled_grammer.h"

  // #define DEBUG_TOKENS

  #ifdef DEBUG_TOKENS
  	#define DEBUG_INFO( ... )  printf( __VA_ARGS__ );
  #else
  	#define DEBUG_INFO( ... )  ;
  #endif
%}

%option outfile="generated_src/compiled_lex.c"
%option noyywrap
%option never-interactive

integer_numbers          [+-]?[0-9]+
float_numbers            [+-]?([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)
one_line_comment         (\/\/)(.*\n)
identifier               [a-zA-Z_]+[a-zA-Z_0-9]*
whitespace               [ \t\r\n\f]

%%
"."                       { DEBUG_INFO(".", yytext);                      return DOT; }
"shader"                  { DEBUG_INFO("shader_func  ->\t%s\n", yytext);  return SHADER_FUNC_ID; }
"("                       { DEBUG_INFO("%s\n", yytext);                   return L_RBRACKET; }
")"                       { DEBUG_INFO("%s\n", yytext);                   return R_RBRACKET; }
"{"                       { DEBUG_INFO("%s\n", yytext);                   return L_CBRACKET; }
"}"                       { DEBUG_INFO("%s\n", yytext);                   return R_CBRACKET; }
"["                       { DEBUG_INFO("%s\n", yytext);                   return L_SBRACKET; }
"]"                       { DEBUG_INFO("%s\n", yytext);                   return R_SBRACKET; }
"+"                       { DEBUG_INFO("%s\n", yytext);                   return OP_ADD; }
"-"                       { DEBUG_INFO("%s\n", yytext);                   return OP_MINUS; }
"*"                       { DEBUG_INFO("%s\n", yytext);                   return OP_MULT; }
"/"                       { DEBUG_INFO("%s\n", yytext);                   return OP_DIV; }
";"                       { DEBUG_INFO("%s\n", yytext);                   return EOL; }
","                       { DEBUG_INFO("%s\n", yytext);                   return COMMA; }
"="                       { DEBUG_INFO("%s\n", yytext);                   return EQUAL; }
{one_line_comment}        { DEBUG_INFO("comment      ->\t%s\n", yytext);  }                        /* eat up one-line comments */
{identifier}              { DEBUG_INFO("indentifier  ->\t%s\n", yytext);  return ID; }             /* variable / function name */
{float_numbers}           { DEBUG_INFO("float        ->\t%s\n", yytext);  return FLT_NUM; }        /* floating point value */
{integer_numbers}         { DEBUG_INFO("int          ->\t%s\n", yytext);  return INT_NUM; }        /* integer value */
{whitespace}              { }                                            					   /* ignore whitespace */
.                         { }                                                                  /* supress output for the unknown */
%%