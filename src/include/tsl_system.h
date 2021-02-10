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
#include "tsl_args.h"
#include "tsl_define.h"

TSL_NAMESPACE_BEGIN

class TslCompiler;
class ShaderUnitTemplate;
class ShadingContext;

struct ShadingSystem_Impl;
struct ShadingContext_Impl;
struct ShaderUnitTemplate_Impl;
struct ShaderGroupTemplate_Impl;
struct ShaderInstance_Impl;

//! @brief  Debug information levels.
enum class TSL_DEBUG_LEVEL : unsigned int {
    TSL_DEBUG_INFO,         // General debugging information.
    TSL_DEBUG_WARNING,      // A warning means there is some badly written code in shader sources.
    TSL_DEBUG_ERROR,        // An error will most likely result failure in shader compilation.
};

//! @brief  Error message for resolving shader templates
enum class TSL_Resolving_Status : unsigned int {
    TSL_Resolving_Succeed = 0,
    TSL_Resolving_InvalidInput,                     /*< Input is nullptr. */
    Tsl_Resolving_InvalidDataType,                  /*< One of the data types is invalid.*/
    TSL_Resolving_ShaderGroupWithoutRoot,           /*< No shader unit defined as root in the input shader group. */
    TSL_Resolving_ShaderGroupWithCycles,            /*< There is cycles detected in the shader group. */
    TSL_Resolving_InvalidShaderGroupTemplate,       /*< Invalid shader group template. */
    TSL_Resolving_LLVMFunctionVerificationFailed,   /*< LLVM verfication of the */
    TSL_Resolving_UndefinedShaderUnit,              /*< A specific shader unit is not defined in the shader group. */
    TSL_Resolving_InvalidArgType,                   /*< One of the input arguments defined in the shader group template is not defined. */
    TSL_Resolving_ArgumentWithoutInitialization,    /*< One of the arguments passed in doesn't have a valid initialization value. */
    TSL_Resolving_InconsistentTSLGlobalType,        /*< Shader units defined in the shader group has multile version of TSL Global. */
    TSL_Resolving_InvalidExposedParameter,          /*< Shader group can't have invalid exposed parameter. */
    TSL_Resolving_UnspecifiedError                  /*< This error type is not specified somehow. */
};

//! @brief  ShadingSystem callback interface.
/**
 * ShadingSystemInterface offers a chance for renderers to do things like, outputing errors or logs, allocating memory for bxdf.
 * All methods in this interface need to be implemented in a thread-safe manner, it is renderer's job to make sure of it.
 * TSL won't synchronize upon calling these calls.
 */
class TSL_INTERFACE ShadingSystemInterface {
public:
    //! @brief  Virtual destructor.
    virtual ~ShadingSystemInterface() = default;

    //! @brief  Allocate memory inside shaders.
    //!
    //! There are things to be noticed in this interface.
    //!  - Shaders are not responsible to release the memory allocator allocates, it is up to the renderer to do so.
    //!  - This implementation has to be thread safe.
    virtual void*   allocate(unsigned int size, void* tsl_global) const = 0;

    //! @brief  This will be automatically called when there is error during shader compilation.
    //!
    //! @param  level       Debug level.
    //! @param  error       String describing the error.
    virtual void    catch_debug(const TSL_DEBUG_LEVEL level, const char* error) const = 0;

    //! @brief  Sample a 2d texture.
    //!
    //! @param  texture     Texture handle.
    //! @param  u           UV coordinate.
    //! @param  v           UV coordinate.
    //! @param  color       RGB of the texture pixel.
    virtual void    sample_2d(const void* texture, float u, float v, float3& color) const = 0;

    //! @brief  Sample alpha channel in a 2d texture.
    //!
    //! Having two separate interfaces for 2d sampling for RGB and alpha may not sound wise from a performance perspective.
    //! However, in order to get the ball rolling as soon as possible, I'll live with it now.
    //! To support float4 is also an alternative to be considered in the future.
    //!
    //! @param  texture     Texture handle.
    //! @param  u           UV coordinate.
    //! @param  v           UV coordinate.
    //! @param  alpha       Alpha channel of the texture.
    virtual void    sample_alpha_2d(const void* texture, float u, float v, float& alpha) const = 0;
};

//! @brief  Shading system is the root interface exposed through TSL system.
/*
 * A shading_system owns the whole TSL compiling system. A ray tracer with TSL integrated should have only one
 * instance of this class. Its interfaces are not thread-safe, use them in one single thread at a time.
 * It owns all memory allocated of the system, it will also deallocate all memory allocated so there is no need
 * manually maintain memory allocated through this interface.
 */
class TSL_INTERFACE ShadingSystem {
public:
    //! @brief  Destructor.
    ~ShadingSystem();

    //! @brief  Make the only instance of shading system in TSL system.
    //!
    //! This has to be called before anything in TSL called. The interface to be registered is very important to shader
    //! compilation. ShadingSystem will take over the ownership of the pointer passed in. Renderers don't need to 
    //! deallocate the memory of the passed in parameter, it will also need to avoid access of this parameter in renderer 
    //! later.
    //!
    //! @param  ssi         The interface to be registered.
    static void                             register_shadingsystem_interface(std::unique_ptr<ShadingSystemInterface> ssi);

    //! @brief  Get shading system instance.
    //!
    //! In order to make sure there is not a second instance of shading system in renderers, this class is a class of singleton.
    //! There is no way to have a second instance of it, which is secured duing compilation time.
    //!
    //! @return             Reference to the only instance of the class.
    static ShadingSystem&                   get_instance();

    //! @brief  Create a new shading context.
    //!
    //! TSL shading system won't take responsibility of keeping shading context alive. It is up to renderers to make sure it is alive
    //! when it is still needed. However, shading context life time will also be observed by things like shader unit template and
    //! shader instance, meaning as long as there is a shader instance or shader unit template alive, the context which creates them
    //! will also be alive.
    //!
    //! @return             A smart pointer to the newly created shading context returned by TSL system.
    std::shared_ptr<class ShadingContext>   make_shading_context();

    //! @brief  Register closure id.
    //!
    //! @param  name            Name of the closure. This has to match the one used in TSL shaders.
    //! @param  mapping         Mapping of the data inside the closure.
    //! @param  closure_size    Size of the data structure.
    //! @return                 Allocated closure id for the closure.
    ClosureID                               register_closure_type(const std::string& name, ClosureArgList& mapping, int closure_size);

    TSL_HIDE_CONSTRUCTOR(ShadingSystem)
};

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
    //! @brief  Resolve the shader instance.
    //!
    //! This needs to be called before the below interface is called.
    //!
    //! @return     Whether resolving is successful or not.
    TSL_Resolving_Status    resolve_shader_instance();

    //! @brief  Get the function pointer to execute the shader.
    //!
    //! It is up to renderers to inteprate the returned pointer. It has to match what the shader exposes.
    //! Failing to match the signature will result in unknown error, likely crash.
    //! Since the function pointer is already catched, this function has very minimal cost in term of performance.
    //!
    //! @return     A function pointer points to code memory.
    uint64_t                get_function() const;

private:
    /**< Private data inside shader instance. */
    std::shared_ptr<ShaderInstance_Impl> m_shader_instance_data;

    TSL_MAKE_CLASS_FRIEND(ShaderUnitTemplate)
    TSL_MAKE_CLASS_FRIEND(TslCompiler)
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
    //! @brief          Get name of the shader unit.
    //!
    //! @return         Name of the shader unit template.
    const std::string& get_name() const;

    //! @brief  Make a shader instance
    //!
    //! @return         Make a new shader instance.
    std::shared_ptr<ShaderInstance>     make_shader_instance();

    //! @brief  Register tsl global.
    //!
    //! @param  tslg    Tsl global memory layout.
    //! @return         Whether the tsl global is registered successfully.
    bool                                register_tsl_global(const GlobalVarList& tslg);

    //! @brief  Register resource handle in this shader unit.
    //!
    //! Resource could be anything that will be used in shaders. Commonly, this is used to implement
    //! texture and measured brdf.
    //!
    //! @param  name    Name of the shader resource handle defined in shader.
    //! @param  srh     The shader resource handle to be registered.
    bool                                register_shader_resource(const std::string& name, const ShaderResourceHandle* srh);

    //! @brief  Compile the shader unit given a piece of source code.
    //!
    //! If the shader unit template is compiled with other source code, it will fail.
    //!
    //! @param  source  The source code of the shader unit template.
    //! @return         Whether the source code is compiled successfully.
    bool                                compile_shader_source(const char* source);

    //! @brief  Enable llvm verification.
    //!
    //! By default it is disabled for faster compilation, it is not suggest to enable it unless there is shader related
    //! rendering problem. However, even if there is rendering artifacts, enabling it won't guarantee finding the problem.
    //!
    //! @param  enabled         Whether to enable llvm verification, by default it is skipped for faster compilation.
    void                                set_llvm_vericiation_enabled(bool enabled);

protected:
    /**< Private data inside shader unit template. */
    std::shared_ptr<ShaderUnitTemplate_Impl> m_shader_unit_template_impl;

    TSL_MAKE_CLASS_FRIEND(ShaderInstance)
    TSL_MAKE_CLASS_FRIEND(ShaderGroupTemplate)
    TSL_MAKE_CLASS_FRIEND(ShadingContext)
    TSL_MAKE_CLASS_FRIEND(TslCompiler)
    TSL_MAKE_STRUCT_FRIEND(ShaderGroupTemplate_Impl)
    TSL_HIDE_CONSTRUCTOR(ShaderUnitTemplate, const std::string& name, std::shared_ptr<ShadingContext> context)
};

//! @brief  Shader group is a basic unit of shader execution.
/**
 * A shader group is composed of multiple shader units with all of them connected with each other.
 * A shader group itself is also a shader unit, which is a quite useful feature to get recursive
 * node supported in material editors.
 */
class TSL_INTERFACE ShaderGroupTemplate : public ShaderUnitTemplate {
public:
    //! @brief  Add a shader unit in the group.
    //!
    //! The original name of the shader template means little in shader group since a same shader unit template could be 'instanced'
    //! multiple times in a shader group template. The name passed in is used to differentiate them.
    //!
    //! @param  name            Name of the shader unit added in the group.
    //! @param  shader_unit     A shader unit to be added in the group.
    //! @param  is_root         Whether the shader unit is the root of the group, there has to be exactly one root in each shader group.
    //! @return                 Whether the shader unit is added in the group.
    bool add_shader_unit(const std::string& name, std::shared_ptr<ShaderUnitTemplate> shader_unit, const bool is_root = false);

    //! @brief  Connect shader unit in the shader group.
    //!
    //! The name used here for both source and target nodes are the ones passed in through add_shader_unit, not the shader unit
    //! template node.
    //! Note, this function has very minimal cost since it only caches the connection instead of connecting them for real.
    //! This also means it is not mandatory to keep any specific order of connection shader unit templates and adding shader unit templates.
    //!
    //! @param  ssu     source shader unit
    //! @param  sspn    source shader parameter name
    //! @param  tsu     target shader unit
    //! @param  tspn    target shader parameter name
    void connect_shader_units(const std::string& ssu, const std::string& sspn, const std::string& tsu, const std::string& tspn);

    //! @brief  Setup shader group output
    //!
    //! It is up to renderers to make sure exposed arguments don't have duplicated names.
    //!
    //! @param  su          source shader unit
    //! @param  spn         source shader parameter name
    //! @param  is_ouput    whether the exposed parameter is an output.
    //! @param  name        name of the exposed parameter, if it is empty, spn will be used as exposed parameter name.
    void expose_shader_argument(const std::string& su, const std::string& spn, const bool is_output = true, const std::string& name = "");

    //! @brief  Setup default shader argument init value
    //!
    //! @param  su      source shader unit
    //! @param  spn     source shader parameter name
    void init_shader_input(const std::string& su, const std::string& spn, const ShaderUnitInputDefaultValue& val);

    TSL_MAKE_CLASS_FRIEND(ShadingContext)
    TSL_MAKE_CLASS_FRIEND(TslCompiler)
    TSL_HIDE_CONSTRUCTOR(ShaderGroupTemplate, const std::string& name, std::shared_ptr<ShadingContext> context)
};

//! @brief  Shading context should be a per-thread resource that is for making shader templates.
/**
 * Though, ShadingContext's interface is not thread-safe, just like ShadingSystem. The difference here is that
 * it is allowed to have multiple instances of shading context. It is up to renderers to make sure a single 
 * shading context doesn't get accessed into different threads simultaneously. An easy way to guarantee this is
 * to cache one shading context for each thread and each thread only uses its own shading context.
 * Shading context is responsible for maintaining lots of internal useful data structures under the hood, though
 * the only exposed interfaces are only for making shader template.
 * Also it is important to make sure shading context is alive during the life time of the shader it allocates,
 * renderers do not need to explicitly do it since there is a reference of its shader keeping the context alive,
 * meaning there is no way to destroy the shading context illegally with shaders left alive.
 */
class TSL_INTERFACE ShadingContext : public std::enable_shared_from_this<ShadingContext> {
public:
    //! @brief  Make a new shader group.
    //!
    //! ShadingContext won't keep maintaining the lifetime the returned ShaderGroupTemplate.
    //! It is renderers' job to keep them alive.
    //!
    //! @param  name    Name of the shader group.
    //! @return         A smart shared pointer to the newly allocated shader group.
    std::shared_ptr<ShaderGroupTemplate>    begin_shader_group_template(const std::string& name);

    //! @brief  Resolve a shader group template before using it.
    //!
    //! @param sg       The shader group to be resolved.
    //! @return         Whether the shader is resolved successfully.
    TSL_Resolving_Status                    end_shader_group_template(ShaderGroupTemplate* sg) const;

    //! @brief  Make a new shader unit template.
    //!
    //! It is up to the renderer to keep this template alive during its usage.
    //! Though shader unit template is needed during shader compilation or groupping,
    //! once it creates an instance, the instance will also keep its owner template alive by
    //! holding a shared pointer.
    //!
    //! @param  name    Name of the shader unit template.
    //! @return         Shader unit template to be returned.
    std::shared_ptr<ShaderUnitTemplate>     begin_shader_unit_template(const std::string& name);

    //! @breif  Ending of making a shader unit template.
    //!
    //! @param  sut     The shader unit template to close.
    TSL_Resolving_Status                    end_shader_unit_template(ShaderUnitTemplate* su) const;

private:
    /**< Shading context implementation. */
    std::shared_ptr<ShadingContext_Impl> m_shading_context_impl;

    TSL_MAKE_CLASS_FRIEND(ShadingSystem)
    TSL_MAKE_CLASS_FRIEND(ShaderUnitTemplate)
    TSL_MAKE_CLASS_FRIEND(ShaderInstance)
    TSL_HIDE_CONSTRUCTOR(ShadingContext, std::shared_ptr<ShadingSystem_Impl> shading_system_impl)
};

TSL_NAMESPACE_END