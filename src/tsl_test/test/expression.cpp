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

TEST(Expression, Simple_Test) {
    validate_shader(R"(
        shader func(){
            int k = 234234;
        }
    )");
}

TEST(Expression, MathOperation) {
    validate_shader(R"(
        shader func(){
            int a = 23;
            int k = a + 2;
            int k2 = k - a;
            int g = k * k;
            int w = k / k;
        }
    )");
}

/*
TEST(Expression, Compound_Expression) {
    validate_shader(R"(
        shader func(){
            int g = ( 1 + 2 , 34 );
        }
    )");
}
TEST(Expression, Recursive_Compound_Expression) {
    validate_shader(R"(
        shader func(){
            int g = ( 1 + 2 , ( 34 + k ) );
        }
    )");
}
*/

TEST(Expression, Type_Cast) {
    validate_shader(R"(
        shader func(){
            // int g = (int) 23.0;
            // float k = (float) 2;
        }
    )");
}