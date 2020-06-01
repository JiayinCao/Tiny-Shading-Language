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
            if( arg0 )
                data = 3.0;
            else
                data = 2.0;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int,float*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    float test_value = 1.0f;
    func_ptr(2, &test_value);
    EXPECT_EQ(test_value, 3.0f);

    func_ptr(0, &test_value);
    EXPECT_EQ(test_value, 2.0f);
}

TEST(Logic, Ternary_Operation) {
    auto shader_source = R"(
        shader func(int a, int b, int c, out int o0 , out int o1){
            o0 = ( a ) ? b : c;
            o1 = ( o0 < 100 ) ? c : 12;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int, int, int*, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int a = 12, b = 32, c = 0, o0 = 0, o1 = 0;
    func_ptr(a, b, c, &o0, &o1);
    EXPECT_EQ(o0, 32);
    EXPECT_EQ(o1, 0);
}

TEST(Logic, Logic_And) {
    auto shader_source = R"(
        shader func(int a, int b, int c, out int o0 , out int o1){
            if( a && c ){
                o0 = a * b;
            }else if( 0 ){
                o0 = 12;
            }else
                o0 = ( a + b ) / b;

            o1 = ( a && c ) ? a * b : 12;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int, int, int*, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int a = 12, b = 32, c = 0, o0 = 0, o1 = 0;
    func_ptr(a, b, c, &o0, &o1);
    EXPECT_EQ(o0, (a + b) / b);
    EXPECT_EQ(o1, 12);
}

TEST(Logic, While_Loop) {
    auto shader_source = R"(
        shader main( int cnt, out int arg2 ){
			int k = cnt;
			int g = 0;
			while( k && --k ){
                if( k % 3 == 1 )
                    g = g + 1;
			}

            arg2 = g;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int o1 = 0;
    func_ptr(100, &o1);
    EXPECT_EQ(o1, 99 / 3);

    func_ptr(0, &o1);
    EXPECT_EQ(o1, 0);
}

TEST(Logic, Do_While_Loop) {
    auto shader_source = R"(
        shader main( int cnt , out int arg2 ){
			int k = 1;
			int g = 0;
			do{
                if( k % 3 == 1 )
                    g = g + 1;
                k = k + 1;
			}while( k < cnt );
            
            arg2 = g;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int o1 = 0;
    func_ptr(100, &o1);
    EXPECT_EQ(o1, 99 / 3);

    func_ptr(1, &o1);
    EXPECT_EQ(o1, 1);
}

TEST(Logic, For_Loop) {
    auto shader_source = R"(
        shader main( int cnt , out int arg2 ){
			int k = 1;
			int g = 0;
			for(; k < cnt ; ++k ){
                if( k % 3 == 1 )
                    g = g + 1;
			}
            
            arg2 = g;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int o1 = 0;
    func_ptr(100, &o1);
    EXPECT_EQ(o1, 99 / 3);

    func_ptr(1, &o1);
    EXPECT_EQ(o1, 0);
}

TEST(Logic, While_Break_Continue) {
    auto shader_source = R"(
        shader main( int cnt , out int arg2 ){
			int k = 1;
			int g = 0;
			while( k < cnt ){
                if( k % 3 == 0 ){
                    k = k + 1;
                    continue;
                }

                g = g + 1;
                if( k > 20 )
                   break;
                k = k + 1;
			}
            
            arg2 = g;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int o1 = 0;
    func_ptr(100, &o1);
    EXPECT_EQ(15, o1);

    func_ptr(1, &o1);
    EXPECT_EQ(0, o1);
}

TEST(Logic, DoWhile_Break_Continue) {
    auto shader_source = R"(
        shader main( int cnt , out int arg2 ){
			int k = 1;
			int g = 0;
			do{
                if( k % 3 == 0 ){
                    k = k + 1;
                    continue;
                }

                g = g + 1;
                if( k > 20 )
                   break;
                k = k + 1;
			}while( k < cnt );
            
            arg2 = g;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int o1 = 0;
    func_ptr(100, &o1);
    EXPECT_EQ(15, o1);

    func_ptr(1, &o1);
    EXPECT_EQ(1, o1);
}

TEST(Logic, For_Break_Continue) {
    auto shader_source = R"(
        shader main( int cnt , out int arg2 ){
            int g = 0;
            int kk = 0;
            for( int k = 1 ; k < cnt ; ++k ){
                if( k % 3 == 0 ){
                    continue;
                }

                g = g + 1;
                if( k > 20 )
                   break;
            }
            arg2 = g;
        }
    )";

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(int, int*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    int o1 = 0;
    func_ptr(100, &o1);
    EXPECT_EQ(15, o1);

    func_ptr(1, &o1);
    EXPECT_EQ(0, o1);
}