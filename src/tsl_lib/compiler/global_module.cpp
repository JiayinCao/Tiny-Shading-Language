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
#include "global_module.h"
#include "compiler/ast.h"
#include "compiler/llvm_util.h"
#include "closure.h"
#include "tsl_std.h"

TSL_NAMESPACE_BEGIN

using namespace llvm;

bool GlobalModule::init() {
    // this global module always exists in the system
    m_module = std::make_unique<llvm::Module>("tsl_global_module", m_llvm_context);

    // declare some global data structure
    declare_closure_tree_types(m_llvm_context);

    // this malloc needs to be replaced once the memory allocator is available
    std::vector<Type*> proto_args;
    proto_args.push_back(Type::getInt32Ty(m_llvm_context));
    // proto_args.push_back(Type::getInt32Ty(m_llvm_context)->getPointerTo());
    m_malloc_function = Function::Create(FunctionType::get(Type::getInt32PtrTy(m_llvm_context), proto_args, false), Function::ExternalLinkage, "TSL_MALLOC", m_module.get());

    return true;
}

void GlobalModule::declare_closure_tree_types(llvm::LLVMContext& context, StructSymbolTable* mapping) {
	// N.B, this function implementation will be 'externally' defined in std.tsl

    // ClosureTreeNodeBase, it has to have this 4 bytes memory padding in it so that the size is 8, otherwise, it will crash the system.
    const auto closure_tree_node_base = "closure_base";
    const std::vector<Type*> args_base = {
        Type::getInt32Ty(context),       /* m_id */
        Type::getInt32PtrTy(context)     /* m_params */
    };
	auto closure_tree_node_base_ty = StructType::create(args_base, closure_tree_node_base);

    // ClosureTreeNodeMul
	const auto closure_tree_node_mul = "closure_mul";
	const std::vector<Type*> args_mul = {
		Type::getInt32Ty(context),       /* m_id */
		Type::getInt32PtrTy(context),    /* m_params */
		Type::getFloatTy(context),		 /* m_weight */
		Type::getInt32PtrTy(context)	 /* m_closure */
	};
	auto closure_tree_node_mul_ty = StructType::create(args_mul, closure_tree_node_mul);

    // ClosureTreeNodeMul
    const auto closure_tree_node_add = "closure_add";
    const std::vector<Type*> args_add = {
        Type::getInt32Ty(context),       /* m_id */
        Type::getInt32PtrTy(context),    /* m_params */
        Type::getInt32PtrTy(context),    /* m_closure0 */
        Type::getInt32PtrTy(context)	 /* m_closure1 */
    };
    auto closure_tree_node_add_ty = StructType::create(args_add, closure_tree_node_add);

	// keep track of the allocated type
	if (!mapping) {
		m_closure_base_type = StructType::create(args_base, closure_tree_node_base);
	} else {
		(*mapping)["closure_base"].m_llvm_type = closure_tree_node_base_ty;
		(*mapping)["closure_mul"].m_llvm_type = closure_tree_node_mul_ty;
		(*mapping)["closure_add"].m_llvm_type = closure_tree_node_add_ty;
	}
}

llvm::Module* GlobalModule::get_closure_module() {
    return m_module.get();
}

void GlobalModule::register_tsl_global(GlobalVarList& mapping) {
    m_tsl_global_mapping = mapping;
}

void GlobalModule::declare_tsl_global(LLVM_Compile_Context& context) {
    // it is allowed that tsl global has nothing.
    if (m_tsl_global_mapping.empty())
        return;

    // assemble the variable types
    std::vector<Type*> arg_types;
    for (auto& arg : m_tsl_global_mapping) {
        auto type = get_type_from_context(arg.m_type, context);
        // this is a VERY DIRTY hack, I'll try to get back to it once most features are done.
        if (!type)
            type = get_int_32_ptr_ty(context);
        arg_types.push_back(type);
    }

    const std::string tsl_global_name = "Tsl_Global";
    context.tsl_global_ty = StructType::create(arg_types, tsl_global_name);
    context.m_tsl_global_mapping = m_tsl_global_mapping;
}

ClosureID GlobalModule::register_closure_type(const std::string& name, ClosureVarList& arg_list, int structure_size) {
    std::lock_guard<std::mutex> lock(m_closure_mutex);

	// if it is already registered, simply return with the previous registered id
	const auto it = m_closures.find(name);
    if (it != m_closures.end())
        return it->second.m_closure_id;
    
	// construct the llvm compile context
    Tsl_Namespace::LLVM_Compile_Context llvm_compiling_context;
    llvm_compiling_context.module = m_module.get();
    llvm_compiling_context.context = &m_llvm_context;

    const auto closure_type_name = "closure_type_" + name;
    const auto function_name = "make_closure_" + name;

	// assemble the variable types
    std::vector<Type*> arg_types;
    for (auto& arg : arg_list) {
        auto type = get_type_from_context(arg.m_type, llvm_compiling_context);
        // this is a VERY DIRTY hack, I'll try to get back to it once most features are done.
        if (!type)
            type = get_int_32_ptr_ty(llvm_compiling_context);
        arg_types.push_back(type);
    }

	// declare the closure parameter data structure
    const auto closure_param_type = StructType::create(arg_types, closure_type_name);

    // the function to allocate the closure data structure
    Function* function = Function::Create(FunctionType::get(m_closure_base_type->getPointerTo(), arg_types, false), Function::ExternalLinkage, function_name, m_module.get());
    IRBuilder<> builder(BasicBlock::Create(m_llvm_context, "EntryBlock", function));

    // allocate a structure for keeping parameters
    const auto param_table_ptr = builder.CreateCall(m_malloc_function, { ConstantInt::get(m_llvm_context, APInt(32, structure_size)) }, "TSL_MALLOC");
    const auto converted_param_table_ptr = builder.CreatePointerCast(param_table_ptr, closure_param_type->getPointerTo());

	// copy all variables in this parameter table
    for (int i = 0; i < arg_list.size(); ++i) {
        const auto& arg = arg_list[i];

        // this obviously won't work for pointer type data, I will fix it later.
        auto var_type = get_type_from_context(arg.m_type, llvm_compiling_context);
        // this is a VERY DIRTY hack, I'll try to get back to it once most features are done.
        if (!var_type)
            var_type = get_int_32_ptr_ty(llvm_compiling_context);

        const auto var_ptr = builder.CreateConstGEP2_32(nullptr, converted_param_table_ptr, 0, i);
        builder.CreateStore(function->getArg(i), var_ptr);
    }
    
    // allocate closure tree node
    auto closure_tree_node_ptr = builder.CreateCall(m_malloc_function, { ConstantInt::get(m_llvm_context, APInt(32, sizeof(ClosureTreeNodeBase))) });
    auto converted_closure_tree_node_ptr = builder.CreatePointerCast(closure_tree_node_ptr, m_closure_base_type->getPointerTo());

	// setup closure id
	auto closure_id = ConstantInt::get(m_llvm_context, APInt(32, m_current_closure_id));
	auto gep0 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 0);
	auto dst_closure_id_ptr = builder.CreatePointerCast(gep0, Type::getInt32PtrTy(m_llvm_context));
	builder.CreateStore(closure_id, dst_closure_id_ptr);

    // assign the closure parameter pointer
    auto gep1 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 1);
    auto dst_closure_param_ptr = builder.CreatePointerCast(gep1, closure_param_type->getPointerTo()->getPointerTo());
    builder.CreateStore(converted_param_table_ptr, dst_closure_param_ptr);

    // return the tree node pointer
    builder.CreateRet(closure_tree_node_ptr);

    // for debugging purpose, set the name of the arguments
    for( int i = 0 ; i < arg_list.size() ; ++i ){
        auto arg = function->getArg(i);
        arg->setName(arg_list[i].m_name);
    }

    m_closures.insert(make_pair(name, ClosureItem(m_current_closure_id, arg_list, structure_size)));

    return m_current_closure_id++;
}

llvm::Function* GlobalModule::declare_closure_function(const std::string& name, LLVM_Compile_Context& context) {
    const auto it = m_closures.find(name);
    if (it == m_closures.end())
        return nullptr;

    std::vector<Type*> arg_types;
    for (auto& arg : it->second.m_var_list) {
        // this is a VERY DIRTY hack, I'll try to get back to it once most features are done.
        auto var_type = get_type_from_context(arg.m_type, context);
        if (!var_type)
            var_type = get_int_32_ptr_ty(context);

        arg_types.push_back(var_type);
    }

    const auto function_name = "make_closure_" + name;
    const auto ret_type = context.m_structure_type_maps["closure_base"].m_llvm_type->getPointerTo();
    return Function::Create(FunctionType::get(ret_type, arg_types, false), Function::ExternalLinkage, function_name, *context.module);
}

void GlobalModule::declare_global_module(LLVM_Compile_Context& context){
	// this malloc needs to be replaced once the memory allocator is available
	// Function* malloc_function = Function::Create(FunctionType::get(get_int_32_ptr_ty(context), { get_int_32_ty(context) , get_int_32_ptr_ty(context) }, false), Function::ExternalLinkage, "TSL_ALLOC", context.module);
    Function* malloc_function = Function::Create(FunctionType::get(get_int_32_ptr_ty(context), { get_int_32_ty(context) }, false), Function::ExternalLinkage, "TSL_MALLOC", context.module);

	context.m_func_symbols["TSL_MALLOC"] = std::make_pair(malloc_function, nullptr);

	// float3 data structure, this can be used as vector, color in TSL
	const auto float3_struct = "float3";
	const std::vector<Type*> float3_members = {
		get_float_ty(context),
		get_float_ty(context),
		get_float_ty(context)
	};
	auto float3_struct_llvm_type = StructType::create(float3_members, float3_struct);

	StructMemberTypeMetaData float3_meta_data;
	float3_meta_data.m_llvm_type = float3_struct_llvm_type;
	float3_meta_data.m_member_types["x"] = { 0 , { DataTypeEnum::FLOAT , nullptr } };
	float3_meta_data.m_member_types["y"] = { 1 , { DataTypeEnum::FLOAT , nullptr } };
	float3_meta_data.m_member_types["z"] = { 2 , { DataTypeEnum::FLOAT , nullptr } };
	float3_meta_data.m_member_types["r"] = { 0 , { DataTypeEnum::FLOAT , nullptr } };
	float3_meta_data.m_member_types["g"] = { 1 , { DataTypeEnum::FLOAT , nullptr } };
	float3_meta_data.m_member_types["b"] = { 2 , { DataTypeEnum::FLOAT , nullptr } };
	context.m_structure_type_maps["float3"] = float3_meta_data;

    // add some intrinsic data structure
    const auto float4_struct = "float4";
    const std::vector<Type*> float4_members = {
        get_float_ty(context),
        get_float_ty(context),
        get_float_ty(context),
        get_float_ty(context)
    };
    auto closure_tree_node_base_ty = StructType::create(float4_members, float4_struct);

    StructMemberTypeMetaData float4_meta_data;
    float4_meta_data.m_llvm_type = closure_tree_node_base_ty;
    float4_meta_data.m_member_types["x"] = { 0 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["y"] = { 1 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["z"] = { 2 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["w"] = { 3 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["r"] = { 0 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["g"] = { 1 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["b"] = { 2 , { DataTypeEnum::FLOAT , nullptr } };
    float4_meta_data.m_member_types["w"] = { 3 , { DataTypeEnum::FLOAT , nullptr } };
    context.m_structure_type_maps["float4"] = float4_meta_data;
}

TSL_NAMESPACE_END