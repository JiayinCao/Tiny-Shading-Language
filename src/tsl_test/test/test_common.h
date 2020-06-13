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

#pragma once

#include <memory>
#include "gtest/gtest.h"
#include "shading_system.h"
#include "shading_context.h"
#include "global.h"

USE_TSL_NAMESPACE

DECLARE_TSLGLOBAL_BEGIN()
DECLARE_TSLGLOBAL_VAR(Tsl_Namespace::MemoryAllocator*, m_allocator);
DECLARE_TSLGLOBAL_END()

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambert)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, int, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, float, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLambert)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacet)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, specular)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacet)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLayeredBxdf)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, float, specular)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, void*, closure)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLayeredBxdf)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeBxdfWithDouble)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeBxdfWithDouble, double, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeBxdfWithDouble, float, specular)
DECLARE_CLOSURE_TYPE_END(ClosureTypeBxdfWithDouble)

inline void validate_shader(const char* shader_source, bool valid = true, TslCompiler* compiler = nullptr) {
    ShadingSystem shading_system;
    auto shading_context = shading_system.make_shading_context();

    const auto shader_unit = shading_context->compile_shader_unit_template("test", shader_source);
    const auto ret = shader_unit != nullptr;

    EXPECT_EQ(ret, valid);
}

template<class T>
inline std::pair<T, std::unique_ptr<ShaderInstance>> compile_shader(const char* shader_source, ShadingSystem& shading_system) {
    auto shading_context = shading_system.make_shading_context();

    // register the tsl global data structure
    TslGlobal::RegisterGlobal(shading_system);

    const auto shader_unit_template = shading_context->compile_shader_unit_template("test", shader_source);

    if (!shader_unit_template)
        return std::make_pair(nullptr, nullptr);
    
    auto shader_instance = shader_unit_template->make_shader_instance();

    // resolve the shader before using it.
    if(!shading_context->resolve_shader_instance(shader_instance.get()))
        return std::make_pair(nullptr, nullptr);

    return std::make_pair((T)shader_instance->get_function(), std::move(shader_instance));
}
