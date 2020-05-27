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

// this is a very simple real practical use case that demonstrating how tsl can fit in a ray tracing renderer.
TEST(ShaderGroup, BasicShaderGroup) {
    // global tsl shading system
    ShadingSystem shading_system;

    // make a shading context for shader compiling, since there is only one thread involved in this unit test, it is good enough.
    auto shading_context = shading_system.make_shading_context();

    // shading_system.register_closure_type<ClosureTypeLambert>("lambert");
    auto closure_id = shading_system.register_closure_type("Lambert", ClosureTypeLambert::m_offsets, (int)sizeof(ClosureTypeLambert));

    // the root shader node, this usually matches to the output node in material system
    const auto root_shader_unit = shading_context->compile_shader_unit("root_shader", R"(
        shader output_node( in closure in_bxdf , out closure out_bxdf ){
            out_bxdf = in_bxdf * 0.5f;
        }
    )");
    EXPECT_NE(nullptr, root_shader_unit);

    // a bxdf node
    const auto bxdf_shader_unit = shading_context->compile_shader_unit("bxdf_shader", R"(
        shader lambert_node( out closure out_bxdf ){
            out_bxdf = make_closure<Lambert>( 111, 4.0f );
        }
    )");
    EXPECT_NE(nullptr, bxdf_shader_unit);

    // make a shader group
    auto shader_group = shading_context->make_shader_group("first shader");
    EXPECT_NE(nullptr, shader_group);

    // add the two shader units in this group
    auto ret = shader_group->add_shader_unit(root_shader_unit, true);
    EXPECT_EQ(true, ret);
    ret = shader_group->add_shader_unit(bxdf_shader_unit);
    EXPECT_EQ(true, ret);

    // setup connections between shader units
    shader_group->connect_shader_units("bxdf_shader", "out_bxdf", "root_shader", "in_bxdf");

    // resolve the shader group
    ret = shading_context->resolve_shader_unit(shader_group);
    EXPECT_EQ(true, ret);

    // get the function pointer
    auto raw_function = (void(*)(ClosureTreeNodeBase**))shader_group->get_function();
    EXPECT_NE(nullptr, raw_function);

    // execute the shader
    ClosureTreeNodeBase* closure = nullptr;
    raw_function(&closure);
    EXPECT_EQ(CLOSURE_MUL, closure->m_id);

    ClosureTreeNodeMul* mul_closure = (ClosureTreeNodeMul*)closure;
    EXPECT_EQ(0.5f, mul_closure->m_weight);
    EXPECT_EQ(closure_id, mul_closure->m_closure->m_id);

    closure = mul_closure->m_closure;
    ClosureTypeLambert* lambert_param = (ClosureTypeLambert*)closure->m_params;
    EXPECT_EQ(111, lambert_param->base_color);
    EXPECT_EQ(4.0f, lambert_param->normal);
}