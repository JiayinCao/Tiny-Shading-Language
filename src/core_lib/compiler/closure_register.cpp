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
#include "compiler/ast.h"
#include "compiler/llvm_util.h"
#include "closure.h"

TSL_NAMESPACE_BEGIN

using namespace llvm;

bool ClosureRegister::init() {
    m_module = std::make_unique<llvm::Module>("shader", m_llvm_context);

    // declare some global data structure
    declare_closure_tree_types(m_llvm_context);

    return true;
}

void ClosureRegister::declare_closure_tree_types(llvm::LLVMContext& context, std::unordered_map<std::string, llvm::Type*>* mapping) {
    // ClosureTreeNodeBase, it has to have this 4 bytes memory padding in it so that the size is 8, otherwise, it will crash the system.
    const auto closure_tree_node_base = "closure_base";
    const std::vector<Type*> args_base = {
        Type::getInt32Ty(m_llvm_context),        /* this matches to m_id in ClosureTreeNodeBase. */
        Type::getInt32PtrTy(m_llvm_context)     /* this matches to m_params in ClosureTreeNodeBase. */
    };

    if(!mapping)
        m_typing_maps["closure_base"] = StructType::create(args_base, closure_tree_node_base);
    else
        (*mapping)["closure_base"] = StructType::create(args_base, closure_tree_node_base);

    // There should be two more to be defined.
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

    auto closure_param_type = StructType::create(arg_types, closure_type);

    // external declaration of malloc function
    std::vector<Type*> proto_args(1, Type::getInt32Ty(m_llvm_context));
    Function* malloc_function = Function::Create(FunctionType::get(Type::getInt32PtrTy(m_llvm_context), proto_args, false), Function::ExternalLinkage, "malloc", m_module.get());

    // the function to allocate the closure data structure
    auto closure_base_type = m_typing_maps["closure_base"];
    auto ret_type = closure_base_type->getPointerTo();
    Function* function = Function::Create(FunctionType::get(ret_type, arg_types, false), Function::ExternalLinkage, function_name, m_module.get());
    BasicBlock* bb = BasicBlock::Create(m_llvm_context, "EntryBlock", function);
    IRBuilder<> builder(bb);

    // allocate a structure for keeping parameters
    auto value = builder.CreateCall(malloc_function, { ConstantInt::get(m_llvm_context, APInt(32, structure_size)) }, "malloc");
    auto closure_param_ptr = builder.CreatePointerCast(value, closure_param_type->getPointerTo());

    for (int i = 0; i < arg_list.size(); ++i) {
        auto& arg = arg_list[i];

        // this obviously won't work for pointer type data, I will fix it later.
        auto var_type = get_type_from_context(arg.m_type, llvm_compiling_context);
        // auto var_ptr = builder.CreateConstGEP1_32(nullptr, closure_param_ptr, arg.m_offset / 4);
        auto var_ptr = builder.CreateConstGEP2_32(nullptr, closure_param_ptr, 0, arg.m_offset / 4);
        builder.CreateStore(function->getArg(i), var_ptr);

        arg_types.push_back(get_type_from_context(arg.m_type, llvm_compiling_context));
    }
    
    // allocate closure tree node
    value = builder.CreateCall(malloc_function, { ConstantInt::get(m_llvm_context, APInt(32, sizeof(ClosureTreeNodeBase))) }, "malloc");
    auto closure_tree_node_ptr = builder.CreatePointerCast(value, closure_base_type->getPointerTo());

    // assign the closure parameter pointer
    auto gep = builder.CreateConstGEP2_32(nullptr, closure_tree_node_ptr, 0, 1);
    auto dst_closure_param_ptr = builder.CreatePointerCast(gep, closure_param_type->getPointerTo()->getPointerTo());
    builder.CreateStore(closure_param_ptr, dst_closure_param_ptr);

    // return the tree node pointer
    builder.CreateRet(closure_tree_node_ptr);

    // for debugging purpose, set the name of the arguments
    for( int i = 0 ; i < arg_list.size() ; ++i ){
        auto arg = function->getArg(i);
        arg->setName(arg_list[i].m_name);
    }

    m_closures.insert(make_pair(name, ClosureItem(++m_current_closure_id, arg_list, structure_size)));

    return m_current_closure_id;
}

llvm::Function* ClosureRegister::declare_closure_function(const std::string& name, LLVM_Compile_Context& context) {
    auto it = m_closures.find(name);
    if (it == m_closures.end())
        return nullptr;

    auto& llvm_context = *context.context;
    auto& module = *context.module;
    const auto& closure_item = it->second;

    std::vector<Type*> arg_types;
    for (auto& arg : closure_item.m_var_list)
        arg_types.push_back(get_type_from_context(arg.m_type, context));

    const std::string closure_type = "closure_type_" + name;
    auto closure_tree_node_base = StructType::create(arg_types, closure_type);

    // external declaration of malloc function
    const std::string function_name = "make_closure_" + name;

    auto ret_type = context.m_closure_type_maps["closure_base"]->getPointerTo();
    return Function::Create(FunctionType::get(ret_type, arg_types, false), Function::ExternalLinkage, function_name, module);
}

TSL_NAMESPACE_END