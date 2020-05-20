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

TEST(Basic, MathOps) {
    auto shader_source = R"(
        shader function_name( int a , int b , out int o0 , out int o1 , out int o2 , out int o3 ){
            o0 = a + b;
            o1 = a - b;
            o2 = a * b;
            o3 = a / b;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int, int*, int*, int*, int*)>(shader_source, shading_system);

    auto verify_func = [&](int a, int b) {
        int o0, o1, o2, o3;
        func_ptr(a, b, &o0, &o1, &o2, &o3);
        EXPECT_EQ(a + b, o0);
        EXPECT_EQ(a - b, o1);
        EXPECT_EQ(a * b, o2);
        EXPECT_EQ(a / b, o3);
    };

    verify_func(1, 1);
    verify_func(23, 12);
    verify_func(0, 1024);
    verify_func(1, 213);
}

TEST(Basic, Inc_Dec) {
    auto shader_source = R"(
        shader function_name(int a, out int o0, out int o1, out int o2, 
                             out int o3, out int o4, out int o5, out int o6, out int o7){
            int b = a;
            o0 = b++;
            o4 = b;
            b = a;
            o1 = ++b;
            o5 = b;
            b = a;
            o2 = b--;
            o6 = b;
            b = a;
            o3 = --b;
            o7 = b;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int*, int*, int*, int*, int*, int*, int*, int*)>(shader_source, shading_system);

    auto verify_func = [&](int a) {
        int o0, o1, o2, o3, o4, o5, o6, o7;
        func_ptr(a, &o0, &o1, &o2, &o3, &o4, &o5, &o6, &o7);
        EXPECT_EQ(a , o0);
        EXPECT_EQ(a + 1, o1);
        EXPECT_EQ(a , o2);
        EXPECT_EQ(a - 1, o3);
        EXPECT_EQ(a + 1, o4);
        EXPECT_EQ(a + 1, o5);
        EXPECT_EQ(a - 1, o6);
        EXPECT_EQ(a - 1, o7);
    };

    verify_func(1);
    verify_func(23);
    verify_func(0);

    // make sure it has the same overflow behavior as c++ code.
    verify_func(std::numeric_limits<int>::min());
    verify_func(std::numeric_limits<int>::max());
}

TEST(Basic, And_Or_Xor) {
    auto shader_source = R"(
        shader function_name(int a, int b, out int o0, out int o1, out int o2){
            o0 = a & b;
            o1 = a | b;
            o2 = a ^ b;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(int, int, int*, int*, int*)>(shader_source, shading_system);

    auto verify_func = [&](int a, int b) {
        int o0, o1, o2;
        func_ptr(a, b, &o0, &o1, &o2);
        EXPECT_EQ(a & b, o0);
        EXPECT_EQ(a | b, o1);
        EXPECT_EQ(a ^ b, o2);
    };

    verify_func(1, 12);
    verify_func(23, 0x3232);
    verify_func(0, 0xffffffff);

    // make sure it has the same overflow behavior as c++ code.
    verify_func(std::numeric_limits<int>::min(), 12);
    verify_func(std::numeric_limits<int>::max(), 12);
}