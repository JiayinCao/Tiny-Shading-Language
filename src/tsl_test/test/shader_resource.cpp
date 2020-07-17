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

namespace {
    DECLARE_TSLGLOBAL_BEGIN(AnotherTslGlobal)
    DECLARE_TSLGLOBAL_VAR(float3, basecolor)
    DECLARE_TSLGLOBAL_END()

    IMPLEMENT_TSLGLOBAL_BEGIN(AnotherTslGlobal)
    IMPLEMENT_TSLGLOBAL_VAR(float3, basecolor)
    IMPLEMENT_TSLGLOBAL_END()
}

TEST(ShaderResource, SimpleTexture) {
    auto shader_source = R"(
        texture2d g_diffuse;
        shader function_name(out color diffuse){
            color base_color = global_value<basecolor>;
            diffuse = texture2d_sample<g_diffuse>( base_color.r , 2.0f );
        }
    )";

    // tsl global data
    AnotherTslGlobal tsl_global;
    tsl_global.basecolor = make_float3(123.0f);

    // the texture handle
    TextureSimple texture_simple;

    // the tsl shading system
    auto& shading_system = ShadingSystem::get_instance();

    // make a shader context
    auto shading_context = shading_system.make_shading_context();
    
    // compile the shader
    const auto shader_unit_template = shading_context->begin_shader_unit_template("texture_handle_shader");

    // register the texture handle
    shader_unit_template->register_shader_resource("g_diffuse", (const ShaderResourceHandle*)&texture_simple);

    // register tsl_global
    shader_unit_template->register_tsl_global(tsl_global.m_var_list);

    // compile the shader unit
    shading_context->compile_shader_unit_template(shader_unit_template.get(), shader_source);

    // shader unit done.
    shading_context->end_shader_unit_template(shader_unit_template.get());

    // make a shader instance after the template is ready
    auto shader_instance = shader_unit_template->make_shader_instance();
    EXPECT_VALID_SMART_PTR(shader_instance);

    // resolve the shader instance
    const auto resolve_ret = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, resolve_ret);

    // get the raw function pointer for execution
    auto func_ptr = (void(*)(float3*, AnotherTslGlobal*))shader_instance->get_function();
    EXPECT_VALID_RAW_PTR(func_ptr);

    float3 data;
    func_ptr(&data, &tsl_global);
    EXPECT_EQ(123.0f, data.x);
    EXPECT_EQ(2.0f, data.y);
    EXPECT_EQ(1234.0f, data.z);
}

TEST(ShaderResource, SimpleTextureAlpha) {
    auto shader_source = R"(
        texture2d g_diffuse;
        shader function_name(out float diffuse){
            color base_color = global_value<basecolor>;
            diffuse = texture2d_sample_alpha<g_diffuse>( base_color.r , 2.0f );
        }
    )";

    // tsl global data
    AnotherTslGlobal tsl_global;
    tsl_global.basecolor = make_float3(123.0f);

    // the texture handle
    TextureSimple texture_simple;

    // the tsl shading system
    auto& shading_system = ShadingSystem::get_instance();

    // make a shader context
    auto shading_context = shading_system.make_shading_context();

    // compile the shader
    const auto shader_unit_template = shading_context->begin_shader_unit_template("texture_handle_alpha");

    // register the texture handle
    shader_unit_template->register_shader_resource("g_diffuse", (const ShaderResourceHandle *)&texture_simple);

    // register tsl_global
    shader_unit_template->register_tsl_global(tsl_global.m_var_list);

    // compile the shader unit
    shading_context->compile_shader_unit_template(shader_unit_template.get(), shader_source);

    // shader unit done.
    shading_context->end_shader_unit_template(shader_unit_template.get());

    // make a shader instance after the template is ready
    auto shader_instance = shader_unit_template->make_shader_instance();
    EXPECT_VALID_SMART_PTR(shader_instance);

    // resolve the shader instance
    const auto resolve_ret = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, resolve_ret);

    // get the raw function pointer for execution
    auto func_ptr = (void(*)(float*, AnotherTslGlobal*))shader_instance->get_function();
    EXPECT_VALID_RAW_PTR(func_ptr);

    float data;
    func_ptr(&data, &tsl_global);
    EXPECT_EQ(123.0f, data);
}

class CustomShaderResource : public ShaderResourceHandle {
public:
    // just for verification purpose
    const int m_signature = 0x12345678;
};

TEST(ShaderResource, CustomShaderResource) {
    auto shader_source = R"(
        shader_resource custom_data;
        shader function_name(out closure diffuse){
            diffuse = make_closure<measured_brdf>( 123 , custom_data );
        }
    )";

    // the texture handle
    CustomShaderResource custom_data;

    // the tsl shading system
    auto& shading_system = ShadingSystem::get_instance();

    // make a shader context
    auto shading_context = shading_system.make_shading_context();

    // compile the shader
    const auto shader_unit_template = shading_context->begin_shader_unit_template("custom_reousrce_shader");
    EXPECT_VALID_SMART_PTR(shader_unit_template);

    // register the texture handle
    shader_unit_template->register_shader_resource("custom_data", &custom_data);

    // compile the shader unit
    shading_context->compile_shader_unit_template(shader_unit_template.get(), shader_source);

    // shader unit done.
    shading_context->end_shader_unit_template(shader_unit_template.get());

    // make a shader instance after the template is ready
    auto shader_instance = shader_unit_template->make_shader_instance();
    EXPECT_VALID_SMART_PTR(shader_instance);

    // resolve the shader instance
    const auto resolve_ret = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, resolve_ret);

    // get the raw function pointer for execution
    auto func_ptr = (void(*)(Tsl_Namespace::ClosureTreeNodeBase**))shader_instance->get_function();
    EXPECT_VALID_RAW_PTR(func_ptr);

    Tsl_Namespace::ClosureTreeNodeBase* closure = nullptr;
    func_ptr(&closure);

    EXPECT_EQ(g_measured_brdf_id, closure->m_id);
    const ClosureTypeMeasuredBrdf* param = (const ClosureTypeMeasuredBrdf*)closure->m_params;
    EXPECT_EQ(123, param->signature);

    auto handle = (const CustomShaderResource*)param->custom_data;
    EXPECT_EQ(&custom_data, handle);
    EXPECT_EQ(0x12345678, handle->m_signature);
}