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
#include <string>
#include <vector>
#include "tslversion.h"
#include "types.h"

TSL_NAMESPACE_BEGIN

class AstNode_Shader;
class AstNode_FunctionDefinition;

//! @brief  Internal compiler implementation.
/**
 * The sole purpose of another compiler implementation thing is to keep TslCompiler as simple as possible.
 * This class hides all details from the TslCompiler, which will eventually be exported to TSL users.
 */
class TslCompiler_Impl {
public:
    //! @brief  Default constructor
    TslCompiler_Impl();

    //! @brief  Nuke the state of the compiler so that it can be used for another pass of compiling.
    void    reset();

    //! @brief  Compile a shader.
    //!
    //! Ideally, this should be thread-safe. Flex and Bison support it, as long as I can make sure LLVM supports too,
    //! it should be thread-safe.
    //!
    //! @param  source_code     The source code of the shader module.
    //! @param  tso             Output object string.
    bool    compile(const char* source_code, std::string& tso);

    //! @brief  Get scanner of the compiler
    //!
    //! @return                 Get the internal scanner, which will be used by bison generated code.
    void*   get_scanner();

    //! @brief  Update a function definition.
    //!
    //! @param  node             Push a function node in the compiler.
    void    push_function(AstNode_FunctionDefinition* node, bool is_shader = false);

	//! @brief	Parameter type cache.
	//!
	//! @param	type			Type of the parameter to be parsed.
	void	cache_next_data_type(DataType type){
		m_type_cache = type;
	}

	//! @brief	Acquire the cached data type.
	DataType	data_type_cache() const {
		return m_type_cache;
	}

private:
    // flex scanner
    void* m_scanner = nullptr;

    // root ast node of the parsed program
    AstNode_Shader* m_ast_root = nullptr;

    // global functions defined in this module
    std::vector<AstNode_FunctionDefinition*>    m_functions;

	// data type cache
	DataType	m_type_cache = DataType::VOID;

    // local llvm context
    llvm::LLVMContext m_llvm_context;
};
TSL_NAMESPACE_END