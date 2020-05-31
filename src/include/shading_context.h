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
#include "shader_arg_types.h"

TSL_NAMESPACE_BEGIN

class ShadingSystem;
class TslCompiler;
class ShaderUnitTemplate;
struct ShaderUnitTemplate_Pvt;
struct ShaderInstance_Pvt;

//! @brief  ShaderInstance is the very basic unit of shader execution.
/**
 * A shader instance keeps track of the raw function pointer for shader execution.
 * Shader instances made from a same thread can't be resolved in multiple threads simultaneously.
 * But shader instance can be executed by multiple threads simultaneously once constructed and 
 * resolved.
 */
class ShaderInstance {
public:
    //! @brief  Get the function pointer to execute the shader.
    //!
    //! @return     A function pointer points to code memory.
    uint64_t        get_function() const;

    //! @brief  Destructor
    ~ShaderInstance();

    //! @brief  Get private shader instance data.
    //!
    //! @return     It returns a pointer to shader instance private data.
    ShaderInstance_Pvt* get_shader_instance_data() {
        return m_shader_instance_data;
    }

    //! @brief  Get shader unit template.
    //!
    //! @return     The shader template.
    const ShaderUnitTemplate& get_shader_template() const {
        return m_shader_unit_template;
    }

private:
    //! @brief  Private constructor to limit the construction of shader instance through ShaderUnitTemplate.
    //!
    //! @param  sut     Shader unit template that is used to construct this shader instance.
    ShaderInstance(const ShaderUnitTemplate& sut);

    /**< Shader unit template that creates this shader instance. */
    const ShaderUnitTemplate& m_shader_unit_template;

    /**< Private data inside shader instance. */
    ShaderInstance_Pvt* m_shader_instance_data = nullptr;

    // making sure ShaderUnitTemplate can access private method of this class so that it can create an instance
    // of it.
    friend class ShaderUnitTemplate;
};

class ShaderUnitTemplate {
public:
    //! @brief  Constructor.
    //!
    //! @param  name            Name of the shader unit.
    ShaderUnitTemplate(const std::string& name);

    //! @brief  Destructor.
    virtual ~ShaderUnitTemplate();

    //! @brief  Get name of the shader unit.
    const std::string& get_name() const {
        return m_name;
    }

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

    //! @brief  Make a shader instance
    ShaderInstance*     make_shader_instance();

    //! @brief  Parse shader dependencies.
    //!
    //! @param sut      Dependencies of this module.
    virtual void        parse_dependencies(ShaderUnitTemplate_Pvt* sut) const;

protected:
    /**< Name of the shader unit. */
    const std::string m_name;

    /**< A private data structure hiding all LLVM details. */
    ShaderUnitTemplate_Pvt* m_shader_unit_data = nullptr;

    // This will be allowed once I have most feature completed.
    const bool  m_allow_optimization = false;
    const bool  m_allow_verification = false;

    /**< Description of exposed arguments. */
    std::vector<ArgDescriptor>  m_exposed_args;

    /**< Make sure all shader instances are alive. */
    std::vector<std::unique_ptr<ShaderInstance>> m_shader_instnaces;

    // make sure shader instance can access private data of shader_unit_template
    friend class ShaderInstance;
    friend class TslCompiler_Impl;
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

//! @brief  Shader group is a basic unit of shader execution.
/**
 * A shader group is composed of multiple shader units with all of them connected with each other.
 * A shader group itself is also a shader unit, which is a quite useful feature to get recursive
 * node supported in certain material editors.
 */
class ShaderGroupTemplate : public ShaderUnitTemplate{
public:
    //! @brief  Constructor.
    //!
    //! @param  name            The name fo the shader group.
    ShaderGroupTemplate(const std::string& name);

    //! @brief  Add a shader unit in the group.
    //!
    //! @param  name            Name of the shader unit added in the group.
    //! @param  shader_unit     A shader unit to be added in the group.
    //! @param  is_root         Whether the shader unit is the root of the group, there has to be exactly one root in each shader group.
    //! @return                 Whether the shader unit is added in the group.
    bool add_shader_unit(const std::string& name, ShaderUnitTemplate* shader_unit, const bool is_root = false);

    //! @brief  Connect shader unit in the shader group.
    //!
    //! @param  ssu     source shader unit
    //! @param  sspn    source shader parameter name
    //! @param  tsu     target shader unit
    //! @param  tspn    target shader parameter name
    void connect_shader_units(const std::string& ssu, const std::string& sspn, const std::string& tsu, const std::string& tspn);

    //! @brief  Setup shader group output
    //!
    //! @param  su      source shader unit
    //! @param  spn     source shader parameter name
    void expose_shader_argument(const std::string& su, const std::string& spn, const ArgDescriptor& arg_desc );

    //! @brief  Setup default shader argument init value
    //!
    //! @param  su      source shader unit
    //! @param  spn     source shader parameter name
    void init_shader_input(const std::string& su, const std::string& spn, const ShaderUnitInputDefaultValue& val);

    //! @brief  Parse shader group dependencies.
    //!
    //! @param sut      Dependencies of this module.
    void parse_dependencies(ShaderUnitTemplate_Pvt* sut) const override;

private:
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

    friend class TslCompiler_Impl;
};

//! @brief  Shading context should be a per-thread resource that is for shader related stuff.
/**
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
    ShaderGroupTemplate* begin_shader_group_template(const std::string& name);

    //! @brief  Resolve a shader group template before using it.
    //!
    //! @param sg       The shader group to be resolved.
    //! @return         Whether the shader is resolved successfully.
    bool                 end_shader_group_template(ShaderGroupTemplate* sg) const;

    //! @brief  Compile shader unit with source code.
    //!
    //! The function will return nullptr if for any reason the shader unit is failed to created,
    //! like invalid shader code or the name is already existed.
    //!
    //! @param name     Name of the shader unit
    //! @param source   Source code of the shader.
    //! @return         A pointer to shader unit.
    ShaderUnitTemplate*  compile_shader_unit_template(const std::string& name, const char* source) const;

    //! @brief  Resolve a shader instance before using it.
    //!
    //! @param si       The shader instance to be resolved.
    //! @return         Whether the shader is resolved successfully.
    bool                 resolve_shader_instance(ShaderInstance* si) const;

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
