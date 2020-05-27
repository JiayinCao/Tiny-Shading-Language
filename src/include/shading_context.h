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

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include "tslversion.h"
#include "closure.h"

TSL_NAMESPACE_BEGIN

class ShadingSystem;
class TslCompiler;
struct ShaderUnit_Pvt;

class ShaderUnit {
public:
    //! @brief  Constructor.
    //!
    //! @param  name    Name of the shader unit.
    ShaderUnit(const std::string& name);

    //! @brief  Destructor.
    virtual ~ShaderUnit();

    //! @brief  Get name of the shader unit.
    const std::string& get_name() const {
        return m_name;
    }

    //! @brief  Get the internal representation of this shader unit
    ShaderUnit_Pvt* get_shader_unit_data(){
        return m_shader_unit_data;
    }

    //! @brief  Resolve the shader unit.
    //!
    //! @return Whether the shader is resolved successfully.
    // virtual bool    resolve();

    //! @brief  Get the function pointer to execute the shader.
    //!
    //! @return     A function pointer points to code memory.
    uint64_t        get_function() const;

    //! @brief  Whether to allow optimization of LLVM generated code.
    //!
    //! @return     Whether optimization pass is on.
    const bool      allow_optimization() const {
        return m_allow_optimization;
    }

    //! @brief  Whether to allow verification of LLVM generated code.
    //!
    //! @return     Whether verification pass is on.
    const bool      allow_verification() const {
        return m_allow_verification;
    }

protected:
    const std::string m_name;

    /**< A private data structure hiding all LLVM details. */
    ShaderUnit_Pvt* m_shader_unit_data = nullptr;

    // This will be allowed once I have most feature completed.
    const bool  m_allow_optimization = false;
    const bool  m_allow_verification = false;
};

//! @brief  Shader group is a basic unit of shader execution.
/**
 * A shader group is composed of multiple shader units with all of them connected with each other.
 * A shader group itself is also a shader unit, which is a quite useful feature to get recursive
 * node supported in certain material editors.
 */
class ShaderGroup : public ShaderUnit{
public:
    //! @brief  Constructor.
    //!
    //! @param  name            The name fo the shader group.
    //! @param  compiler        A compiler belonging to the owning shading context.
    ShaderGroup(const std::string& name, const TslCompiler& compiler);

    //! @brief  Add a shader unit in the group.
    //!
    //! @param  shader_unit     A shader unit to be added in the group.
    //! @param  is_root         Whether the shader unit is the root of the group, there has to be exactly one root in each shader group.
    //! @return                 Whether the shader unit is added in the group.
    bool add_shader_unit(ShaderUnit* shader_unit, const bool is_root = false);

    //! @brief  Connect shader unit in the shader group.
    //!
    //! @param  ssu     source shader unit
    //! @param  sspn    source shader parameter name
    //! @param  tsu     target shader unit
    //! @param  tspn    target shader parameter name
    void connect_shader_units(const std::string& ssu, const std::string& sspn, const std::string& tsu, const std::string& tspn);

private:
    /**< TSL compiler of the owning context. */
    const TslCompiler&   m_compiler;

    /**< Shader units belong to this group. */
    std::unordered_map<std::string, ShaderUnit*> m_shader_units;

    /**< Name of the root shader unit. */
    std::string  m_root_shader_unit_name;

    /**< Shader unit connection. */
    using ShaderUnitConnection = std::unordered_map<std::string, std::unordered_map<std::string, std::pair<std::string, std::string>>>;
    ShaderUnitConnection m_shader_unit_connections;

    friend class TslCompiler_Impl;
};

//! @brief  Shading context should be a per-thread resource that is for shader related stuff.
/*
 * Unlike shading_system, shading_context is not designed to be thread-safe, meaning each thread
 * should have their own copy of a shading_system.
 * shading_context is used for shader related operations, like shader compilation, shader execution.
 * Since shading_context is available in each thread, things like shader compilation and shader 
 * execution could be exectued in multi-threaded too.
 */
class ShadingContext {
public:
    //! @brief  Destructor.
    ~ShadingContext();

    //! @brief  Make a new shader group.
    //!
    //! @param  name    Name of the shader group.
    //! @return         A pointer to the newly allocated shader group.
    ShaderGroup* make_shader_group(const std::string& name);

    //! @brief  Compile shader unit with source code.
    //!
    //! The function will return nullptr if for any reason the shader unit is failed to created,
    //! like invalid shader code or the name is already existed.
    //!
    //! @param name     Name of the shader unit
    //! @param source   Source code of the shader.
    //! @return         A pointer to shader unit.
    ShaderUnit*  compile_shader_unit(const std::string& name, const char* source) const;

    //! @brief  Resolve a shader unit before using it.
    //!
    //! @param su       The shader unit to be resolved.
    //! @return         Whether the shader is resolved successfully.
    bool         resolve_shader_unit(ShaderUnit* su) const;

private:
    //! @brief  Constructor.
    //!
    //! shading_context can only be allocated through shading_system. Making the constructor private
    //! will make sure users of TSL won't create instance of this class manually.
    ShadingContext(ShadingSystem& shading_system);

    /**< TSL compiler. */
    std::unique_ptr<TslCompiler> m_compiler;

    /**< Shading system owning the context. */
    ShadingSystem& m_shading_system;

    friend class ShadingSystem;
};

TSL_NAMESPACE_END
