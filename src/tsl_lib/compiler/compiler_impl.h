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

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "tsl_system.h"
#include "types.h"
#include "ast.h"

TSL_NAMESPACE_BEGIN

class ShaderUnitTemplate;
class ShaderGroupTemplate;
class ShaderInstance;
class GlobalModule;
struct LLVM_Compile_Context;
struct ShaderUnitTemplateCopy;

//! @brief  Internal compiler implementation.
/**
 * The sole purpose of another compiler implementation thing is to keep TslCompiler as simple as possible.
 * This class hides all details from the TslCompiler, which will eventually be exported to TSL users.
 */
class TslCompiler_Impl {
public:
    //! @brief  Default constructor
    TslCompiler_Impl( GlobalModule& global_module );

    //! @brief  Destructor
    ~TslCompiler_Impl();

    //! @brief  Nuke the state of the compiler so that it can be used for another pass of compiling.
    void    reset( const std::string& name = "" );

    //! @brief  Compile a shader.
    //!
    //! Ideally, this should be thread-safe. Flex and Bison support it, as long as I can make sure LLVM supports too,
    //! it should be thread-safe.
    //!
    //! @param  source_code     The source code of the shader module.
    //! @param  su              The shader unit owning this piece of source code.
    bool    compile(const char* source_code, ShaderUnitTemplate* su);

    //! @brief  Resolve a shader group.
    //!
    //! @param  sg              The shader group to be resolved.
    //! @return                 Whether the shader is resolved succesfully.
    TSL_Resolving_Status    resolve(ShaderGroupTemplate* su);

    //! @brief  Resolve a shader instance.
    //!
    //! @param  si              The shader instance to be resolved.
    //! @return                 Whether the shader is resolved successfully.
    TSL_Resolving_Status    resolve(ShaderInstance* si);

    //! @brief  Get scanner of the compiler
    //!
    //! @return                 Get the internal scanner, which will be used by bison generated code.
    void*   get_scanner();

    //! @brief  Update a function definition.
    //!
    //! @param  node             Push a function node in the compiler.
    void    push_function(AstNode_FunctionPrototype* node, bool is_shader = false);

	//! @brief	Push structure declaration.
	//!
	//! @param	node			Push a structure declaration.
	void	push_structure_declaration(AstNode_StructDeclaration* structure);

    //! @brief  Push global parameter
    //!
    //! @param  statement       This statement should be purely variable declaration.
    void    push_global_parameter(const AstNode_Statement* statement);

	//! @brief	Parameter type cache.
	//!
	//! @param	type			Type of the parameter to be parsed.
	void	cache_next_data_type(const DataType& type){
		m_type_cache = type;
	}

	//! @brief	Acquire the cached data type.
	DataType	data_type_cache() const {
		return m_type_cache;
	}

    //! @brief  Ask the compiler to pre-declare make closure function
    void        closure_touched(const std::string& name) {
        m_closures_in_shader.insert(name);
    }

	//! claim string, this will return a permanent address for each string, each unique string get their own ( one single ) address too.
	const char*	claim_permanent_address(const char* str){
		auto it = m_string_container.find(str);
		if( it != m_string_container.end() )
			return it->c_str();

		m_string_container.insert( str );
		
		return m_string_container.find(str)->c_str();
	}

    //! @brief  Name replacement of shader unit root function.
    const std::string& get_shader_root_function_name() const {
        return m_shader_root_function_name;
    }

private:
    // flex scanner
    void* m_scanner = nullptr;

    // root ast node of the parsed program
    std::shared_ptr<const AstNode_FunctionPrototype>                m_ast_root;

    // the shader unit/group template name being compiled.
    std::string                                                     m_shader_root_function_name;

    // global functions defined in this module
    std::vector<std::shared_ptr<const AstNode_FunctionPrototype>>   m_functions;
	// global structure declaration in this module, maybe I should merge it with the above one
	std::vector<std::shared_ptr<const AstNode_StructDeclaration>>	m_structures;
    // global variables defined in this module
    std::vector<std::shared_ptr<const AstNode_Statement>>           m_global_var;

	// data type cache
	DataType	m_type_cache = { DataTypeEnum::VOID , nullptr };

    // local llvm context
    llvm::LLVMContext   m_llvm_context;

    // closure register
    GlobalModule&    m_global_module;

    // closured touched in the shader
    std::unordered_set<std::string> m_closures_in_shader;

	// a string holder, this is purely to workaround bison limitation because DateType can't be non-POD.
	// an extra perk of doing this is to make DataType much cheaper.
	std::unordered_set<std::string>	m_string_container;

    // this data structure keeps track of used values to bridge shader units
    using VarMapping = std::unordered_map<std::string, std::unordered_map<std::string, llvm::Value*>>;

    //! @brief  Generate shader group source code
    TSL_Resolving_Status    generate_shader_source( LLVM_Compile_Context& context, ShaderGroupTemplate* sg, const ShaderUnitTemplateCopy& su, std::unordered_set<std::string>& visited,
                                    std::unordered_set<std::string>& being_visited, VarMapping& var_mapping,
                                    const std::unordered_map<std::string, llvm::Function*>& function_mapping, const std::vector<llvm::Value*>& args);

    //! @brief  A helper wrapper to make sure all contexts are gone after shader compilation
    class ContextWrapper {
    public:
        ContextWrapper(TslCompiler_Impl& impl, const std::string& name):impl(impl){ impl.reset(name); }
        ~ContextWrapper() { impl.reset(); }
    private:
        TslCompiler_Impl& impl;
    };
};

TSL_NAMESPACE_END