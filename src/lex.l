%{
  #include "compiled_grammer.h"
%}

%option outfile="src/generated/compiled_lex.c"
%option noyywrap
%option never-interactive

integer_numbers          [+-]?[0-9]+
float_numbers            [+-]?([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)
one_line_comment         (\/\/)(.*\n)
identifier               [a-zA-Z_]+[a-zA-Z_0-9]*
whitespace               [ \t\r\n\f]

%%
"shader"                  { printf("shader_func  ->\t%s\n", yytext);  return SHADER_FUNC_ID; }
"("                       { printf("%s\n", yytext);                   return L_RBRACKET; }
")"                       { printf("%s\n", yytext);                   return R_RBRACKET; }
"{"                       { printf("%s\n", yytext);                   return L_CBRACKET; }
"}"                       { printf("%s\n", yytext);                   return R_RBRACKET; }
"+"                       { printf("%s\n", yytext);                   return OP_ADD; }
"-"                       { printf("%s\n", yytext);                   return OP_MINUS; }
"*"                       { printf("%s\n", yytext);                   return OP_MULT; }
"/"                       { printf("%s\n", yytext);                   return OP_DIV; }
";"                       { printf("%s\n", yytext);                   return EOL; }
","                       { printf("%s\n", yytext);                   return COMMA; }
"="                       { printf("%s\n", yytext);                   return EQUAL; }
{one_line_comment}        { printf("comment      ->\t%s\n", yytext);  return TO_BE_IGNORED; }  /* eat up one-line comments */
{identifier}              { printf("indentifier  ->\t%s\n", yytext);  return INDENTIFIER; }    /* variable / function name */
{float_numbers}           { printf("float        ->\t%s\n", yytext);  return FLT_NUM; }        /* floating point value */
{integer_numbers}         { printf("int          ->\t%s\n", yytext);  return INT_NUM; }        /* integer value */
{whitespace}              { return TO_BE_IGNORED; }                                            /* ignore whitespace */
.                         { return TO_BE_IGNORED; }                                            /* supress output for the unknown */
%%