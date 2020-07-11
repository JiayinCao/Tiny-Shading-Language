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
#include <string>
#include "tslversion.h"
#include "common.h"
#include "status.h"
#include "closure.h"
#include "shader_arg_types.h"
#include "export.h"

TSL_NAMESPACE_BEGIN

class ShadingSystem;
class TslCompiler;
class ShaderUnitTemplate;
class ShadingContext;

struct ShadingSystem_Impl;
struct ShaderUnitTemplate_Pvt;
struct ShaderInstance_Pvt;
struct ShaderUnitTemplate_Impl;
struct ShaderGroupTemplate_Impl;
struct ShadingContext_Impl;

#if defined(TSL_ON_WINDOWS)
    // WARNING
    // This might introduce some sort of connection between the compiler used to compile TSL and the renderer.
    // However, this is always a trade off between larger audiance and safer code. Since TSL is mainly designed for my
    // own renderer SORT, I would sacrifice some inpendency of the compilers to be used for safer code.
    template class TSL_INTERFACE std::weak_ptr<ShaderUnitTemplate>;
    template class TSL_INTERFACE std::enable_shared_from_this<ShaderUnitTemplate>;
    template class TSL_INTERFACE std::weak_ptr<ShadingContext>;
    template class TSL_INTERFACE std::enable_shared_from_this<ShadingContext>;
#endif

//! @brief  Shader resource handle interface.
class TSL_INTERFACE ShaderResourceHandle {
public:
    //! @brief  Virtual destructor.
    ~ShaderResourceHandle() = default;
};

//! @brief  ShaderInstance is the very basic unit of shader execution.
/**
 * A shader instance keeps track of the raw function pointer for shader execution.
 * Shader instances made from a same thread can't be resolved in multiple threads simultaneously.
 * But shader instance can be executed by multiple threads simultaneously once constructed and 
 * resolved.
 */
class TSL_INTERFACE ShaderInstance {
public:
    //! @brief  Destructor
    ~ShaderInstance();

    //! @brief  Get the function pointer to execute the shader.
    //!
    //! It is up to renderers to inteprate the returned pointer. It has to match what the shader exposes.
    //! Failing to match the signature will result in unknown error, likely crash.
    //! Since the function pointer is already catched, this function has very minimal cost in term of performance.
    //!
    //! @return     A function pointer points to code memory.
    uint64_t        get_function() const;

private:
    /**< Private data inside shader instance. */
    ShaderInstance_Pvt* m_shader_instance_data = nullptr;

    TSL_MAKE_FRIEND(ShaderUnitTemplate)
    TSL_MAKE_FRIEND(TslCompiler_Impl)
    TSL_HIDE_CONSTRUCTOR(ShaderInstance, std::shared_ptr<ShaderUnitTemplate> sut)
};

//! @brief  ShaderUnitTemplate defines the shader of a single shader unit.
/**
 * A shader unit template defines the basic behavior of a shader unit.
 * Multiple shader units can be groupped into a shader group template.
 * A shader unit template can't be executed, it needs to instance a shader instance for shader execution.
 */
class TSL_INTERFACE ShaderUnitTemplate : public std::enable_shared_from_this<ShaderUnitTemplate> {
public:
    //! @brief  Destructor.
    virtual ~ShaderUnitTemplate();

    //! @brief          Get name of the shader unit.
    //!
    //! @return         Name of the shader unit template.
    const std::string&  get_name() const;

    //! @brief  Make a shader instance
    //!
    //! @return         Make a new shader instance.
    std::shared_ptr<ShaderInstance>     make_shader_instance();

    //! @brief  Register resource handle in this shader unit.
    //!
    //! @param  name    Name of the shader resource handle defined in shader.
    //! @param  srh     The shader resource handle to be registered.
    bool                register_shader_resource(const std::string& name, const ShaderResourceHandle* srh);

protected:
    ShaderUnitTemplate_Impl* m_shader_unit_template_impl = nullptr;

    //! @brief  Parse shader dependencies.
    //!
    //! @param sut      Dependencies of this module.
    virtual void        parse_dependencies(ShaderUnitTemplate_Pvt* sut) const;

    TSL_MAKE_FRIEND(ShaderInstance)
    TSL_MAKE_FRIEND(ShaderGroupTemplate)
    TSL_MAKE_FRIEND(ShadingContext)
    TSL_MAKE_FRIEND(TslCompiler_Impl)
    TSL_HIDE_CONSTRUCTOR(ShaderUnitTemplate, const std::string& name, std::shared_ptr<ShadingContext> context)
};

//! @brief  Shader group is a basic unit of shader execution.
/**
 * A shader group is composed of multiple shader units with all of them connected with each other.
 * A shader group itself is also a shader unit, which is a quite useful feature to get recursive
 * node supported in certain material editors.
 */
class TSL_INTERFACE ShaderGroupTemplate : public ShaderUnitTemplate{
public:
    //! @brief  Destructor.
    ~ShaderGroupTemplate();

    //! @brief  Add a shader unit in the group.
    //!
    //! @param  name            Name of the shader unit added in the group.
    //! @param  shader_unit     A shader unit to be added in the group.
    //! @param  is_root         Whether the shader unit is the root of the group, there has to be exactly one root in each shader group.
    //! @return                 Whether the shader unit is added in the group.
    bool add_shader_unit(const std::string& name, std::shared_ptr<ShaderUnitTemplate> shader_unit, const bool is_root = false);

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

private:
    /**< Shader group template implementation. */
    ShaderGroupTemplate_Impl* m_shader_group_template_impl = nullptr;

    //! @brief  Parse shader group dependencies.
    //!
    //! @param sut      Dependencies of this module.
    void parse_dependencies(ShaderUnitTemplate_Pvt* sut) const override;

    TSL_MAKE_FRIEND(ShadingContext)
    TSL_MAKE_FRIEND(TslCompiler_Impl)
    TSL_HIDE_CONSTRUCTOR(ShaderGroupTemplate, const std::string& name, std::shared_ptr<ShadingContext> context)
};

//! @brief  Shading context should be a per-thread resource that is for shader related stuff.
/**
 * Unlike shading_system, shading_context is not designed to be thread-safe, meaning each thread
 * should have their own copy of a shading_context.
 * shading_context is used for shader related operations, like shader compilation, shader resolving.
 * Since shading_context is available in each thread, things like shader compilation and shader 
 * execution could be exectued in multi-threaded too.
 */
class TSL_INTERFACE ShadingContext : public std::enable_shared_from_this<ShadingContext> {
public:
    //! @brief  Destructor.
    ~ShadingContext();

    //! @brief  Make a new shader group.
    //!
    //! ShadingContext won't keep maintaining the lifetime the returned ShaderGroupTemplate.
    //! It is renderers' job to keep them alive.
    //!
    //! @param  name    Name of the shader group.
    //! @return         A pointer to the newly allocated shader group.
    std::shared_ptr<ShaderGroupTemplate> begin_shader_group_template(const std::string& name);

    //! @brief  Resolve a shader group template before using it.
    //!
    //! @param sg       The shader group to be resolved.
    //! @return         Whether the shader is resolved successfully.
    TSL_Resolving_Status end_shader_group_template(ShaderGroupTemplate* sg) const;

    //! @brief  Make a new shader unit template.
    //!
    //! It is up to the renderer to keep this template alive during its usage.
    //! Though shader unit template is needed during shader compilation or groupping,
    //! once it creates an instance, the instance will also keep its owner template alive by
    //! holding a shared pointer.
    //!
    //! @param  name    Name of the shader unit template.
    //! @return         Shader unit template to be returned.
    std::shared_ptr<ShaderUnitTemplate>  begin_shader_unit_template(const std::string& name);

    //! @breif  Ending of making a shader unit template.
    //!
    //! @param  sut     The shader unit template to close.
    TSL_Resolving_Status end_shader_unit_template(ShaderUnitTemplate* su) const;

    //! @brief  Compile shader unit with source code.
    //!
    //! The function will return nullptr if for any reason the shader unit is failed to created,
    //! like invalid shader code or the name is already existed.
    //!
    //! @param sut      Shader unit template.
    //! @param source   Source code of the shader.
    //! @return         A pointer to shader unit.
    bool  compile_shader_unit_template(ShaderUnitTemplate* sut, const char* source) const;

    //! @brief  Resolve a shader instance before using it.
    //!
    //! @param si       The shader instance to be resolved.
    //! @return         Whether the shader is resolved successfully.
    TSL_Resolving_Status      resolve_shader_instance(ShaderInstance* si) const;

private:
    /**< Shading context implementation. */
    ShadingContext_Impl* m_shading_context_impl = nullptr;

    TSL_MAKE_FRIEND(ShadingSystem)
    TSL_HIDE_CONSTRUCTOR(ShadingContext, ShadingSystem_Impl* shading_system_impl)
};

TSL_NAMESPACE_END
