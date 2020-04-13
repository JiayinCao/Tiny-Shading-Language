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
        cout<<"This is a valid shader!"<<endl;
    else {
        cout << "Something is wrong!" << endl;
        return -1;
    }

    return 0;
}