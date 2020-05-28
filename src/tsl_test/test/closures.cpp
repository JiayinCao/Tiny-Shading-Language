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

TEST(Closure, ClosureMake) {
    ShadingSystem shading_system;
    
    // register lambert closure
    const auto closure_id = ClosureTypeLambert::RegisterClosure("lambert", shading_system);

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

// this needs to wait for TSL to support double literal and type conversion later.
TEST(Closure, ClosureMakeWithDouble) {
    ShadingSystem shading_system;
    
    const auto closure_id = ClosureTypeBxdfWithDouble::RegisterClosure("bxdf_with_double", shading_system);

    auto shader_source = R"(
        shader closure_make(out closure o0){
            o0 = make_closure<bxdf_with_double>( 11.0d , 2.0f );
        }
    )";

    Tsl_Namespace::ClosureTreeNodeBase* root = nullptr;
    auto func_ptr = compile_shader<void(*)(Tsl_Namespace::ClosureTreeNodeBase**)>(shader_source, shading_system);
    func_ptr(&root);

    auto bxdf_double_param = (ClosureTypeBxdfWithDouble*)root->m_params;
    EXPECT_NE(root, nullptr);
    EXPECT_EQ(root->m_id, closure_id);
    EXPECT_NE(root->m_params, nullptr);
    EXPECT_EQ(bxdf_double_param->roughness, 11.0);
    EXPECT_EQ(bxdf_double_param->specular, 2.0f);
}

TEST(Closure, ClosureMul) {
    ShadingSystem shading_system;
    
    const auto closure_id = ClosureTypeLambert::RegisterClosure("lambert", shading_system);

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
    EXPECT_NE(root->m_closure, nullptr);
    EXPECT_NE(root->m_closure->m_params, nullptr);
    EXPECT_EQ(root->m_closure->m_id, closure_id);
    auto lambert_param = (ClosureTypeLambert*)root->m_closure->m_params;
    EXPECT_EQ(lambert_param->base_color, 11);
    EXPECT_EQ(lambert_param->normal, 2.0f);
}

TEST(Closure, ClosureAdd) {
	ShadingSystem shading_system;
    const auto closure_id_lambert = ClosureTypeLambert::RegisterClosure("lambert", shading_system);
    const auto closure_id_microfacet = ClosureTypeMicrofacet::RegisterClosure("microfacet", shading_system);

	auto shader_source = R"(
        shader closure_add(out closure o0){
            o0 = make_closure<lambert>( 13 , 4.0 ) + make_closure<microfacet>( 123.0 , 5.0 );
        }
    )";

	Tsl_Namespace::ClosureTreeNodeAdd* root = nullptr;
	auto func_ptr = compile_shader<void(*)(Tsl_Namespace::ClosureTreeNodeAdd**)>(shader_source, shading_system);
	func_ptr(&root);

	EXPECT_NE(root, nullptr);
	EXPECT_EQ(root->m_id, CLOSURE_ADD);
    EXPECT_NE(root->m_closure0, nullptr);
    EXPECT_EQ(root->m_closure0->m_id, closure_id_lambert);
    EXPECT_NE(root->m_closure0->m_params, nullptr);
    auto closure0 = (ClosureTypeLambert*)root->m_closure0->m_params;
    EXPECT_EQ(closure0->base_color, 13);
    EXPECT_EQ(closure0->normal, 4.0f);

    EXPECT_NE(root->m_closure1, nullptr);
    EXPECT_EQ(root->m_closure1->m_id, closure_id_microfacet);
    EXPECT_NE(root->m_closure1->m_params, nullptr);
    auto closure1 = (ClosureTypeMicrofacet*)root->m_closure1->m_params;
    EXPECT_EQ(closure1->roughness, 123.0f);
    EXPECT_EQ(closure1->specular, 5.0f);
}

TEST(Closure, ClosureComplex) {
    ShadingSystem shading_system;
    
    const auto closure_id_lambert = ClosureTypeLambert::RegisterClosure("lambert", shading_system);
    const auto closure_id_microfacet = ClosureTypeMicrofacet::RegisterClosure("microfacet", shading_system);

    auto shader_source = R"(
        shader closure_add(out closure o0){
            o0 = ( 0.3 * make_closure<lambert>( 13 , 4.0 ) + make_closure<microfacet>( 123.0 , 5.0 ) ) * 0.5;
        }
    )";

    Tsl_Namespace::ClosureTreeNodeMul* root = nullptr;
    auto func_ptr = compile_shader<void(*)(Tsl_Namespace::ClosureTreeNodeMul**)>(shader_source, shading_system);
    func_ptr(&root);

    EXPECT_NE(root, nullptr);
    EXPECT_EQ(root->m_id, CLOSURE_MUL);
    EXPECT_NE(root->m_closure, nullptr);
    auto closure = root->m_closure;
    EXPECT_EQ(closure->m_id, CLOSURE_ADD);
    auto closure_add = (ClosureTreeNodeAdd*)closure;
    EXPECT_NE(closure_add->m_closure0, nullptr);
    EXPECT_NE(closure_add->m_closure1, nullptr);

    EXPECT_EQ(closure_add->m_closure0->m_id, CLOSURE_MUL);
    auto closure_lambert_mul = (ClosureTreeNodeMul*)closure_add->m_closure0;
    EXPECT_EQ(closure_lambert_mul->m_weight, 0.3f);
    EXPECT_NE(closure_lambert_mul->m_closure, nullptr);
    EXPECT_NE(closure_lambert_mul->m_closure, nullptr);
    EXPECT_EQ(closure_lambert_mul->m_closure->m_id, closure_id_lambert);
    EXPECT_NE(closure_lambert_mul->m_closure->m_params, nullptr);
    auto lambert_param = (ClosureTypeLambert*)closure_lambert_mul->m_closure->m_params;
    EXPECT_EQ(lambert_param->base_color, 13);
    EXPECT_EQ(lambert_param->normal, 4.0f);

    EXPECT_EQ(closure_add->m_closure1->m_id, closure_id_microfacet);
    auto microfacet_param = (ClosureTypeMicrofacet*)closure_add->m_closure1->m_params;
    EXPECT_EQ(microfacet_param->roughness, 123.0f);
    EXPECT_EQ(microfacet_param->specular, 5.0f);
}

TEST(Closure, ClosureAsOtherClosureInput) {
    ShadingSystem shading_system;
    
    const auto closure_id_layered = ClosureTypeLayeredBxdf::RegisterClosure("layered_bxdf", shading_system);
    const auto closure_id_microfacet = ClosureTypeMicrofacet::RegisterClosure("microfacet", shading_system);

    auto shader_source = R"(
        shader closure_add(out closure o0){
            closure bottom = make_closure<microfacet>( 123.0 , 5.0 );
            o0 = make_closure<layered_bxdf>( 1233.0 , 4.0 , bottom );
        }
    )";

    Tsl_Namespace::ClosureTreeNodeBase* root = nullptr;
    auto func_ptr = compile_shader<void(*)(Tsl_Namespace::ClosureTreeNodeBase**)>(shader_source, shading_system);
    func_ptr(&root);

    EXPECT_NE(root, nullptr);
    EXPECT_EQ(root->m_id, closure_id_layered);
    EXPECT_NE(root->m_params, nullptr);

    ClosureTypeLayeredBxdf* layered_bxdf_params = (ClosureTypeLayeredBxdf*)root->m_params;
    EXPECT_EQ(layered_bxdf_params->roughness, 1233.0);
    EXPECT_EQ(layered_bxdf_params->specular, 4.0);

    ClosureTreeNodeBase* bottom_bxdf = (ClosureTreeNodeBase*)layered_bxdf_params->closure;
    EXPECT_EQ(bottom_bxdf->m_id, closure_id_microfacet);
    ClosureTypeMicrofacet* mf_bxdf = (ClosureTypeMicrofacet*)bottom_bxdf->m_params;

    EXPECT_EQ(mf_bxdf->roughness, 123.0f);
    EXPECT_EQ(mf_bxdf->specular, 5.0f);
}
