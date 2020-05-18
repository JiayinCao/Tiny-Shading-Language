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

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "tslversion.h"

TSL_NAMESPACE_BEGIN

// This data structure hides all LLVM related data from ShaderUnit.
struct ShaderUnit_Pvt{
public:
    // the llvm module owned by this shader unit
    std::unique_ptr<llvm::Module> m_module = nullptr;

    // the execute engine for this module
    std::unique_ptr<llvm::ExecutionEngine> m_execution_engine = nullptr;

    // the function address for host code to call
    uint64_t m_function_pointer = uint64_t(nullptr);
};

TSL_NAMESPACE_END