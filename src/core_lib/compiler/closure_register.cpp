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

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "closure_register.h"
#include "compiler/llvm_util.h"

TSL_NAMESPACE_BEGIN

using namespace llvm;

bool ClosureRegister::init() {
    m_module = std::make_unique<llvm::Module>("shader", m_llvm_context);
    return true;
}

llvm::Module* ClosureRegister::get_closure_module() {
    return m_module.get();
}

ClosureID ClosureRegister::register_closure_type(const std::string& name, ClosureVarList& arg_list, int structure_size) {
    std::lock_guard<std::mutex> lock(m_closure_mutex);

    if (m_closures.count(name))
        return INVALID_CLOSURE_ID;
    
    Tsl_Namespace::LLVM_Compile_Context llvm_compiling_context;
    llvm_compiling_context.module = m_module.get();
    llvm_compiling_context.context = &m_llvm_context;

    // check if the closure is already registered.
    if (m_closures.count(name))
        return false;

    const std::string closure_type = "closure_type_" + name;
    const std::string function_name = "make_closure_" + name;

    std::vector<Type*> arg_types;
    for (auto& arg : arg_list)
        arg_types.push_back(get_type_from_context(arg.m_type, llvm_compiling_context));

    auto closure_tree_node_base = StructType::create(arg_types, closure_type);

    // external declaration of malloc function
    std::vector<Type*> proto_args(1, Type::getInt32Ty(m_llvm_context));
    Function* malloc_function = Function::Create(FunctionType::get(Type::getInt32PtrTy(m_llvm_context), proto_args, false), Function::ExternalLinkage, "malloc", m_module.get());

    // the function to allocate the closure data structure
    Function* function = Function::Create(FunctionType::get(closure_tree_node_base->getPointerTo(), arg_types, false), Function::ExternalLinkage, function_name, m_module.get());
    BasicBlock* bb = BasicBlock::Create(m_llvm_context, "EntryBlock", function);
    IRBuilder<> builder(bb);

    // allocate a structure
    std::vector<Value*> args(1, ConstantInt::get(m_llvm_context, APInt(32, structure_size)));
    auto value = builder.CreateCall(malloc_function, args, "malloc");
    auto closure_ptr = builder.CreatePointerCast(value, closure_tree_node_base->getPointerTo());

    for (int i = 0; i < arg_list.size(); ++i) {
        auto& arg = arg_list[i];

        // this obviously won't work for pointer type data, I will fix it later.
        auto var_ptr = builder.CreateConstGEP1_32(nullptr, closure_ptr, arg.m_offset);
        builder.CreateStore(function->getArg(i), var_ptr);

        arg_types.push_back(get_type_from_context(arg.m_type, llvm_compiling_context));
    }
    
    builder.CreateRet(closure_ptr);

    // for debugging purpose, set the name of the arguments
    for( int i = 0 ; i < arg_list.size() ; ++i ){
        auto arg = function->getArg(i);
        arg->setName(arg_list[i].m_name);
    }

    m_module->print(errs(), nullptr);
    
    return m_closures[name] = ++m_current_closure_id;
}

TSL_NAMESPACE_END