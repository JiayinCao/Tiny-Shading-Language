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

TEST(VerifyOutput, Basic_Output) {
    auto shader_source = R"(
        shader function_name( out float data ){
            data = 2.0;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(float*)>(shader_source, shading_system);

    float test_value = 1.0f;
    func_ptr(&test_value);
    EXPECT_EQ(test_value, 2.0f);
}

TEST(VerifyOutput, Complex_Output) {
    auto shader_source = R"(
        shader function_name( float arg0 , float arg1 , float arg2 , out float oarg0 , out float oarg1 ){
            oarg0 = ( arg0 + arg1 ) * arg2;
            oarg1 = ( arg0 - arg1 ) / arg2 * oarg0;
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(float, float, float, float*, float*)>(shader_source, shading_system);

    float arg0 = 2.0f, arg1 = 3.0f, arg2 = 0.5f;
    float oarg0 = 0.0f, oarg1 = 0.0f;
    func_ptr(arg0, arg1, arg2, &oarg0, &oarg1);
    EXPECT_EQ(oarg0, (arg0 + arg1) * arg2);
    EXPECT_EQ(oarg1, (arg0 - arg1) / arg2 * oarg0);
}