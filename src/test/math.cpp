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

#include "test_common.h"

TEST(Math, Full_Test) {
    validate_shader(R"(
        shader func(){
            int sum_test = a + b;
            sum_test += c;

            int minus_test = a - b;
            minus_test -= c;
            
            int multi_test = a * b;
            multi_test *= c;
            
            int div_test = a / b;
            div_test /= c;

            int mod_test = a % b;
            mod_test %= c;
        }
    )");
}

TEST(Math, Bit_Operation) {
    validate_shader(R"(
        shader func(){
            int sum_test = ( a & b ) ^ ( a | b );

            //if( flag && flag2 )

            int test = flag && flag2;
            {
                sum_test = ( a & b ) ^ ( a | b );
            }
        }
    )");
}

TEST(Math, Assigns) {
    validate_shader(R"(
        shader func(){
            int k = 0, a = 1;
            k += a;
            k -= a;
            k *= a;
            k /= a;
            k %= a;
            k &= a;
            k |= a;
            k ^= a;
            k <<= a;
            k >>= a;
        }
    )");
}

TEST(Math, Bit_Shifts) {
    validate_shader(R"(
        shader func(){
            int k = ( 1 << 2 ) << 3;
            int k1 = ( k >> 1 ) | ( ( k << 3 ) , k ) ;
        }
    )");
}

TEST(Math, Unary_Operation) {
    validate_shader(R"(
        shader func(){
            int k = -2;
            int k1 = ~k ;
            
            int k2 = !k;
            int k3 = -( +k2 );
        }
    )");
}