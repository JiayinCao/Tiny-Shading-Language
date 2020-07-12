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

#include <stdio.h>
#include <vector>
#include <memory>
#include "gtest/gtest.h"
#include "tsl_system.h"
#include "tsl_args.h"

USE_TSL_NAMESPACE

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambert, "lambert")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, int, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, float, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLambert)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacet, "microfacet")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, specular)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMicrofacet)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeRandom0, "random0")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeRandom0, float3, roughness)
DECLARE_CLOSURE_TYPE_END(ClosureTypeRandom0)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLayeredBxdf, "layered_bxdf")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, float, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, float, specular)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, void*, closure)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLayeredBxdf)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeBxdfWithDouble, "bxdf_with_double")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeBxdfWithDouble, double, roughness)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeBxdfWithDouble, float, specular)
DECLARE_CLOSURE_TYPE_END(ClosureTypeBxdfWithDouble)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambertInSORT, "lambert_in_sort")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambertInSORT, float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambertInSORT, float3, normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLambertInSORT)

DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeMeasuredBrdf, "measured_brdf")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMeasuredBrdf, int, signature)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeMeasuredBrdf, void*, custom_data)
DECLARE_CLOSURE_TYPE_END(ClosureTypeMeasuredBrdf)

extern int g_name_counter;

extern ClosureID g_lambert_closure_id;
extern ClosureID g_random_closure_id;
extern ClosureID g_bxdf_with_double_id;
extern ClosureID g_microfacete_id;
extern ClosureID g_layered_bxdf_id;
extern ClosureID g_lambert_in_sort_id;
extern ClosureID g_measured_brdf_id;

class TextureSimple {
public:
    float3 sample2d(float u, float v) const {
        return make_float3(u, v, 1234.0f);
    }

    float sample_alpha_2d(float u, float v) const {
        return u;
    }
};

inline std::shared_ptr<ShaderUnitTemplate> compile_shader_unit_template(ShadingContext* shading_context, const char* name, const char* shader_source) {
    const auto shader_unit_template = shading_context->begin_shader_unit_template(name);
    const auto ret = shading_context->compile_shader_unit_template(shader_unit_template.get(), shader_source);
    shading_context->end_shader_unit_template(shader_unit_template.get());
    return ret && shader_unit_template ? shader_unit_template : nullptr;
}

template<class TG>
inline std::shared_ptr<ShaderUnitTemplate> compile_shader_unit_template(ShadingContext* shading_context, const char* name, const char* shader_source) {
    const auto shader_unit_template = shading_context->begin_shader_unit_template(name);

    // register tsl shader global
    TG tg;
    shader_unit_template->register_tsl_global(tg.m_var_list);

    const auto ret = shading_context->compile_shader_unit_template(shader_unit_template.get(), shader_source);
    shading_context->end_shader_unit_template(shader_unit_template.get());
    return ret && shader_unit_template ? shader_unit_template : nullptr;
}

inline void validate_shader(const char* shader_source, bool valid = true, TslCompiler* compiler = nullptr) {
    auto shading_context = ShadingSystem::get_instance().make_shading_context();

    const auto name = std::to_string(g_name_counter++);
    const auto shader_unit = compile_shader_unit_template(shading_context.get(), name.c_str(), shader_source);
    const auto ret = shader_unit != nullptr;

    EXPECT_EQ(ret, valid);
}

template<class T>
inline std::pair<T, std::shared_ptr<ShaderInstance>> compile_shader(const char* shader_source) {
    auto shading_context = ShadingSystem::get_instance().make_shading_context();

    // this name is meanless, but I just want something unique
    const auto name = std::to_string(g_name_counter++);
    const auto shader_unit_template = compile_shader_unit_template(shading_context.get(), name.c_str(), shader_source);

    if (!shader_unit_template)
        return std::make_pair(nullptr, nullptr);
    
    auto shader_instance = shader_unit_template->make_shader_instance();

    // resolve the shader before using it.
    if(Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed != shading_context->resolve_shader_instance(shader_instance.get()))
        return std::make_pair(nullptr, nullptr);

    return std::make_pair((T)shader_instance->get_function(), shader_instance);
}

template<class T, class TG>
inline std::pair<T, std::shared_ptr<ShaderInstance>> compile_shader(const char* shader_source) {
    auto shading_context = ShadingSystem::get_instance().make_shading_context();

    // this name is meanless, but I just want something unique
    const auto name = std::to_string(g_name_counter++);

    // make the shader
    const auto shader_unit_template = shading_context->begin_shader_unit_template(name);

    if(!shader_unit_template)
        return std::make_pair(nullptr, nullptr);

    // register tsl shader global
    TG tg;
    shader_unit_template->register_tsl_global(tg.m_var_list);

    // compile the shader
    const auto ret = shading_context->compile_shader_unit_template(shader_unit_template.get(), shader_source);

    // ending compiling
    shading_context->end_shader_unit_template(shader_unit_template.get());
    if( !ret )
        return std::make_pair(nullptr, nullptr);

    auto shader_instance = shader_unit_template->make_shader_instance();

    // resolve the shader before using it.
    if (Tsl_Namespace::TSL_Resolving_Status::TSL_Resolving_Succeed != shading_context->resolve_shader_instance(shader_instance.get()))
        return std::make_pair(nullptr, nullptr);

    return std::make_pair((T)shader_instance->get_function(), shader_instance);
}