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

class TextureSimple : public TextureHandle {
public:
    float4 sample2d(float u, float v) const override {
        return { u, v, 0.0f, 1.0f };
    }
};

TEST(Texture, DISABLED_SimpleTexture) {
    auto shader_source = R"(
        texture2d g_diffuse;
        shader function_name(out float var){
            texture2d_sample<g_diffuse>( 1.0f , 2.0f );
            // float t = global_value<intensity>;
            //float4 ret = texture2d_sample<g_diffuse>( 1.0f , 2.0f );
            //var = ret.b;
        }
    )";

    // tsl global data
    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;

    // the texture handle
    TextureSimple texture_simple;

    // the tsl shading system
    ShadingSystem shading_system;
    
    // make a shader context
    auto shading_context = shading_system.make_shading_context();

    // register the tsl global data structure
    TslGlobal::RegisterGlobal(shading_system);

    // compile the shader
    const auto shader_unit_template = shading_context->compile_shader_unit_template("test", shader_source);
    EXPECT_NE(nullptr, shader_unit_template);

    // register the texture handle
    shader_unit_template->register_texture("g_diffuse", &texture_simple);

    // make a shader instance after the template is ready
    auto shader_instance = shader_unit_template->make_shader_instance(); 
    EXPECT_NE(nullptr, shader_instance);

    // resolve the shader instance
    const auto resolve_ret = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(true, resolve_ret);

    // get the raw function pointer for execution
    auto func_ptr = (void(*)(float*, TslGlobal*))shader_instance->get_function();
    EXPECT_NE(nullptr, func_ptr);

    float data = 0.0f;
    func_ptr(&data, &tsl_global);
    EXPECT_EQ(123.0f, data);
}