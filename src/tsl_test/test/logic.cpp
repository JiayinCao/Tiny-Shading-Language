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

TEST(Logic, Basic_Test) {
    auto shader_source = R"(
        shader function_name( int arg0 , out float data ){
            if( arg0 != 0 )
                data = 3.0;
            else
                data = 2.0;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int,float*)>(shader_source, shading_system);

    float test_value = 1.0f;
    func_ptr(2, &test_value);
    EXPECT_EQ(test_value, 3.0f);

    func_ptr(0, &test_value);
    EXPECT_EQ(test_value, 2.0f);
}

TEST(Logic, Ternary_Operation) {
    auto shader_source = R"(
        shader func(int a, int b, int c, out int o0 , out int o1){
            o0 = ( a != 0 ) ? b : c;
            o1 = ( o0 < 100 ) ? c : 12;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int, int, int*, int*)>(shader_source, shading_system);

    int a = 12, b = 32, c = 0, o0 = 0, o1 = 0;
    func_ptr(a, b, c, &o0, &o1);
    EXPECT_EQ(o0, 32);
    EXPECT_EQ(o1, 0);
}

TEST(Logic, Compound_Condition) {
    validate_shader(R"(
        shader func(){
            int flag = 1;

            if( flag , asfdf ){
                int test = 0;
            }
        }
    )");
}

TEST(Logic, While_Loop) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            while( k++ < 100 ){
                k += 1;
                ++k;
            }
        }
    )");
}

TEST(Logic, Do_While_Loop) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            do{
                k += 1;
                ++k;
            }while( k++ < 100 );

            do
                k += 1;
            while( k++ < 100 );
        }
    )");
}

TEST(Logic, For_Loop) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            for( ; k < 100 ; ++k ){
                k += 2;
            }

            for( k = 2 ; k < 100 ; )
                ++k;
  
            for( k = 1 ; k < 100 ; ++k )
                ++k;
        }
    )");
}