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

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>
#include <mutex>
#include <memory>
#include <unordered_map>
#include "tsl_version.h"
#include "ast.h"
#include "tsl_args.h"

TSL_NAMESPACE_BEGIN

struct TSL_Compile_Context;

struct ClosureItem {
    const ClosureID         m_closure_id = INVALID_CLOSURE_ID;
    const ClosureArgList&   m_var_list;
    const int               m_structure_size;

    ClosureItem(ClosureID id, ClosureArgList& var_list, int structure_size) 
        : m_closure_id(id), m_var_list(var_list), m_structure_size(structure_size) {}
};

//! @brief  Global module of TSL.
/**
 * Unlike other modules owned by shader unit template, global module only have one instance owned by the system.
 * It is mainly for defining functions to allocate registered closure data structures.
 * Every single module will have this module attached so that it will know some of the very fundermental data
 * available in TSL
 */
class GlobalModule {
public:
    // initialize the register
    bool init();

    // register a closure type
    ClosureID       register_closure_type(const std::string& name, ClosureArgList& mapping, int structure_size);

    // get global closure maker module
    llvm::Module*   get_closure_module();

    // declare some global data structure type
    void            declare_closure_tree_types(llvm::LLVMContext& context, Struct_Symbol_Table* mapping = nullptr );

    // get declaration
    llvm::Function* declare_closure_function(const std::string& name, TSL_Compile_Context& context);

    // declare global function
    void            declare_global_module(TSL_Compile_Context& context);

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

    Tsl_Namespace::TSL_Compile_Context m_llvm_compiling_context;
};

TSL_NAMESPACE_END