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

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "tslversion.h"
#include "texture_impl.h"

TSL_NAMESPACE_BEGIN

using ShaderTextureTable = std::unordered_map<std::string, std::unique_ptr<TextureHandleWrapper>>;

struct ShadingSystem_Impl {
    /**< Data structure holding all contexts. */
    std::unordered_set<std::unique_ptr<ShadingContext>>    m_contexts;
    /**< Making sure context related operation is thread-safe. */
    std::mutex                                             m_context_mutex;

    /**< a container holding all shader unit. */
    std::unordered_map<std::string, std::unique_ptr<ShaderUnitTemplate>> m_shader_units;
    /**< a mutex to make sure shader_group access is thread-safe. */
    std::mutex m_shader_unit_mutex;

    /**< Closure register */
    GlobalModule* m_global_module = nullptr;
};

struct ShaderUnitTemplate_Impl {
    /**< Name of the shader unit. */
    std::string m_name;

    /**< A private data structure hiding all LLVM details. */
    ShaderUnitTemplate_Pvt* m_shader_unit_data = nullptr;

    /**< Shader texture table. */
    ShaderTextureTable      m_shader_texture_table;

    // This will be allowed once I have most feature completed.
    const bool  m_allow_optimization = false;
    const bool  m_allow_verification = false;

    /**< Description of exposed arguments. */
    std::vector<ArgDescriptor>  m_exposed_args;
};

//! @brief  A thin wrapper to allow a shader unit added in a group more than once.
/**
    * In order to allow a shader unit to be added in a shader group multiple times, there needs to be a thin wrapper
    * to differentiate different instances of shader unit.
    */
struct ShaderUnitTemplateCopy {
    std::string         m_name;
    ShaderUnitTemplate* m_shader_unit_template = nullptr;
};

struct ShaderGroupTemplate_Impl {
    /**< Shader units belong to this group. */
    std::unordered_map<std::string, ShaderUnitTemplateCopy> m_shader_units;

    /**< Name of the root shader unit. */
    std::string  m_root_shader_unit_name;

    /**< Shader unit connection. */
    using ShaderUnitConnection = std::unordered_map<std::string, std::unordered_map<std::string, std::pair<std::string, std::string>>>;
    ShaderUnitConnection m_shader_unit_connections;

    /**< Wrapper parameter connection. */
    using ShaderWrapperConnection = std::unordered_map<std::string, std::unordered_map<std::string, int>>;
    ShaderWrapperConnection     m_input_args;
    ShaderWrapperConnection     m_output_args;

    /**< Shader default value. */
    using ShaderUnitInputDefaultMapping = std::unordered_map<std::string, std::unordered_map<std::string, ShaderUnitInputDefaultValue>>;
    ShaderUnitInputDefaultMapping   m_shader_input_defaults;
};

struct ShadingContext_Impl {
    /**< TSL compiler. */
    std::unique_ptr<TslCompiler> m_compiler;

    /**< Shading system owning the context. */
    ShadingSystem_Impl* m_shading_system_impl = nullptr;
};

TSL_NAMESPACE_END