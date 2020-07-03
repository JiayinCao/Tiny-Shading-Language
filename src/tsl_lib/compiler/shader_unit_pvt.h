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

#include <unordered_set>
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/LegacyPassManager.h"
#include "tslversion.h"
#include "shader_arg_types.h"

TSL_NAMESPACE_BEGIN

class GlobalModule;
class AstNode_FunctionPrototype;

struct ShaderArgMetaData {
    std::string             m_name;
    ShaderArgumentTypeEnum  m_type = ShaderArgumentTypeEnum::TSL_TYPE_INVALID;
    bool                    m_is_output = false;
    llvm::Value*            m_init_value = nullptr;
};

// This data structure hides all LLVM related data from ShaderUnitTemplate.
struct ShaderUnitTemplate_Pvt{
public:
    // the llvm module owned by this shader unit
    std::unique_ptr<llvm::Module> m_module = nullptr;

    // the dependent llvm module
    std::unordered_set<const llvm::Module*> m_dependencies;

    // ast node
    std::unique_ptr<const AstNode_FunctionPrototype> m_ast_root = nullptr;

    // llvm function pointer
    llvm::Function* m_llvm_function = nullptr;
    // root function name
    std::string     m_root_function_name;
};

// This data structure hides all LLVM related data from ShaderInstance.
struct ShaderInstance_Pvt {
public:
    // the execute engine for this module
    std::unique_ptr<llvm::ExecutionEngine> m_execution_engine = nullptr;

    // the legacy manager, this is for optimization
    std::unique_ptr<llvm::legacy::FunctionPassManager> m_fpm = nullptr;

    // the function address for host code to call
    uint64_t m_function_pointer = 0;
};

TSL_NAMESPACE_END