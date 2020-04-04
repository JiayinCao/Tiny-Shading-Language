%{
  #include <iostream>
  using namespace std;
  extern "C" int yylex();
%}

%%
(\/\/)(.*\n)                        cout<<"comment ->\t"<<yytext<<endl;   /* eat up one-line comments */
[a-zA-Z_]+[a-zA-Z_0-9]*             cout<<"word    ->\t"<<yytext<<endl;   /* variable / function name */
([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)     cout<<"float   ->\t"<<yytext<<endl;   /* floating point value */
[0-9]+                              cout<<"int     ->\t"<<yytext<<endl;   /* integer value */
"("                                 cout<<yytext<<endl;
")"                                 cout<<yytext<<endl;
"{"                                 cout<<yytext<<endl;
"}"                                 cout<<yytext<<endl;
"+"                                 cout<<yytext<<endl;
"-"                                 cout<<yytext<<endl;
"*"                                 cout<<yytext<<endl;
"/"                                 cout<<yytext<<endl;
";"                                 cout<<yytext<<endl;
"<<<"                               cout<<yytext<<endl;
">>>"                               cout<<yytext<<endl;
","                                 cout<<yytext<<endl;
"="                                 cout<<yytext<<endl;
[\t\r\n\f]                          ;                                     /* ignore whitespace */
.                                   ;                                     /* supress output for the unknown */
%%

int main() {
  yylex();
}
