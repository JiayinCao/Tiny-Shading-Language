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
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LegacyPassManager.h>
#include "tsl_system.h"
#include "compiler/types.h"

TSL_NAMESPACE_BEGIN

class GlobalModule;
class ShaderResourceHandle;
class AstNode_FunctionPrototype;
using ShaderResourceTable = std::unordered_map<std::string, const ShaderResourceHandle*>;
using ShaderUnitConnection = std::unordered_map<std::string, std::unordered_map<std::string, std::pair<std::string, std::string>>>;
using ShaderWrapperConnection = std::unordered_map<std::string, std::unordered_map<std::string, int>>;
using ShaderUnitInputDefaultMapping = std::unordered_map<std::string, std::unordered_map<std::string, ShaderUnitInputDefaultValue>>;

//! @brief  Exposed argument descriptor.
/**
 * Argument descriptor is used to describe the exposed arguments in a shader group template.
 * This data structure keeps track of argument name, type and output signature, meaning it is
 * both for input arguments and output arguments.
 * This is only used for shader group, shader unit exposes everything defined in the shader
 * source code.
 */
struct ExposedArgDescriptor {
    std::string             m_source_shader_unit_name;
    std::string             m_source_shader_unit_arg_name;
    std::string             m_name;
    DataType                m_type;
    bool                    m_is_output = false;
};

// This data structure hides all LLVM related data from ShaderInstance.
struct ShaderInstance_Impl {
public:
    ~ShaderInstance_Impl() {
        // explicit order of destruction is mandatory here to prevent crashing in LLVM code.
        m_execution_engine = nullptr;
        m_shader_unit_template = nullptr;
    }

    /**< Shader unit template that creates this shader instance. */
    std::shared_ptr<ShaderUnitTemplate> m_shader_unit_template;

    // the execute engine for this module, it is important to keep this execution engine alive to make sure
    // the raw function pointer is still valid.
    std::unique_ptr<llvm::ExecutionEngine> m_execution_engine;

    // the function address for host code to call
    uint64_t m_function_pointer = 0;
};

struct ShadingSystem_Impl {
    /**< Closure register */
    std::unique_ptr<GlobalModule> m_global_module;

    /**< This is needs to be bound before shader compilation. */
    std::unique_ptr<ShadingSystemInterface> m_callback;
};

struct ShaderUnitTemplate_Impl {
    /**< Name of the shader unit. */
    std::string m_name;

    /**< TSL global data. */
    GlobalVarList           m_tsl_global;
    unsigned                m_tsl_global_hash = 0;

    /**< root function name. */
    std::string             m_root_function_name;

    /**< root ast node. */
    std::shared_ptr<const AstNode_FunctionPrototype> m_ast_root;

    /**< The llvm context created in the shading_context is needed as long as this is alive. */
    std::shared_ptr<ShadingContext> m_shading_context;

    /**< Shader resource table. */
    ShaderResourceTable     m_shader_resource_table;

    /**< Description of exposed arguments. */
    std::vector<ExposedArgDescriptor>  m_exposed_args;

    // the llvm module owned by this shader unit
    std::unique_ptr<llvm::Module> m_module;

    // the dependent llvm module
    std::unordered_set<const llvm::Module*> m_dependencies;

    // llvm function pointer
    // this is just a copy to the llvm object, the memory ownership is maintained through m_module.
    llvm::Function* m_llvm_function = nullptr;

    //! @brief  Parse shader group dependencies.
    //!
    //! @param sut      Dependencies of this module.
    virtual void parse_dependencies(ShaderUnitTemplate_Impl* sut) const;

    //! @brief  Virtual destructor, this is needed to make sure deletes all members of derived class.
    virtual ~ShaderUnitTemplate_Impl() {}
};

//! @brief  A thin wrapper to allow a shader unit added in a group more than once.
/**
  * In order to allow a shader unit to be added in a shader group multiple times, there needs to be a thin wrapper
  * to differentiate different instances of shader unit.
  */
struct ShaderUnitTemplateCopy {
    std::string         m_name;
    std::shared_ptr<ShaderUnitTemplate> m_shader_unit_template;
};

struct ShaderGroupTemplate_Impl : public ShaderUnitTemplate_Impl {
    /**< Name of the root shader unit. */
    std::string  m_root_shader_unit_name;

    /**< Shader units belong to this group. */
    std::unordered_map<std::string, ShaderUnitTemplateCopy> m_shader_units;

    /**< Shader unit connection. */
    ShaderUnitConnection            m_shader_unit_connections;

    /**< Wrapper parameter connection. */
    ShaderWrapperConnection         m_input_args;
    ShaderWrapperConnection         m_output_args;

    /**< Shader default value. */
    ShaderUnitInputDefaultMapping   m_shader_input_defaults;

    //! @brief  Parse shader group dependencies.
    //!
    //! @param sut      Dependencies of this module.
    void parse_dependencies(ShaderUnitTemplate_Impl* sut) const override;
};

struct ShadingContext_Impl {
    /**< TSL compiler. */
    std::unique_ptr<TslCompiler> m_compiler;

    /**< Shading system owning the context. */
    std::shared_ptr<ShadingSystem_Impl> m_shading_system_impl = nullptr;
};

//! @brief  Allocate memory in shader.
void* allocate_memory(const unsigned size);

//! @brief  Output error during shader compilation.
void  emit_error(const char* error, ...);

//! @brief  Output warning during shader compilation.
void  emit_warning(const char* format, ...);

//! @brief  Texture sampling
void  sample_2d(const void* texture, float u, float v, float3& color);

//! @brief  Texture sampling alpha channel only.
void  sample_alpha_2d(const void* texture, float u, float v, float& alpha);

TSL_NAMESPACE_END