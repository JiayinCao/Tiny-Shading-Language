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
            int a = 1 , b = 2 , c = 4;

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
            int a = 1 , b = 2;

            int sum_test = ( a & b ) ^ ( a | b );

            if( a && b )
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
            int k1 = ( k >> 1 ) | ( k << 3 );
        }
    )");
}

TEST(Math, Unary_Operation) {
    validate_shader(R"(
        shader func(){
            int k = -2;
            int k1 = ~k ;
            
            bool k2 = !k;
            int k3 = -( +k2 );
        }
    )");
}

TEST(Math, VecMulVec) {
    auto shader_source = R"(
        vector make_float3( float x , float y , float z ){
            vector ret;
            ret.x = x; ret.y = y; ret.z = z;
            return ret;
        }

        shader piecewise_mul( out vector data ){
            vector arg0, arg1;
            arg0 = make_float3( 1.0f, 2.0f, 3.0f );
            arg1 = make_float3( 2.0f, 4.0f, 4.0f );
            data = arg0 * arg1;
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 2.0f);
    EXPECT_EQ(v.y, 8.0f);
    EXPECT_EQ(v.z, 12.0f);
}

TEST(Math, VecMulFloat) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            vector arg0 = vector( 1.0f, 2.0f, 3.0f );
            vector arg1 = arg0 * 2.0f;
            data = arg1;
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 2.0f);
    EXPECT_EQ(v.y, 4.0f);
    EXPECT_EQ(v.z, 6.0f);
}

TEST(Math, FloatMulVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            vector arg0 = vector( 1.0f, 2.0f, 3.0f );
            vector arg1 = 2.0f * arg0;
            data = arg1;
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 2.0f);
    EXPECT_EQ(v.y, 4.0f);
    EXPECT_EQ(v.z, 6.0f);
}

TEST(Math, VectorSubFloat) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = vector( 1.0f, 2.0f, 3.0f ) - 2.0f;
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, -1.0f);
    EXPECT_EQ(v.y, 0.0f);
    EXPECT_EQ(v.z, 1.0f);
}

TEST(Math, FloatSubVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = 2.0f - vector( 1.0f, 2.0f, 3.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 1.0f);
    EXPECT_EQ(v.y, 0.0f);
    EXPECT_EQ(v.z, -1.0f);
}

TEST(Math, VectorSubVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = vector( 3.0f, 2.0f, 1.0f ) - vector( 1.0f, 2.0f, 3.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 2.0f);
    EXPECT_EQ(v.y, 0.0f);
    EXPECT_EQ(v.z, -2.0f);
}

TEST(Math, VectorAddFloat) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = vector( 1.0f, 2.0f, 3.0f ) + 2.0f;
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 3.0f);
    EXPECT_EQ(v.y, 4.0f);
    EXPECT_EQ(v.z, 5.0f);
}

TEST(Math, FloatAddVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = 2.0f + vector( 1.0f, 2.0f, 3.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 3.0f);
    EXPECT_EQ(v.y, 4.0f);
    EXPECT_EQ(v.z, 5.0f);
}

TEST(Math, VectorDivVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = vector( 3.0f, 2.0f, 1.0f ) / vector( 1.0f, 2.0f, 3.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 3.0f);
    EXPECT_EQ(v.y, 1.0f);
    EXPECT_EQ(v.z, 1.0f / 3.0f);
}

TEST(Math, VectorDivFloat) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = vector( 1.0f, 2.0f, 3.0f ) / 2.0f;
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 0.5f);
    EXPECT_EQ(v.y, 1.0f);
    EXPECT_EQ(v.z, 1.5f);
}

TEST(Math, FloatDivVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = 2.0f / vector( 1.0f, 2.0f, 3.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 2.0f);
    EXPECT_EQ(v.y, 1.0f);
    EXPECT_EQ(v.z, 2.0f/3.0f);
}

TEST(Math, VectorAddVector) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = vector( 3.0f, 2.0f, 1.0f ) + vector( 1.0f, 2.0f, 3.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, 4.0f);
    EXPECT_EQ(v.y, 4.0f);
    EXPECT_EQ(v.z, 4.0f);
}

TEST(Math, VectorNegate) {
    auto shader_source = R"(
        shader piecewise_mul( out vector data ){
            data = -vector( 3.0f, 2.0f, 1.0f );
        }
    )";

    TslGlobal tsl;
    auto ret = compile_shader<void(*)(float3*, TslGlobal*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;

    func_ptr(&v, &tsl);
    EXPECT_EQ(v.x, -3.0f);
    EXPECT_EQ(v.y, -2.0f);
    EXPECT_EQ(v.z, -1.0f);
}