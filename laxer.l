%{
  // nothing for now
%}

integer_numbers          [+-]?[0-9]+
float_numbers            [+-]?([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)
one_line_comment         (\/\/)(.*\n)
identifier               [a-zA-Z_]+[a-zA-Z_0-9]*

%%
{one_line_comment}        printf("comment      ->\t%s\n", yytext);   /* eat up one-line comments */
{identifier}              printf("indentifier  ->\t%s\n", yytext);   /* variable / function name */
{float_numbers}           printf("float        ->\t%s\n", yytext);   /* floating point value */
{integer_numbers}         printf("int          ->\t%s\n", yytext);   /* integer value */
"("                       printf("%s\n", yytext);
")"                       printf("%s\n", yytext);
"{"                       printf("%s\n", yytext);
"}"                       printf("%s\n", yytext);
"+"                       printf("%s\n", yytext);
"-"                       printf("%s\n", yytext);
"*"                       printf("%s\n", yytext);
"/"                       printf("%s\n", yytext);
";"                       printf("%s\n", yytext);
"<<<"                     printf("%s\n", yytext);
">>>"                     printf("%s\n", yytext);
","                       printf("%s\n", yytext);
"="                       printf("%s\n", yytext);
[\t\r\n\f]                ;                                     /* ignore whitespace */
.                         ;                                     /* supress output for the unknown */
%%

int main() {
  yylex();
}
