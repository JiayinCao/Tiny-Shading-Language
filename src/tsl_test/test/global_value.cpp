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
    DECLARE_TSLGLOBAL_BEGIN(TslGlobal)
    DECLARE_TSLGLOBAL_VAR(float, intensity)
    DECLARE_TSLGLOBAL_VAR(float3, diffuse)
    DECLARE_TSLGLOBAL_END()

    IMPLEMENT_TSLGLOBAL_BEGIN(TslGlobal)
    IMPLEMENT_TSLGLOBAL_VAR(float, intensity)
    IMPLEMENT_TSLGLOBAL_VAR(float3, diffuse)
    IMPLEMENT_TSLGLOBAL_END()
}

TEST(GlobalValue, AccessData) {
    auto shader_source = R"(
        shader function_name(out float var){
            var = global_value<intensity>;
        }
    )";

    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;

    auto ret = compile_shader<void(*)(float*, TslGlobal*), TslGlobal>(shader_source);
    auto func_ptr = ret.first;

    float data = 0.0f;
    func_ptr(&data, &tsl_global);
    EXPECT_EQ(123.0f, data);
}

TEST(GlobalValue, GlobalValueAsDefaultValueForArgument) {
    // register tsl global
    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = ShadingSystem::get_instance().make_shading_context();

    // the root shader node, this usually matches to the output node in material system
    const auto root_shader_unit = compile_shader_unit_template<TslGlobal>(shading_context.get(), "random_root_shader", R"(
        shader output_node( float in_var, out float out_bxdf ){
            out_bxdf = in_var;
        }
    )");
    EXPECT_VALID_SMART_PTR(root_shader_unit);

    // make a shader group
    auto shader_group = shading_context->begin_shader_group_template("GlobalValueAsDefaultValueForArgument");
    EXPECT_VALID_SMART_PTR(shader_group);

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit("root_shader", root_shader_unit, true);
    EXPECT_EQ(true, ret);

    shader_group->register_tsl_global(tsl_global.m_var_list);

    // expose the shader interface
    shader_group->expose_shader_argument("root_shader", "out_bxdf");

    shader_group->init_shader_input("root_shader", "in_var", make_tsl_global_ref("intensity"));

    // resolve the shader group
    auto status = shading_context->end_shader_group_template(shader_group.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    auto shader_instance = shader_group->make_shader_instance();
    status = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    // get the function pointer
    auto raw_function = (void(*)(float*, TslGlobal*))shader_instance->get_function();
    EXPECT_VALID_RAW_PTR(raw_function);

    // execute the shader
    float ret_value = 0.0f;
    raw_function(&ret_value, &tsl_global);
    EXPECT_EQ(ret_value, 123.0f);
}

TEST(GlobalValue, AccessDataFloat3) {
    auto shader_source = R"(
        shader function_name(out float var){
            color diff = global_value<diffuse>;
            var = diff.g;
        }
    )";

    TslGlobal tsl_global;
    tsl_global.intensity = 321.0f;
    tsl_global.diffuse = make_float3(1.0f, 123.0f, 3.0f);

    auto ret = compile_shader<void(*)(float*, TslGlobal*), TslGlobal>(shader_source);
    auto func_ptr = ret.first;

    float data = 0.0f;
    func_ptr(&data, &tsl_global);
    EXPECT_EQ(123.0f, data);
}

TEST(GlobalValue, GlobalValueInShaderGroup_Simple) {
    // register tsl global
    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;
    // TslGlobal::RegisterGlobal(shading_system);

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = ShadingSystem::get_instance().make_shading_context();

    // the root shader node, this usually matches to the output node in material system
    const auto root_shader_unit = compile_shader_unit_template<TslGlobal>(shading_context.get(), "root_shader", R"(
        shader output_node( out float out_bxdf ){
            out_bxdf = global_value<intensity>;
        }
    )");
    EXPECT_VALID_SMART_PTR(root_shader_unit);

    // make a shader group
    auto shader_group = shading_context->begin_shader_group_template("GlobalValueInShaderGroup_Simple");
    EXPECT_VALID_SMART_PTR(shader_group);

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit("root_shader", root_shader_unit, true);
    EXPECT_EQ(true, ret);

    // register tsl global
    shader_group->register_tsl_global(tsl_global.m_var_list);

    // expose the shader interface
    shader_group->expose_shader_argument("root_shader", "out_bxdf");

    // resolve the shader group
    auto status = shading_context->end_shader_group_template(shader_group.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    auto shader_instance = shader_group->make_shader_instance();
    status = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    // get the function pointer
    auto raw_function = (void(*)(float*, TslGlobal*))shader_instance->get_function();
    EXPECT_VALID_RAW_PTR(raw_function);

    // execute the shader
    float ret_value = 0.0f;
    raw_function(&ret_value, &tsl_global);
    EXPECT_EQ(ret_value, 123.0f);
}

TEST(GlobalValue, GlobalValueInShaderGroup) {
    // global tsl shading system
    auto& shading_system = ShadingSystem::get_instance();

    // register tsl global
    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = shading_system.make_shading_context();

    // the root shader node, this usually matches to the output node in material system
    const auto root_shader_unit = compile_shader_unit_template<TslGlobal>(shading_context.get(), "root_shader_GlobalValueInShaderGroup", R"(
        shader output_node( in closure in_bxdf , out closure out_bxdf ){
            out_bxdf = in_bxdf * global_value<intensity>;
        }
    )");
    EXPECT_VALID_SMART_PTR(root_shader_unit);

    // a bxdf node
    const auto bxdf_shader_unit = compile_shader_unit_template<TslGlobal>(shading_context.get(), "bxdf_shader_GlobalValueInShaderGroup", R"(
        shader lambert_node( out closure out_bxdf ){
            out_bxdf = make_closure<lambert>( 111, 4.0f );
        }
    )");
    EXPECT_VALID_SMART_PTR(bxdf_shader_unit);

    // make a shader group
    auto shader_group = shading_context->begin_shader_group_template("GlobalValueInShaderGroup");
    EXPECT_VALID_SMART_PTR(shader_group);

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit("root_shader_GlobalValueInShaderGroup", root_shader_unit, true);
    EXPECT_EQ(true, ret);
    ret = shader_group->add_shader_unit("bxdf_shader_GlobalValueInShaderGroup", bxdf_shader_unit);
    EXPECT_EQ(true, ret);

    // setup connections between shader units
    shader_group->connect_shader_units("bxdf_shader_GlobalValueInShaderGroup", "out_bxdf", "root_shader_GlobalValueInShaderGroup", "in_bxdf");

    // register tsl global
    shader_group->register_tsl_global(tsl_global.m_var_list);

    // expose the shader interface
    shader_group->expose_shader_argument("root_shader_GlobalValueInShaderGroup", "out_bxdf");

    // resolve the shader group
    auto status = shading_context->end_shader_group_template(shader_group.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    auto shader_instance = shader_group->make_shader_instance();
    status = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed, status);

    // get the function pointer
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader_instance->get_function();
    EXPECT_VALID_RAW_PTR(raw_function);

    // execute the shader
    ClosureTreeNodeBase* closure = nullptr;
    raw_function(&closure, &tsl_global);
    EXPECT_EQ(CLOSURE_MUL, closure->m_id);

    ClosureTreeNodeMul* mul_closure = (ClosureTreeNodeMul*)closure;
    EXPECT_EQ(123.0f, mul_closure->m_weight);
    EXPECT_EQ(g_lambert_closure_id, mul_closure->m_closure->m_id);

    closure = mul_closure->m_closure;
    ClosureTypeLambert* lambert_param = (ClosureTypeLambert*)closure->m_params;
    EXPECT_EQ(111, lambert_param->base_color);
    EXPECT_EQ(4.0f, lambert_param->normal);
}