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

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT float custom_square(float x) {
    return x * x;
}

TEST(CallbackFunction, Basic_Callback) {
    auto shader_source = R"(
        float custom_square(float x);

        shader function_name( float arg0 , out float data ){
            data = custom_square(arg0);
        }
    )";
    
    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(float, float*)>(shader_source, shading_system);

    float arg0 = 2.0f , test_value = 1.0f;
    func_ptr(arg0, &test_value);
    EXPECT_EQ(test_value, 4.0f);
}

TEST(CallbackFunction, System_Callback) {
    auto shader_source = R"(
        double cos(double x);

        shader function_name( double arg0 , out double data ){
            data = cos(arg0);
        }
    )";

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(double, double*)>(shader_source, shading_system);

    double arg0 = 2.0, test_value = 1.0;
    func_ptr(arg0, &test_value);
    EXPECT_EQ(test_value, cos(arg0));
}

TEST(CallbackFunction, Complex_Callback) {
    auto shader_source = R"(
        float custom_square(float x);

        shader function_name( float arg0 , out float data ){
            float local = 1.0;
            float a = arg0 / local;
            float b = (custom_square(a) + local) * (arg0 + 3.0);
            data = custom_square(b+local);
        }
    )";

    auto referenced_func = [](float arg0) {
        float local = 1.0;
        float a = arg0 / local;
        float b = (custom_square(a) + local) * (arg0 + 3.0);
        return custom_square(b + local);
    };

    ShadingSystem shading_system;
    auto func_ptr = compile_shader<void(*)(float, float*)>(shader_source, shading_system);

    float arg0 = 2.0, test_value = 1.0;
    func_ptr(arg0, &test_value);
    EXPECT_EQ(test_value, referenced_func(arg0));
}