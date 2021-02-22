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
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include "tsl_version.h"
#include "tsl_system.h"
#include "types.h"
#include "system/impl.h"

TSL_NAMESPACE_BEGIN

class AstNode_FunctionPrototype;

struct StructMemberTypeMetaData {
    llvm::Type* m_llvm_type = nullptr;
    std::unordered_map<std::string, std::pair<int, DataType>> m_member_types;
};

using Var_MetaData = std::unordered_map<std::string, std::pair<llvm::Value*, DataType>>;
using Struct_Symbol_Table = std::unordered_map<std::string, StructMemberTypeMetaData>;
using Var_Symbol_Table_Stack = std::vector<Var_MetaData>;
using Func_Symbol_Table = std::unordered_map<std::string, std::pair<llvm::Function*, const AstNode_FunctionPrototype*>>;
using Closure_Symbol_Table = std::unordered_map<std::string, llvm::Function*>;
using Block_Stack = std::stack<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>>;

//! @brief  Compiling context of TSL.
/**
 * This data structure keeps track of all necessary information during compilation. Put it in other words,
 * it is more of an intermediate data structure to keep track of things during shader compilation.
 * Unless the memory is allocated inside this data structure, the compile context takes NO responsibility
 * of maintaining the lifetime of the data it points to.
 */
struct TSL_Compile_Context {
    // The llvm context pointer, it is just not an owning pointer.
    llvm::LLVMContext*      context = nullptr;
    // The llvm module, again this is not an owning pointer.
    llvm::Module*           module = nullptr;
    // llvm builder pointer
    llvm::IRBuilder<>*      builder = nullptr;
    // tsl global data type
    llvm::Type*             tsl_global_ty = nullptr;
    // tsl global value passed in
    llvm::Value*            tsl_global_value = nullptr;
    // a map keeps track of all global variables
    GlobalVarList*          tsl_global_mapping = nullptr;
    // a map keeps track of all shader resources
    ShaderResourceTable*    m_shader_resource_table = nullptr;

    // closured touched in the shader
    Closure_Symbol_Table            m_closures_maps;

    // a table keeps track of structure types
    Struct_Symbol_Table       m_structure_type_maps;

    // a table keeps track of function symbols
    Func_Symbol_Table       m_func_symbols;

    // a table keeps track of visited blocks
    Block_Stack             m_blocks;

    // get a variable if possible
    llvm::Value* get_var_symbol(const std::string& name, bool only_top_layer = false);

    // get a variable type if possible
    DataType     get_var_type(const std::string& name, bool only_top_layer = false);

    // push a variable into the variable symbol table
    llvm::Value* push_var_symbol(const std::string& name, llvm::Value* value, DataType type);

    // push/pop a symbol layer
    void push_var_symbol_layer();
    void pop_var_symbol_layer();

    // default constructor simply resets the context
    TSL_Compile_Context() {
        reset();
    }

    // reset the compile context
    void reset();

private:
    // variable symbol table
    Var_Symbol_Table_Stack  m_var_symbols;
};

TSL_NAMESPACE_END