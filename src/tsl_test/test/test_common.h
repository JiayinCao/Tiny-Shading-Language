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

#include "gtest/gtest.h"
#include "shading_system.h"
#include "shading_context.h"

USE_TSL_NAMESPACE

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

    const auto shader_unit = shading_context->compile_shader_unit("test", shader_source);
    const auto ret = shader_unit != nullptr;

    EXPECT_EQ(ret, valid);
}

template<class T>
inline T compile_shader(const char* shader_source, ShadingSystem& shading_system) {
    auto shading_context = shading_system.make_shading_context();

    const auto shader_unit = shading_context->compile_shader_unit("test", shader_source);
    const auto ret = shader_unit != nullptr;

    // resolve the shader before using it.
    if(!shader_unit->resolve())
        return nullptr;

    if (!shader_unit)
        return nullptr;
    return (T)shader_unit->get_function();
}