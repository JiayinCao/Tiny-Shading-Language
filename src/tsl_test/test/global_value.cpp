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

TEST(GlobalValue, AccessData) {
    auto shader_source = R"(
        shader function_name(out float var){
            var = global_value<intensity>;
        }
    )";

    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;

    ShadingSystem shading_system;
    auto ret = compile_shader<void(*)(float*, TslGlobal*)>(shader_source, shading_system);
    auto func_ptr = ret.first;

    float data = 0.0f;
    func_ptr(&data, &tsl_global);
    EXPECT_EQ(123.0f, data);
}

class SimpleMemoryAllocator : public MemoryAllocator {
public:
    void* allocate(unsigned int size) const override {
        return (void*)new char[size];
    }
};

TEST(GlobalValue, GlobalValueInShaderGroup_Simple) {
    // global tsl shading system
    ShadingSystem shading_system;

    // register tsl global
    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;
    TslGlobal::RegisterGlobal(shading_system);

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = shading_system.make_shading_context();

    // registered closure id
    const auto closure_id = ClosureTypeLambert::RegisterClosure("Lambert", shading_system);

    // the root shader node, this usually matches to the output node in material system
    const auto root_shader_unit = shading_context->compile_shader_unit_template("root_shader", R"(
        shader output_node( out float out_bxdf ){
            out_bxdf = global_value<intensity>;
        }
    )");
    EXPECT_NE(nullptr, root_shader_unit);

    // make a shader group
    auto shader_group = shading_context->begin_shader_group_template("first shader");
    EXPECT_NE(nullptr, shader_group);

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit("root_shader", root_shader_unit, true);
    EXPECT_EQ(true, ret);

    // expose the shader interface
    ArgDescriptor arg;
    arg.m_name = "out_bxdf";
    arg.m_type = TSL_TYPE_FLOAT;
    arg.m_is_output = true;
    shader_group->expose_shader_argument("root_shader", "out_bxdf", arg);

    // resolve the shader group
    ret = shading_context->end_shader_group_template(shader_group);
    EXPECT_EQ(true, ret);

    auto shader_instance = shader_group->make_shader_instance();
    ret = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(true, ret);

    // get the function pointer
    auto raw_function = (void(*)(float*, TslGlobal*))shader_instance->get_function();
    EXPECT_NE(nullptr, raw_function);

    // execute the shader
    float ret_value = 0.0f;
    raw_function(&ret_value, &tsl_global);
    EXPECT_EQ(ret_value, 123.0f);
}

TEST(GlobalValue, GlobalValueInShaderGroup) {
    // global tsl shading system
    ShadingSystem shading_system;

    Tsl_MemoryAllocator ma;
    shading_system.register_memory_allocator(&ma);

    // register tsl global
    TslGlobal tsl_global;
    tsl_global.intensity = 123.0f;
    TslGlobal::RegisterGlobal(shading_system);

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = shading_system.make_shading_context();

    // registered closure id
    const auto closure_id = ClosureTypeLambert::RegisterClosure("Lambert", shading_system);

    // the root shader node, this usually matches to the output node in material system
    const auto root_shader_unit = shading_context->compile_shader_unit_template("root_shader", R"(
        shader output_node( in closure in_bxdf , out closure out_bxdf ){
            out_bxdf = in_bxdf * global_value<intensity>;
        }
    )");
    EXPECT_NE(nullptr, root_shader_unit);

    // a bxdf node
    const auto bxdf_shader_unit = shading_context->compile_shader_unit_template("bxdf_shader", R"(
        shader lambert_node( out closure out_bxdf ){
            out_bxdf = make_closure<Lambert>( 111, 4.0f );
        }
    )");
    EXPECT_NE(nullptr, bxdf_shader_unit);

    // make a shader group
    auto shader_group = shading_context->begin_shader_group_template("first shader");
    EXPECT_NE(nullptr, shader_group);

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit("root_shader", root_shader_unit, true);
    EXPECT_EQ(true, ret);
    ret = shader_group->add_shader_unit("bxdf_shader_test", bxdf_shader_unit);
    EXPECT_EQ(true, ret);

    // setup connections between shader units
    shader_group->connect_shader_units("bxdf_shader_test", "out_bxdf", "root_shader", "in_bxdf");

    // expose the shader interface
    ArgDescriptor arg;
    arg.m_name = "out_bxdf";
    arg.m_type = TSL_TYPE_CLOSURE;
    arg.m_is_output = true;
    shader_group->expose_shader_argument("root_shader", "out_bxdf", arg);

    // resolve the shader group
    ret = shading_context->end_shader_group_template(shader_group);
    EXPECT_EQ(true, ret);

    auto shader_instance = shader_group->make_shader_instance();
    ret = shading_context->resolve_shader_instance(shader_instance.get());
    EXPECT_EQ(true, ret);

    // get the function pointer
    auto raw_function = (void(*)(ClosureTreeNodeBase**, TslGlobal*))shader_instance->get_function();
    EXPECT_NE(nullptr, raw_function);

    // execute the shader
    ClosureTreeNodeBase* closure = nullptr;
    raw_function(&closure, &tsl_global);
    EXPECT_EQ(CLOSURE_MUL, closure->m_id);

    ClosureTreeNodeMul* mul_closure = (ClosureTreeNodeMul*)closure;
    EXPECT_EQ(123.0f, mul_closure->m_weight);
    EXPECT_EQ(closure_id, mul_closure->m_closure->m_id);

    closure = mul_closure->m_closure;
    ClosureTypeLambert* lambert_param = (ClosureTypeLambert*)closure->m_params;
    EXPECT_EQ(111, lambert_param->base_color);
    EXPECT_EQ(4.0f, lambert_param->normal);
}