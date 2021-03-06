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
/*
    --------------------------------------------------------------------
    WARNING:
            This file is automatically generated, do not modify.
    --------------------------------------------------------------------
*/
    
  #include <string.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <iostream>
  #include "tsl_version.h"
  #include "tsl_define.h"
  #include "compiler/compiler.h"
  #include "compiler/str_helper.h"
  #include "compiled_grammar.hpp"

  #if __cplusplus > 199711L
    #define register      // Deprecated in C++11.
  #endif  // #if __cplusplus > 199711L

    #define YY_USER_ACTION \
        {\
            yylloc->first_line   = yylloc->last_line;\
            yylloc->first_column = yylloc->last_column;\
            int curr_line = yylloc->first_line;\
            int curr_col = yylloc->first_column;\
            for(char* s = yytext; *s != '\0'; s++){\
                if(*s == '\n'){\
                    curr_line++;\
                    curr_col = 1;\
                }else{\
                    curr_col++;\
                }\
            }\
            yylloc->last_line   = curr_line;\
            yylloc->last_column = curr_col - 1;\
        }
%}

%option outfile="generated_src/compiled_lex.cpp"
%option noyywrap
%option never-interactive
%option yylineno
%option reentrant
%option bison-bridge
%option bison-locations

alpha                    [a-zA-Z]
digit                    [0-9]
integer_numbers          {digit}+
hex_numbers              0[xX][0-9a-fA-F]+
exp                      [eE][-+]?{digit}+
flt1                     {digit}+\.{digit}*{exp}?
flt2                     {digit}*\.{digit}+{exp}?
flt3                     {digit}+{exp}
float_numbers            ({flt1}|{flt2}|{flt3})f?
double_numbers           ({flt1}|{flt2}|{flt3})d
one_line_comment         (\/\/)(.*\n)
multi_line_comment       \/\*[^*]*\*+(?:[^\/*][^*]*\*+)*\/
identifier               ({alpha}|[_])({alpha}|{digit}|[_])*
whitespace               [ \t\r\n\f]+

%%
"struct"				  { return STRUCT; }
"make_closure"            { return MAKE_CLOSURE; }
"global_value"            { return GLOBAL_VALUE; }
"shader_resource"         { return SHADER_RESOURCE_HANDLE; }
"texture2d"               { return TEXTURE2D_HANDLE; }
"texture2d_sample"        { return TEXTURE2D_SAMPLE; }
"texture2d_sample_alpha"  { return TEXTURE2D_SAMPLE_ALPHA; }
"closure"                 { return CLOSURE; }
"true"                    { return TRUE; }
"false"                   { return FALSE; }
"do"                      { return DO; }
"while"                   { return WHILE; }
"for"                     { return FOR; }
"break"                   { return BREAK; }
"continue"                { return CONTINUE; }
"if"                      { return IF; }
"else"                    { return ELSE; }
"++"                      { return INC_OP; }
"--"                      { return DEC_OP; }
"in"					  { return IN; }
"const"					  { return CONST; }
"out"					  { return OUT; }
":"                       { return COLON; }
"?"                       { return QUESTION_MARK; }
"return"                  { return RETURN; }
"<<<"                     { return METADATA_START; }
">>>"                     { return METADATA_END; }
"int"                     { return TYPE_INT; }
"float"                   { return TYPE_FLOAT; }
"double"                  { return TYPE_DOUBLE; }
"void"                    { return TYPE_VOID; }
"color"					  { return TYPE_COLOR; }
"vector"				  { return TYPE_VECTOR; }
"bool"                    { return TYPE_BOOL; }
"."                       { return DOT; }
"shader"                  { return SHADER_FUNC_ID; }
"("                       { return L_RBRACKET; }
")"                       { return R_RBRACKET; }
"{"                       { return L_CBRACKET; }
"}"                       { return R_CBRACKET; }
"["                       { return L_SBRACKET; }
"]"                       { return R_SBRACKET; }
"+"                       { return OP_ADD; }
"-"                       { return OP_MINUS; }
"*"                       { return OP_MULT; }
"/"                       { return OP_DIV; }
"%"                       { return OP_MOD; }
"+="                      { return OP_ADD_ASSIGN; }
"-="                      { return OP_MINUS_ASSIGN; }
"*="                      { return OP_MULT_ASSIGN; }
"/="                      { return OP_DIV_ASSIGN; }
"%="                      { return OP_MOD_ASSIGN; }
";"                       { return EOL; }
","                       { return COMMA; }
"="                       { return OP_ASSIGN; }
"&"                       { return OP_AND; }
"|"                       { return OP_OR; }
"^"                       { return OP_XOR; }
"&&"                      { return OP_LOGIC_AND; }
"||"                      { return OP_LOGIC_OR; }
"=="                      { return OP_EQ; }
"!="                      { return OP_NE; }
">="                      { return OP_GE; }
">"                       { return OP_G; }
"<="                      { return OP_LE; }
"<"                       { return OP_L; }
"<<"                      { return OP_SHL; }
">>"                      { return OP_SHR; }
"&="                      { return OP_AND_ASSIGN; }
"|="                      { return OP_OR_ASSIGN; }
"^="                      { return OP_XOR_ASSIGN; }
"<<="                     { return OP_SHL_ASSIGN; }
">>="                     { return OP_SHR_ASSIGN; }
"~"                       { return OP_COMP; }
"!"                       { return OP_NOT; }

{identifier}              { 
                                yylval->s = Tsl_Namespace::make_str_unique(yytext);
                                return ID;
                          }

{float_numbers}           { 
                                yylval->f = atof(yytext);
                                return FLT_NUM; 
                          }

{double_numbers}          {
                                yylval->d = std::stod(yytext);
                                return DBL_NUM;
                          }

{integer_numbers}         {
                                // no over flow control for now
                                yylval->i = std::atoi(yytext);
                                return INT_NUM;
                          }

{hex_numbers}             {
                                // no over flow control for now
                                yylval->i = std::stoi("0xeffffff", 0 , 16);
                                return INT_NUM;
                          }

{one_line_comment}        { /* eat up one-line comments */ }
{multi_line_comment}      { /* eat up multi-line comments */ }
{whitespace}              { /* ignore whitespace */ }
.                         { /* supress output for the unknown */ }
%%
