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
#include "llvm/IR/Module.h"
#include <string>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "tslversion.h"
#include "closure.h"
#include "ast.h"

TSL_NAMESPACE_BEGIN

struct LLVM_Compile_Context;

struct ClosureItem {
    const ClosureID         m_closure_id = INVALID_CLOSURE_ID;
    const ClosureVarList&   m_var_list;
    const int               m_structure_size;

    ClosureItem(ClosureID id, ClosureVarList& var_list, int structure_size) 
        : m_closure_id(id), m_var_list(var_list), m_structure_size(structure_size) {}
};

class GlobalModule {
public:
    // initialize the register
    bool init();

    // I prefer this way of registering a lot than the below one, need to follow this
    //template<class T>
    //ClosureID register_closure_type(const std::string& name);

    ClosureID register_closure_type(const std::string& name, ClosureVarList& mapping, int structure_size);

    // get global closure maker module
    llvm::Module* get_closure_module();

    // declare some global data structure type
    void          declare_closure_tree_types(llvm::LLVMContext& context, StructSymbolTable* mapping = nullptr );

    // get declaration
    llvm::Function* declare_closure_function(const std::string& name, LLVM_Compile_Context& context);
	// declare global function
	void			declare_global_module(LLVM_Compile_Context& context);

private:
    /**< a container holding all closures ids. */
    std::unordered_map<std::string, ClosureItem>  m_closures;
    /**< current allocated closure id. */
    int m_current_closure_id = INVALID_CLOSURE_ID + 1;
    /**< a mutex to make sure access to closure container is thread-safe. */
    std::mutex m_closure_mutex;

    llvm::LLVMContext                               m_llvm_context;
    std::unique_ptr<llvm::Module>                   m_module;
    std::unordered_map<std::string, llvm::Type*>    m_typing_maps;

	/**< The type of base closure node. */
	const llvm::Type*   m_closure_base_type = nullptr;
    llvm::Function*     m_malloc_function = nullptr;
};

TSL_NAMESPACE_END