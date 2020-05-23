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

#include <unordered_map>
#include <string>
#include "test_common.h"
#include "closure.h"

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambert)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, int, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, float, normal)
DECLARE_CLOSURE_TYPE_END()

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeLambert)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, int, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, float, normal)
IMPLEMENT_CLOSURE_TYPE_END()

TEST(Closure, ClosureMake) {
    ShadingSystem shading_system;
    auto shading_context = shading_system.make_shading_context();

    // shading_system.register_closure_type<ClosureTypeLambert>("lambert");
    auto closure_id = shading_system.register_closure_type("lambert", ClosureTypeLambert::m_offsets, (int)sizeof(ClosureTypeLambert));

    auto shader_source = R"(
        shader closure_make(out closure o0){
            o0 = make_closure<lambert>( 11 , 2.0 );
        }
    )";

    Tsl_Namespace::ClosureTreeNodeBase* root = nullptr;
    auto func_ptr = compile_shader<void(*)(Tsl_Namespace::ClosureTreeNodeBase**)>(shader_source, shading_system);
    func_ptr(&root);

    auto lambert_param = (ClosureTypeLambert*)root->m_params;
	EXPECT_NE(root, nullptr);
	EXPECT_EQ(root->m_id, closure_id);
	EXPECT_NE(root->m_params, nullptr);
    EXPECT_EQ(lambert_param->base_color, 11);
    EXPECT_EQ(lambert_param->normal, 2.0f);
}

// I'm working on it
TEST(Closure, DISABLED_ClosureMul) {
	ShadingSystem shading_system;
	auto shading_context = shading_system.make_shading_context();

	// shading_system.register_closure_type<ClosureTypeLambert>("lambert");
	auto closure_id = shading_system.register_closure_type("lambert", ClosureTypeLambert::m_offsets, (int)sizeof(ClosureTypeLambert));

	auto shader_source = R"(
        shader closure_mul(out closure o0){
            o0 = 3.0 * make_closure<lambert>( 11 , 2.0 );
        }
    )";

	Tsl_Namespace::ClosureTreeNodeMul* root = nullptr;
	auto func_ptr = compile_shader<void(*)(Tsl_Namespace::ClosureTreeNodeMul**)>(shader_source, shading_system);
	func_ptr(&root);

	// auto lambert_param = (ClosureTypeLambert*)root->m_params;
	EXPECT_NE(root, nullptr);
	EXPECT_EQ(root->m_id, CLOSURE_MUL);
	//EXPECT_NE(root->m_params, nullptr);
	//EXPECT_EQ(lambert_param->base_color, 11);
	//EXPECT_EQ(lambert_param->normal, 2.0f);
}