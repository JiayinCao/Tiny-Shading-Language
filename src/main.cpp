#include <iostream>
using namespace std;

extern "C"{
    int yyparse();
    void yyerror(const char * p);
    int yylex();
}

int main(){
    int result = yyparse();
    if( 0 == result )
        cout<<"input is valid"<<endl;
    else
        cout<<"input is invalid"<<endl;
    return 0;
}