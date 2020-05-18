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

TEST(Output, DISABLED_One_Output) {
    auto shader_source = R"(
        shader function_name( out float data ){
            data = 2.0;
        }
    )";

    // the shading system of TSL
    ShadingSystem shading_system;

    // create a separate shading context
    auto shading_context = shading_system.make_shading_context();

    // try compiling the above resources
    const auto shader_unit = shading_context->compile_shader_unit("test", shader_source);

    // get the function pointer
    auto raw_function = (void(*)(float*))shader_unit->get_function();

    // try calling the function and expect 2.0 since this is what the shader does
    float test_value = 1.0f;
    //raw_function(&test_value);
    EXPECT_EQ(test_value, 2.0f);
}