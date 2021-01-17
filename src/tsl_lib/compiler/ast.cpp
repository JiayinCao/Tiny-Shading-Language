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

#include <iostream>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include "ast.h"
#include "llvm_util.h"
#include "global_module.h"
#include "tsl_args.h"
#include "system/impl.h"

using namespace llvm;

TSL_NAMESPACE_BEGIN


llvm::Function* AstNode_FunctionPrototype::codegen( TSL_Compile_Context& context ) const {
    static std::vector<std::shared_ptr<const AstNode_SingleVariableDecl>> empty_args;

    // no function overloading for simplicity, at least for now.
    if (context.m_func_symbols.count(m_name) != 0) {
        std::cout << "Duplicated function named : " << m_name << std::endl;
        return nullptr;
    }

    const auto& args = m_variables ? m_variables->get_var_list() : empty_args;
    const auto arg_cnt = args.size();

    // clear the symbol maps, no global var for now
    context.push_var_symbol_layer();

	// parse argument types
	std::vector<llvm::Type*>	llvm_args;
	int i = 0;
	for( auto& arg : args ){
        auto raw_type = get_type_from_context(arg->data_type(), context);
        llvm_args.push_back((arg->get_config() & VariableConfig::OUTPUT) ? raw_type->getPointerTo() : raw_type);
	}

    // the last argument is always tsl_global
    if(context.tsl_global_ty && m_is_shader)
        llvm_args.push_back(context.tsl_global_ty->getPointerTo());

	// parse return types
	auto return_type = get_type_from_context(m_return_type , context);

	// declare the function prototype
	llvm::FunctionType *function_type = llvm::FunctionType::get(return_type, llvm_args, false);

	if( !function_type )
		return nullptr;

	// create the function prototype
    const auto link_type = ( m_is_shader || ( m_body == nullptr ) )? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;
	llvm::Function* function = llvm::Function::Create(function_type, link_type, m_name, context.module);

	// For debugging purposes, set the name of all arguments
    i = 0;
	for (auto &arg : function->args()) {
        if (i >= args.size()) {
            arg.setName("tsl_global");
            context.tsl_global_value = &arg;
            break;
        }
		arg.setName(args[i]->get_var_name());
        ++i;
	}

    context.m_func_symbols[m_name] = std::make_pair(function, this);

    if( m_body ){
        // create a separate code block
	    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context.context, "entry", function);
	    context.builder->SetInsertPoint(BB);

        // push the argument into the symbol table first
        int i = 0;
        for( auto& variable : args ){
            const auto name = variable->get_var_name();
            const auto raw_type = get_type_from_context(variable->data_type(), context);

            if( nullptr != context.get_var_symbol(name, true) ){
                emit_error("Redefined argument '%s' in function '%s'.", name, m_name.c_str());
                return nullptr;
            }

            auto arg = function->getArg(i);
            if (variable->get_config() & VariableConfig::OUTPUT) {
                context.push_var_symbol(name, arg, variable->data_type());
            } else {
                // allocate the variable on stack
                auto alloc_var = context.builder->CreateAlloca(raw_type, nullptr, name);

                // duplicate the value so that it can be a copy instead of a reference.
                context.builder->CreateStore(arg, alloc_var);
                
                context.push_var_symbol(name, alloc_var, variable->data_type());
            }
            ++i;
        }
        
        if(m_body->m_statements)
            m_body->m_statements->codegen(context);

        auto& last_block = function->getBasicBlockList().back();
        if (nullptr == last_block.getTerminator())
            context.builder->CreateRetVoid();
    }

    context.pop_var_symbol_layer();

	return function;
}

void AstNode_FunctionPrototype::parse_shader_parameters(std::vector<ExposedArgDescriptor>& params) const {
    params.clear();

    if (m_variables) {
        const auto& args = m_variables->get_var_list();
        for (auto& variable : args) {
            const auto raw_type = variable->data_type();

            ExposedArgDescriptor arg;
            arg.m_name = variable->get_var_name();
            arg.m_type = raw_type;
            arg.m_is_output = variable->get_config() & VariableConfig::OUTPUT;
            params.push_back(arg);
        }
    }
}

llvm::Value* AstNode_FunctionBody::codegen( TSL_Compile_Context& context ) const{    
	return nullptr;
}

llvm::Value* AstNode_Literal_Int::codegen(TSL_Compile_Context& context) const {
    return get_llvm_constant_int(m_val, 32, context);
}

llvm::Value* AstNode_Literal_Flt::codegen(TSL_Compile_Context& context) const {
    return get_llvm_constant_fp(m_val, context);
}

llvm::Value* AstNode_Literal_Double::codegen(TSL_Compile_Context& context) const {
    return get_llvm_constant_fp(m_val, context);
}

llvm::Value* AstNode_Literal_Bool::codegen(TSL_Compile_Context& context) const {
    return get_llvm_constant_int((int)m_val, 1, context);
}

llvm::Value* AstNode_Literal_GlobalValue::codegen(TSL_Compile_Context& context) const {
    if (!context.tsl_global_mapping) {
        emit_error("TSL global variable is not registered.");
        return nullptr;
    }
    const auto& tsl_global_mapping = context.tsl_global_mapping->m_var_list;

    for (int i = 0; i < tsl_global_mapping.size(); ++i){
        const auto& arg = tsl_global_mapping[i];
        if (arg.m_name == m_value_name) {
            if (!context.tsl_global_value) {
                emit_error("TSL global variable is not passed in, fatal error.");
                return nullptr;
            }

            auto gep0 = context.builder->CreateConstGEP2_32(nullptr, context.tsl_global_value, 0, i);
            return context.builder->CreateLoad(gep0);
        }
    }
    
    emit_error("Unregistered global value '%s'.", m_value_name.c_str());

    return nullptr;
}

bool AstNode_Binary_Add::is_closure(TSL_Compile_Context& context) const {
    if (m_left->is_closure(context) != m_right->is_closure(context)) {
        emit_error("Closure color can't be added with non closure color.");
        return false;
    }

    return m_left->is_closure(context) && m_right->is_closure(context);
}

llvm::Value* AstNode_Binary_Add::codegen(TSL_Compile_Context& context) const {
    auto& builder = *context.builder;

    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (!m_left->is_closure(context) && !m_right->is_closure(context)) {
        // something is not right
        if (!left || !right)
            return nullptr;

        const auto float3_struct_ty = context.m_structure_type_maps["float3"].m_llvm_type;
        const auto float_ty = get_float_ty(context);

        if (left->getType() == float3_struct_ty && right->getType() == float3_struct_ty) {
            auto ret = builder.CreateAlloca(float3_struct_ty);

            // I have no idea how to access per-channel data from a struct.
            // Until I figure out a better solution, I'll live with the current one.
            auto tmp_left = builder.CreateAlloca(float3_struct_ty);
            builder.CreateStore(left, tmp_left);
            builder.CreateStore(right, ret);

            auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
            auto left_x = builder.CreateConstGEP2_32(nullptr, tmp_left, 0, 0);
            auto mul_x = get_llvm_add(builder.CreateLoad(left_x), builder.CreateLoad(ret_x), context);
            builder.CreateStore(mul_x, ret_x);

            auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
            auto left_y = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 1);
            auto mul_y = get_llvm_add(builder.CreateLoad(left_y), builder.CreateLoad(ret_y), context);
            builder.CreateStore(mul_y, ret_y);

            auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
            auto left_z = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 2);
            auto mul_z = get_llvm_add(builder.CreateLoad(left_z), builder.CreateLoad(ret_z), context);
            builder.CreateStore(mul_z, ret_z);

            return builder.CreateLoad(ret);
        }
        else if ( ( left->getType() == float3_struct_ty && right->getType() == float_ty ) ||
            ( left->getType() == float_ty && right->getType() == float3_struct_ty ) ) {
            // always make sure left is the float3
            if (left->getType() == float_ty) {
                auto tmp = left;
                left = right;
                right = tmp;
            }

            auto ret = builder.CreateAlloca(float3_struct_ty);

            builder.CreateStore(left, ret);

            auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
            auto mul_x = get_llvm_add(builder.CreateLoad(ret_x), right, context);
            builder.CreateStore(mul_x, ret_x);

            auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
            auto mul_y = get_llvm_add(builder.CreateLoad(ret_y), right, context);
            builder.CreateStore(mul_y, ret_y);

            auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
            auto mul_z = get_llvm_add(builder.CreateLoad(ret_z), right, context);
            builder.CreateStore(mul_z, ret_z);

            return builder.CreateLoad(ret);
        }

        return get_llvm_add(left, right, context);
    }

    // this must be a closure multiplied by a regular expression
    if (!(m_left->is_closure(context) && m_right->is_closure(context))) {
        emit_error("Closure color can't be added with non closure color.");
        return nullptr;
    }

    auto malloc_function = context.m_func_symbols["TSL_MALLOC"].first;
    if (!malloc_function) {
        // this should not happen at all
        return nullptr;
    }

    const auto closure_tree_node_type = context.m_structure_type_maps["closure_add"].m_llvm_type;
    const auto closure_tree_node_ptr_type = closure_tree_node_type->getPointerTo();

    // allocate the tree data structure
    std::vector<llvm::Value*> args = { ConstantInt::get(*context.context, APInt(32, sizeof(ClosureTreeNodeAdd))) };
    auto closure_tree_node_ptr = builder.CreateCall(malloc_function, args);
    auto converted_closure_tree_node_ptr = builder.CreatePointerCast(closure_tree_node_ptr, closure_tree_node_ptr_type);

    // setup closure id
    auto node_mul_id = get_llvm_constant_int(CLOSURE_ADD, 32, context);
    auto gep0 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 0);
    auto dst_closure_id_ptr = builder.CreatePointerCast(gep0, get_int_32_ptr_ty(context));
    builder.CreateStore(node_mul_id, dst_closure_id_ptr);

    // assign one pointer
    auto gep1 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 2);
    auto weight_ptr = builder.CreatePointerCast(gep1, left->getType()->getPointerTo());
    builder.CreateStore(left, weight_ptr);

    // assign the other pointer
    auto gep2 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 3);
    auto closure_ptr = builder.CreatePointerCast(gep2, right->getType()->getPointerTo());
    builder.CreateStore(right, closure_ptr);

    const auto final_type = get_closure_ty(context);
    auto converted_ret = builder.CreatePointerCast(converted_closure_tree_node_ptr, final_type);
    return converted_ret;
}

llvm::Value* AstNode_Binary_Minus::codegen(TSL_Compile_Context& context) const {
    auto& builder = *context.builder;

    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    // something is not right
    if (!left || !right)
        return nullptr;

    const auto float3_struct_ty = context.m_structure_type_maps["float3"].m_llvm_type;
    const auto float_ty = get_float_ty(context);

    // piece wise multiplication
    if (left->getType() == float3_struct_ty && right->getType() == float3_struct_ty) {
        auto ret = builder.CreateAlloca(float3_struct_ty);

        // I have no idea how to access per-channel data from a struct.
        // Until I figure out a better solution, I'll live with the current one.
        auto tmp_left = builder.CreateAlloca(float3_struct_ty);
        builder.CreateStore(left, tmp_left);
        builder.CreateStore(right, ret);

        auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
        auto left_x = builder.CreateConstGEP2_32(nullptr, tmp_left, 0, 0);
        auto sub_x = get_llvm_sub(builder.CreateLoad(left_x), builder.CreateLoad(ret_x), context);
        builder.CreateStore(sub_x, ret_x);

        auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
        auto left_y = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 1);
        auto sub_y = get_llvm_sub(builder.CreateLoad(left_y), builder.CreateLoad(ret_y), context);
        builder.CreateStore(sub_y, ret_y);

        auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
        auto left_z = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 2);
        auto sub_z = get_llvm_sub(builder.CreateLoad(left_z), builder.CreateLoad(ret_z), context);
        builder.CreateStore(sub_z, ret_z);

        return builder.CreateLoad(ret);
    }
    else if (left->getType() == float3_struct_ty && right->getType() == float_ty) {
        auto ret = builder.CreateAlloca(float3_struct_ty);

        builder.CreateStore(left, ret);

        auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
        auto sub_x = get_llvm_sub(builder.CreateLoad(ret_x), right, context);
        builder.CreateStore(sub_x, ret_x);

        auto ret_y = builder.CreateConstGEP2_32(nullptr, ret, 0, 1);
        auto sub_y = get_llvm_sub(builder.CreateLoad(ret_y), right, context);
        builder.CreateStore(sub_y, ret_y);

        auto ret_z = builder.CreateConstGEP2_32(nullptr, ret, 0, 2);
        auto sub_z = get_llvm_sub(builder.CreateLoad(ret_z), right, context);
        builder.CreateStore(sub_z, ret_z);

        return builder.CreateLoad(ret);
    }
    else if (left->getType() == float_ty && right->getType() == float3_struct_ty) {
        auto ret = builder.CreateAlloca(float3_struct_ty);

        auto tmp_right = builder.CreateAlloca(float3_struct_ty);
        builder.CreateStore(right, tmp_right);

        auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
        auto right_x = builder.CreateConstGEP2_32(nullptr, tmp_right, 0, 0);
        auto sub_x = get_llvm_sub(left, builder.CreateLoad(right_x), context);
        builder.CreateStore(sub_x, ret_x);

        auto ret_y = builder.CreateConstGEP2_32(nullptr, ret, 0, 1);
        auto right_y = context.builder->CreateConstGEP2_32(nullptr, tmp_right, 0, 1);
        auto sub_y = get_llvm_sub(left, builder.CreateLoad(right_y), context);
        builder.CreateStore(sub_y, ret_y);

        auto ret_z = builder.CreateConstGEP2_32(nullptr, ret, 0, 2);
        auto right_z = context.builder->CreateConstGEP2_32(nullptr, tmp_right, 0, 2);
        auto sub_z = get_llvm_sub(left, builder.CreateLoad(right_z), context);
        builder.CreateStore(sub_z, ret_z);

        return builder.CreateLoad(ret);
    }

    return get_llvm_sub(left, right, context);
}

bool AstNode_Binary_Multi::is_closure(TSL_Compile_Context& context) const {
    if (m_left->is_closure(context) && m_right->is_closure(context)) {
        emit_error("Closure color can't muliply with each other.");
        return false;
    }

    return m_left->is_closure(context) || m_right->is_closure(context);
}

llvm::Value* AstNode_Binary_Multi::codegen(TSL_Compile_Context& context) const {
    auto& builder = *context.builder;

    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);
    
    // something is not right
    if (!left || !right)
        return nullptr;

    if (!m_left->is_closure(context) && !m_right->is_closure(context)) {
        const auto float3_struct_ty = context.m_structure_type_maps["float3"].m_llvm_type;
        const auto float_ty = get_float_ty(context);

        // piece wise multiplication
        if (left->getType() == float3_struct_ty && right->getType() == float3_struct_ty) {
            auto ret = builder.CreateAlloca(float3_struct_ty);

            // I have no idea how to access per-channel data from a struct.
            // Until I figure out a better solution, I'll live with the current one.
            auto tmp_left = builder.CreateAlloca(float3_struct_ty);
            builder.CreateStore(left, tmp_left);
            builder.CreateStore(right, ret);

            auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
            auto left_x = builder.CreateConstGEP2_32(nullptr, tmp_left, 0, 0);
            auto mul_x = get_llvm_mul(builder.CreateLoad(left_x), builder.CreateLoad(ret_x), context);
            builder.CreateStore(mul_x, ret_x);

            auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
            auto left_y = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 1);
            auto mul_y = get_llvm_mul(builder.CreateLoad(left_y), builder.CreateLoad(ret_y), context);
            builder.CreateStore(mul_y, ret_y);

            auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
            auto left_z = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 2);
            auto mul_z = get_llvm_mul(builder.CreateLoad(left_z), builder.CreateLoad(ret_z), context);
            builder.CreateStore(mul_z, ret_z);

            return builder.CreateLoad(ret);
        }
        else if (( left->getType() == float3_struct_ty && right->getType() == float_ty ) ||
                 ( left->getType() == float_ty && right->getType() == float3_struct_ty )) {
            // always make sure left is the float3
            if (left->getType() == float_ty) {
                auto tmp = left;
                left = right;
                right = tmp;
            }

            auto ret = builder.CreateAlloca(float3_struct_ty);

            builder.CreateStore(left, ret);

            auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
            auto mul_x = get_llvm_mul(builder.CreateLoad(ret_x), right, context);
            builder.CreateStore(mul_x, ret_x);

            auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
            auto mul_y = get_llvm_mul(builder.CreateLoad(ret_y), right, context);
            builder.CreateStore(mul_y, ret_y);

            auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
            auto mul_z = get_llvm_mul(builder.CreateLoad(ret_z), right, context);
            builder.CreateStore(mul_z, ret_z);

            return builder.CreateLoad(ret);
        }

        return get_llvm_mul(left, right, context);
    }

	// this must be a closure multiplied by a regular expression
	if(m_left->is_closure(context) && m_right->is_closure(context)){
        emit_error("Closure color can't muliply with each other.");
		return nullptr;
	}

	auto closure = m_left->is_closure(context) ? left : right;
	auto weight = m_left->is_closure(context) ? right : left;

	auto malloc_function = context.m_func_symbols["TSL_MALLOC"].first;
	if( !malloc_function ){
		// this should not happen at all
		return nullptr;
	}

	const auto closure_tree_node_type = context.m_structure_type_maps["closure_mul"].m_llvm_type;
	const auto closure_tree_node_ptr_type = closure_tree_node_type->getPointerTo();

	// allocate the tree data structure
	std::vector<llvm::Value*> args = { ConstantInt::get(*context.context, APInt(32, sizeof(ClosureTreeNodeMul))) };
	auto closure_tree_node_ptr = builder.CreateCall(malloc_function, args);
	auto converted_closure_tree_node_ptr = builder.CreatePointerCast(closure_tree_node_ptr, closure_tree_node_ptr_type);

	// setup closure id
	auto node_mul_id = get_llvm_constant_int(CLOSURE_MUL, 32, context);
	auto gep0 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 0);
	auto dst_closure_id_ptr = builder.CreatePointerCast(gep0, get_int_32_ptr_ty(context));
	builder.CreateStore(node_mul_id, dst_closure_id_ptr);

	// assign the closure parameter pointer
	auto gep1 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 2);
	auto weight_ptr = builder.CreatePointerCast(gep1, get_float_ptr_ty(context));
	builder.CreateStore(weight, weight_ptr);

	// copy the pointer
	auto gep2 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 3);
	auto closure_ptr = builder.CreatePointerCast(gep2, closure->getType()->getPointerTo());
	builder.CreateStore(closure, closure_ptr);

    const auto final_type = get_closure_ty(context);
    auto converted_ret = builder.CreatePointerCast(converted_closure_tree_node_ptr, final_type);
    return converted_ret;
}

llvm::Value* AstNode_Binary_Div::codegen(TSL_Compile_Context& context) const {
    auto& builder = *context.builder;

    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    // something is not right
    if (!left || !right)
        return nullptr;

    const auto float3_struct_ty = context.m_structure_type_maps["float3"].m_llvm_type;
    const auto float_ty = get_float_ty(context);

    // piece wise multiplication
    if (left->getType() == float3_struct_ty && right->getType() == float3_struct_ty) {
        auto ret = builder.CreateAlloca(float3_struct_ty);

        // I have no idea how to access per-channel data from a struct.
        // Until I figure out a better solution, I'll live with the current one.
        auto tmp_left = builder.CreateAlloca(float3_struct_ty);
        builder.CreateStore(left, tmp_left);
        builder.CreateStore(right, ret);

        auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
        auto left_x = builder.CreateConstGEP2_32(nullptr, tmp_left, 0, 0);
        auto sub_x = get_llvm_div(builder.CreateLoad(left_x), builder.CreateLoad(ret_x), context);
        builder.CreateStore(sub_x, ret_x);

        auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
        auto left_y = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 1);
        auto sub_y = get_llvm_div(builder.CreateLoad(left_y), builder.CreateLoad(ret_y), context);
        builder.CreateStore(sub_y, ret_y);

        auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
        auto left_z = context.builder->CreateConstGEP2_32(nullptr, tmp_left, 0, 2);
        auto sub_z = get_llvm_div(builder.CreateLoad(left_z), builder.CreateLoad(ret_z), context);
        builder.CreateStore(sub_z, ret_z);

        return builder.CreateLoad(ret);
    }
    else if (left->getType() == float3_struct_ty && right->getType() == float_ty) {
        auto ret = builder.CreateAlloca(float3_struct_ty);

        builder.CreateStore(left, ret);

        auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
        auto sub_x = get_llvm_div(builder.CreateLoad(ret_x), right, context);
        builder.CreateStore(sub_x, ret_x);

        auto ret_y = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 1);
        auto sub_y = get_llvm_div(builder.CreateLoad(ret_y), right, context);
        builder.CreateStore(sub_y, ret_y);

        auto ret_z = context.builder->CreateConstGEP2_32(nullptr, ret, 0, 2);
        auto sub_z = get_llvm_div(builder.CreateLoad(ret_z), right, context);
        builder.CreateStore(sub_z, ret_z);

        return builder.CreateLoad(ret);
    }
    else if (left->getType() == float_ty && right->getType() == float3_struct_ty) {
        auto ret = builder.CreateAlloca(float3_struct_ty);

        auto tmp_right = builder.CreateAlloca(float3_struct_ty);
        builder.CreateStore(right, tmp_right);

        auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
        auto right_x = builder.CreateConstGEP2_32(nullptr, tmp_right, 0, 0);
        auto sub_x = get_llvm_div(left, builder.CreateLoad(right_x), context);
        builder.CreateStore(sub_x, ret_x);

        auto ret_y = builder.CreateConstGEP2_32(nullptr, ret, 0, 1);
        auto right_y = context.builder->CreateConstGEP2_32(nullptr, tmp_right, 0, 1);
        auto sub_y = get_llvm_div(left, builder.CreateLoad(right_y), context);
        builder.CreateStore(sub_y, ret_y);

        auto ret_z = builder.CreateConstGEP2_32(nullptr, ret, 0, 2);
        auto right_z = context.builder->CreateConstGEP2_32(nullptr, tmp_right, 0, 2);
        auto sub_z = get_llvm_div(left, builder.CreateLoad(right_z), context);
        builder.CreateStore(sub_z, ret_z);

        return builder.CreateLoad(ret);
    }

    return get_llvm_div(left, right, context);
}

llvm::Value* AstNode_Binary_Mod::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFRem(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateSRem(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_And::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    left = convert_to_bool(left, context);
    right = convert_to_bool(right, context);

    return context.builder->CreateAnd(left, right);
}

llvm::Value* AstNode_Binary_Or::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    left = convert_to_bool(left, context);
    right = convert_to_bool(right, context);

    return context.builder->CreateOr(left, right);
}

llvm::Value* AstNode_Binary_Eq::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOEQ(left, right);
    else
        return context.builder->CreateICmpEQ(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Ne::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpONE(left, right);
    else
        return context.builder->CreateICmpNE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_G::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOGT(left, right);
    else
        return context.builder->CreateICmpSGT(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_L::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOLT(left, right);
    else
        return context.builder->CreateICmpSLT(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Ge::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOGE(left, right);
    else
        return context.builder->CreateICmpSGE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Le::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOLE(left, right);
    else
        return context.builder->CreateICmpSLE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Shl::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateShl(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Shr::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateAShr(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_And::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateAnd(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_Or::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateOr(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_Xor::codegen(TSL_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateXor(left, right);

    return nullptr;
}

llvm::Value* AstNode_Statement_Return::codegen(TSL_Compile_Context& context) const {
    if (!m_expression)
        return context.builder->CreateRetVoid();

    auto ret_value = m_expression->codegen(context);
    return context.builder->CreateRet(ret_value);
}

llvm::Value* AstNode_VariableRef::codegen(TSL_Compile_Context& context) const {
    // just find it in the symbol table
    auto var = context.get_var_symbol(m_name);
    if (!var)
        return var;
    return context.builder->CreateLoad(var);
}

llvm::Value* AstNode_VariableRef::get_value_address(TSL_Compile_Context& context) const {
    return context.get_var_symbol(m_name);
}

bool AstNode_VariableRef::is_closure(TSL_Compile_Context& context) const {
    auto type = context.get_var_type(m_name);
    if (DataTypeEnum::CLOSURE == type.m_type)
        return true;

    return false;
}

DataType AstNode_VariableRef::get_var_type(TSL_Compile_Context& context) const {
	return context.get_var_type(m_name);
}

llvm::Value* AstNode_ArrayAccess::codegen(TSL_Compile_Context& context) const {
    auto var = m_var->get_value_address(context);
    auto index = m_index->codegen(context);
    
    if (!is_llvm_integer(index)) {
        emit_error("Array index has to be an integer.");
        return nullptr;
    }

    auto value_ptr = context.builder->CreateGEP(var, index);
    return context.builder->CreateLoad(value_ptr);
}

llvm::Value* AstNode_ArrayAccess::get_value_address(TSL_Compile_Context& context) const {
    auto var = m_var->get_value_address(context);
    auto index = m_index->codegen(context);

    if (!is_llvm_integer(index)) {
        emit_error("Array index has to be an integer.");
        return nullptr;
    }

    return context.builder->CreateGEP(var, index);
}

DataType AstNode_ArrayAccess::get_var_type(TSL_Compile_Context& context) const {
	return m_var->get_var_type(context);
}

llvm::Value* AstNode_SingleVariableDecl::codegen(TSL_Compile_Context& context) const {
    auto name = m_name;
    auto type = get_type_from_context(m_type, context);
    auto init = m_init_exp.get();

    if (nullptr != context.get_var_symbol(name, true)) {
        emit_error("Redefined variabled named '%s'.", name.c_str());
        return nullptr;
    }

    // allocate the variable on stack
    auto alloc_var = context.builder->CreateAlloca(type, nullptr, name);

    // initialize it if necessary
    if (init) {
        Value* init_value = init->codegen(context);

        if (init_value)
            context.builder->CreateStore(init_value, alloc_var);
    }

    context.push_var_symbol(name, alloc_var, m_type);

    return nullptr;
}

llvm::Value* AstNode_ArrayDecl::codegen(TSL_Compile_Context& context) const {
    auto name = m_name;

    if (nullptr != context.get_var_symbol(name, true)) {
        emit_error("Redefined variabled named '%s'.", name.c_str());
        return nullptr;
    }
    
    auto type = get_type_from_context(m_type, context);
    auto cnt = m_cnt->codegen(context);

    if (!is_llvm_integer(cnt)) {
        emit_error("Invalid type of array size, it has to be an integer.", name.c_str());
        return nullptr;
    }

    // allocate the variable on stack
    auto alloc_var = context.builder->CreateAlloca(type, cnt, name);

    context.push_var_symbol(name, alloc_var, m_type);

    // If there is initialize list, initialize it.
    // The way it is initialized here is terrible, it simply loops through all elements and assign them one by one.
    if (m_init) {
        const auto& init_list = m_init->get_init_list();
        
        // Ideally, I should check the number of elements in the initialize list and make them match.
        // But since cnt is dynamically resolved, the number of elements in the array is not decided until run-time.
        // Maybe I should disallow non-literal array count, it sounds like a reasonable solution.
        // For now, I will simply loop through everything, risking in out of memory access.
        auto i = 0;
        for (auto& var : init_list) {
            auto index = get_llvm_constant_int(i++, 32, context);
            auto element = context.builder->CreateGEP(alloc_var, index);
            context.builder->CreateStore(var->codegen(context), element);
        }
    }

    return nullptr;
}

llvm::Value* AstNode_Statement_VariableDecl::codegen(TSL_Compile_Context& context) const {
    m_var_decls->codegen(context);
    return nullptr;
}

llvm::Value* AstNode_ExpAssign_Eq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);
    context.builder->CreateStore(to_assign, value_ptr);
    return to_assign;
}

llvm::Value* AstNode_ExpAssign_AddEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_add(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_MinusEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_sub(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_MultiEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_mul(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_DivEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_div(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_ModEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_mod(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_AndEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'&=' is only value for integers.");
        return nullptr;
    }

    auto to_assign = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'&=' is only value for integers.");
        return nullptr;
    }

    if (to_assign->getType()->getIntegerBitWidth() != value->getType()->getIntegerBitWidth()) {
        // copy = ConstantInt::get(value->getType(), APInt(value->getType()->getIntegerBitWidth(), ))
        to_assign = context.builder->CreateCast(Instruction::Trunc, to_assign, value->getType());
    }

    auto updated_value = context.builder->CreateAnd(value, to_assign);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_OrEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'|=' is only value for integers.");
        return nullptr;
    }

    auto to_assign = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'|=' is only value for integers.");
        return nullptr;
    }

    if (to_assign->getType()->getIntegerBitWidth() != value->getType()->getIntegerBitWidth())
        to_assign = context.builder->CreateCast(Instruction::Trunc, to_assign, value->getType());

    auto updated_value = context.builder->CreateOr(value, to_assign);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_XorEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'^=' is only value for integers.");
        return nullptr;
    }

    auto to_assign = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'^=' is only value for integers.");
        return nullptr;
    }

    if (to_assign->getType()->getIntegerBitWidth() != value->getType()->getIntegerBitWidth())
        to_assign = context.builder->CreateCast(Instruction::Trunc, to_assign, value->getType());

    auto updated_value = context.builder->CreateXor(value, to_assign);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_ShlEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'<<=' is only value for integers.");
        return nullptr;
    }

    auto to_shift = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'<<=' is only value for integers.");
        return nullptr;
    }

    auto updated_value = context.builder->CreateShl(value, to_shift);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_ShrEq::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'>>=' is only value for integers.");
        return nullptr;
    }

    auto to_shift = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        emit_error("'>>=' is only value for integers.");
        return nullptr;
    }

    auto updated_value = context.builder->CreateAShr(value, to_shift);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_Statement_Expression::codegen(TSL_Compile_Context& context) const{
	return m_expression->codegen(context);
}

llvm::Value* AstNode_Expression_PreInc::codegen(TSL_Compile_Context& context) const{
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);
    
    if (value->getType()->isIntegerTy()) {
        auto bw = value->getType()->getIntegerBitWidth();
        auto one = ConstantInt::get(value->getType(), APInt(bw, 1));
        auto updated_value = context.builder->CreateAdd(value, one);
        context.builder->CreateStore(updated_value, value_ptr);
        return updated_value;
    }

    // something is wrong here, we must be using this ++x for other type of data.
    return value;
}

llvm::Value* AstNode_Expression_PreDec::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (value->getType()->isIntegerTy()) {
        auto bw = value->getType()->getIntegerBitWidth();
        auto one = ConstantInt::get(value->getType(), APInt(bw, 1));
        auto updated_value = context.builder->CreateSub(value, one);
        context.builder->CreateStore(updated_value, value_ptr);
        return updated_value;
    }

    // something is wrong here, we must be using this --x for other type of data.
    return value;
}

llvm::Value* AstNode_Expression_PostInc::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (value->getType()->isIntegerTy()) {
        auto bw = value->getType()->getIntegerBitWidth();
        auto one = ConstantInt::get(value->getType(), APInt(bw, 1));
        auto updated_value = context.builder->CreateAdd(value, one);
        context.builder->CreateStore(updated_value, value_ptr);
        return value;
    }

    // something is wrong here, we must be using this x++ for other type of data.
    return value;
}

llvm::Value* AstNode_Expression_PostDec::codegen(TSL_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (value->getType()->isIntegerTy()) {
        auto bw = value->getType()->getIntegerBitWidth();
        auto one = ConstantInt::get(value->getType(), APInt(bw, 1));
        auto updated_value = context.builder->CreateSub(value, one);
        context.builder->CreateStore(updated_value, value_ptr);
        return value;
    }

    // something is wrong here, we must be using this x-- for other type of data.
    return value;
}

llvm::Value* AstNode_Unary_Pos::codegen(TSL_Compile_Context& context) const {
	return m_exp->codegen(context);
}

llvm::Value* AstNode_Unary_Neg::codegen(TSL_Compile_Context& context) const{
    auto& builder = *context.builder;

	auto operand = m_exp->codegen(context);
    if (!operand)
        return nullptr;

    auto operand_type = operand->getType();
	if (operand_type == get_float_ty(context))
		return builder.CreateFNeg(operand);
	else if (operand_type == get_int_32_ty(context))
		return builder.CreateNeg(operand);
    else {
        auto it = context.m_structure_type_maps.find("float3");
        auto float3_struct_ty = it->second.m_llvm_type;
        if (operand_type == float3_struct_ty) {
            auto ret = builder.CreateAlloca(float3_struct_ty);
            builder.CreateStore(operand, ret);

            auto ret_x = builder.CreateConstGEP2_32(nullptr, ret, 0, 0);
            auto negated_x = context.builder->CreateFNeg(builder.CreateLoad(ret_x));
            builder.CreateStore(negated_x, ret_x);

            auto ret_y = builder.CreateConstGEP2_32(nullptr, ret, 0, 1);
            auto negated_y = context.builder->CreateFNeg(builder.CreateLoad(ret_y));
            builder.CreateStore(negated_y, ret_y);

            auto ret_z = builder.CreateConstGEP2_32(nullptr, ret, 0, 2);
            auto negated_z = context.builder->CreateFNeg(builder.CreateLoad(ret_z));
            builder.CreateStore(negated_z, ret_z);

            return builder.CreateLoad(ret);
        }
    }

	return nullptr;
}

llvm::Value* AstNode_Unary_Not::codegen(TSL_Compile_Context& context) const {
    auto& builder = *context.builder;

    auto operand = m_exp->codegen(context);

    // convert to bool if needed
    operand = convert_to_bool(operand, context);

    return builder.CreateNot(operand);
}

llvm::Value* AstNode_Unary_Compl::codegen(TSL_Compile_Context& context) const {
    auto operand = m_exp->codegen(context);

    if (!operand->getType()->isIntegerTy()) {
        emit_error("'~' is only value for integers.");
        return nullptr;
    }

    auto zero = get_llvm_constant_int(0, operand->getType()->getIntegerBitWidth(), context);
    return context.builder->CreateXor(zero, operand);
}

llvm::Value* AstNode_TypeCast::codegen(TSL_Compile_Context& context) const {
    auto value = m_exp->codegen(context);

    const auto target_ty = get_type_from_context(m_target_type, context);

    const auto float_ty = get_float_ty(context);
    const auto int_32_ty = get_int_32_ty(context);
    if (value->getType() == float_ty) {
        if (m_target_type.m_type == DataTypeEnum::INT)
            return context.builder->CreateFPToSI(value, target_ty);
        else if(m_target_type.m_type == DataTypeEnum::FLOAT)
            return value;

        emit_warning("Unsupported casting.");

    }
    else if (value->getType() == int_32_ty) {
        if (m_target_type.m_type == DataTypeEnum::INT)
            return value;
        else if (m_target_type.m_type == DataTypeEnum::FLOAT)
            return context.builder->CreateSIToFP(value, target_ty);

        emit_warning("Unsupported casting.");
    }

    emit_warning("Unsupported casting.");
    return value;
}

llvm::Value* AstNode_Expression_MakeClosure::codegen(TSL_Compile_Context& context) const {
    const auto function_name = "make_closure_" + m_name;

    if (context.m_closures_maps.count(m_name) == 0) {
        emit_error("Unregistered closure '%s'.", m_name.c_str());
        return nullptr;
    }
        
    // declare the function first.
    auto function = context.m_closures_maps[m_name];

    std::vector<Value*> args_llvm;
    if (m_args) {
        for (const auto& arg : m_args->get_arg_list())
            args_llvm.push_back(arg->codegen(context));
    }

    return context.builder->CreateCall(function, args_llvm);
}

llvm::Value* AstNode_FunctionCall::codegen(TSL_Compile_Context& context) const {
    auto it = context.m_func_symbols.find(m_name);
    if (it == context.m_func_symbols.end()) {
        emit_error( "Undefined function %s." , m_name.c_str() );
        return nullptr;
    }

    auto ast_func = it->second.second;

    static const std::vector<std::shared_ptr<const AstNode_SingleVariableDecl>> empty_var_decls;
    const auto& var_decls = ast_func->m_variables ? ast_func->m_variables->get_var_list() : empty_var_decls;
    std::vector<Value*> args_llvm;

    if (m_args) {
        const auto& args = m_args->get_arg_list();

        if (args.size() != var_decls.size()) {
            emit_error("Incorrect number of arguments passed in function %s", m_name.c_str());
        }

        for (auto i = 0u; i < args.size(); ++i) {
            if (var_decls[i]->get_config() & VariableConfig::OUTPUT) {
                const AstNode_Lvalue* lvalue = dynamic_cast<const AstNode_Lvalue*>(args[i].get());
                if (!lvalue) {
                    emit_error("Right value can't be used as an output argument.");
                    return nullptr;
                }
                args_llvm.push_back(lvalue->get_value_address(context));
            }
            else {
                args_llvm.push_back(args[i]->codegen(context));
            }
        }
    }
    else {
        if (var_decls.size()) {
            emit_error("Missing arguments in function call %s.", m_name.c_str());
            return nullptr;
        }
    }

    return context.builder->CreateCall(it->second.first, args_llvm);
}

llvm::Value* AstNode_Float3Constructor::codegen(TSL_Compile_Context& context) const {
    // if it is not a function, it could be a constructor of a structure.
    auto struct_ty = context.m_structure_type_maps.find("float3");
    if (struct_ty == context.m_structure_type_maps.end()) {
        emit_error("Fatal internal error, vector is not defined.");
        return nullptr;
    }

    const auto type = struct_ty->second.m_llvm_type;
    auto ret = context.builder->CreateAlloca(type);

    llvm::Value* last_arg = nullptr;

    if (!m_arguments) {
        const auto default_value = get_llvm_constant_fp(0.0f, context);
        for (auto i = 0u; i < 3u; ++i) {
            auto gep = context.builder->CreateConstGEP2_32(nullptr, ret, 0, i);
            context.builder->CreateStore(default_value, gep);
        }
    } else {
        const auto& args = m_arguments->get_arg_list();

        int i = 0;
        for (auto& arg : args) {
            if (i >= 3) {
                emit_warning("Too many arguments in vector constructor, the dummy ones will be ignored.");
                break;
            }

            auto value = arg->codegen(context);
            auto gep = context.builder->CreateConstGEP2_32(nullptr, ret, 0, i++);
            context.builder->CreateStore(value, gep);

            last_arg = gep;
        }

        assert(last_arg);

        while (i < 3) {
            auto gep = context.builder->CreateConstGEP2_32(nullptr, ret, 0, i++);
            context.builder->CreateStore(last_arg, gep);
        }
    }

    return context.builder->CreateLoad(ret);
}

llvm::Value* AstNode_Ternary::codegen(TSL_Compile_Context& context) const {
    auto cond = m_condition->codegen(context);
    auto true_exp = m_true_expr->codegen(context);
    auto false_exp = m_false_expr->codegen(context);

    // convert to bool if needed
    cond = convert_to_bool(cond, context);

    return context.builder->CreateSelect(cond, true_exp, false_exp);
}

llvm::Value* AstNode_Statement_Condition::codegen(TSL_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;
    
    auto cond = m_condition->codegen(context);

    // this should not happen, but it happens due to incomplete implementation.
    if (!cond)
        return nullptr;

    // convert to bool if needed
    cond = convert_to_bool(cond, context);

    auto function = context.builder->GetInsertBlock()->getParent();

    BasicBlock* then_bb = BasicBlock::Create(llvm_context, "then", function);
    BasicBlock* else_bb = m_false_statements ? BasicBlock::Create(llvm_context, "else") : nullptr;
    BasicBlock* merge_bb = BasicBlock::Create(llvm_context, "ifcont");

    if(else_bb)
        builder.CreateCondBr(cond, then_bb, else_bb);
    else
        builder.CreateCondBr(cond, then_bb, merge_bb);

    context.push_var_symbol_layer();

    builder.SetInsertPoint(then_bb);
    if(m_true_statements)
        m_true_statements->codegen(context);
    
    if (function->getBasicBlockList().back().getTerminator() == nullptr)
        builder.CreateBr(merge_bb);

    if (else_bb) {
        function->getBasicBlockList().push_back(else_bb);
        builder.SetInsertPoint(else_bb);

        if(m_false_statements)
            m_false_statements->codegen(context);

        if (function->getBasicBlockList().back().getTerminator() == nullptr)
            builder.CreateBr(merge_bb);
    }

    function->getBasicBlockList().push_back(merge_bb);
    builder.SetInsertPoint(merge_bb);

    context.pop_var_symbol_layer();

    return nullptr;
}

llvm::Value* AstNode_Statement_Loop_While::codegen(TSL_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;

    auto function = context.builder->GetInsertBlock()->getParent();
    BasicBlock* loop_begin_bb = BasicBlock::Create(llvm_context, "loop_begin", function);
    BasicBlock* loop_body_bb = BasicBlock::Create(llvm_context, "loop_body");
    BasicBlock* loop_end_bb = BasicBlock::Create(llvm_context, "loop_end");

    builder.CreateBr(loop_begin_bb);

    // push the loop blocks
    context.push_var_symbol_layer();
    context.m_blocks.push(std::make_pair(loop_begin_bb, loop_end_bb));

    builder.SetInsertPoint(loop_begin_bb);

    auto cond = m_condition->codegen(context);
    cond = convert_to_bool(cond, context);
    builder.CreateCondBr(cond, loop_body_bb, loop_end_bb);

    function->getBasicBlockList().push_back(loop_body_bb);
    builder.SetInsertPoint(loop_body_bb);

    if(m_statements)
        m_statements->codegen(context);
    builder.CreateBr(loop_begin_bb);

    function->getBasicBlockList().push_back(loop_end_bb);
    builder.SetInsertPoint(loop_end_bb);

    // pop the loop blocks
    context.m_blocks.pop();

    context.pop_var_symbol_layer();

    return nullptr;
}

llvm::Value* AstNode_Statement_Loop_DoWhile::codegen(TSL_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;

    auto function = context.builder->GetInsertBlock()->getParent();
    BasicBlock* loop_bb = BasicBlock::Create(llvm_context, "loop_block", function);
    BasicBlock* loop_end_bb = BasicBlock::Create(llvm_context, "loop_end");

    // push the loop blocks
    context.push_var_symbol_layer();
    context.m_blocks.push(std::make_pair(loop_bb, loop_end_bb));

    builder.CreateBr(loop_bb);

    builder.SetInsertPoint(loop_bb);

    if(m_statements)
        m_statements->codegen(context);

    auto cond = m_condition->codegen(context);
    cond = convert_to_bool(cond, context);
    builder.CreateCondBr(cond, loop_bb, loop_end_bb);

    function->getBasicBlockList().push_back(loop_end_bb);
    builder.SetInsertPoint(loop_end_bb);

    // pop the loop blocks
    context.m_blocks.pop();
    context.pop_var_symbol_layer();

    return nullptr;
}

llvm::Value* AstNode_Statement_Loop_For::codegen(TSL_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;

    auto function = context.builder->GetInsertBlock()->getParent();
    BasicBlock* loop_begin_bb = BasicBlock::Create(llvm_context, "for_loop_block_begin");
    BasicBlock* loop_body_bb = BasicBlock::Create(llvm_context, "for_loop_block_body");
    BasicBlock* loop_iter_bb = BasicBlock::Create(llvm_context, "for_loop_block_iter");
    BasicBlock* loop_end_bb = BasicBlock::Create(llvm_context, "for_loop_end");

    auto condition_branch = [&]() {
        if (m_condition) {
            auto cond = m_condition->codegen(context);
            cond = convert_to_bool(cond, context);
            builder.CreateCondBr(cond, loop_body_bb, loop_end_bb);
        }
        else {
            // unlike while loop, for loop does allow no condition cases.
            builder.CreateBr(loop_body_bb);
        }
    };

    if (m_init_exp)
        m_init_exp->codegen(context);
    builder.CreateBr(loop_begin_bb);

    // push the loop blocks
    context.push_var_symbol_layer();
    context.m_blocks.push(std::make_pair(loop_iter_bb, loop_end_bb));

    // the for loop begins from the condition blocks
    function->getBasicBlockList().push_back(loop_begin_bb);
    builder.SetInsertPoint(loop_begin_bb);
    condition_branch();

    // here is the body of the loop
    function->getBasicBlockList().push_back(loop_body_bb);
    builder.SetInsertPoint(loop_body_bb);

    if(m_statements)
        m_statements->codegen(context);

    builder.CreateBr(loop_iter_bb);

    // the iteration block
    function->getBasicBlockList().push_back(loop_iter_bb);
    builder.SetInsertPoint(loop_iter_bb);
    if(m_iter_exp)
        m_iter_exp->codegen(context);
    builder.CreateBr(loop_begin_bb);

    // the next block
    function->getBasicBlockList().push_back(loop_end_bb);
    builder.SetInsertPoint(loop_end_bb);

    // pop the loop blocks
    context.m_blocks.pop();
    context.pop_var_symbol_layer();

    return nullptr;
}

llvm::Value* AstNode_ScoppedStatement::codegen(TSL_Compile_Context& context) const {
    context.push_var_symbol_layer();
    
    if(m_statement)
        m_statement->codegen(context);

    context.pop_var_symbol_layer();
    return nullptr;
}

void AstNode_CompoundStatements::append_statement(AstNode_Statement* statement) {
    auto ptr = ast_ptr_from_raw<AstNode_Statement>(statement);
    m_statements.push_back(ptr);
}

llvm::Value* AstNode_CompoundStatements::codegen(TSL_Compile_Context& context) const {
    for (const auto& statement : m_statements)
        statement->codegen(context);
    return nullptr;
}

llvm::Value* AstNode_Statement_Break::codegen(TSL_Compile_Context& context) const {
    BasicBlock* bb = BasicBlock::Create(*context.context, "next_block");

    auto top = context.m_blocks.top();
    context.builder->CreateBr(top.second);

    auto function = context.builder->GetInsertBlock()->getParent();
    function->getBasicBlockList().push_back(bb);
    context.builder->SetInsertPoint(bb);

    return nullptr;
}

llvm::Value* AstNode_Statement_Continue::codegen(TSL_Compile_Context& context) const {
    BasicBlock* bb = BasicBlock::Create(*context.context, "next_block");

    auto top = context.m_blocks.top();
    context.builder->CreateBr(top.first);

    auto function = context.builder->GetInsertBlock()->getParent();
    function->getBasicBlockList().push_back(bb);
    context.builder->SetInsertPoint(bb);

    return nullptr;
}

llvm::Value* AstNode_StructDeclaration::codegen(TSL_Compile_Context& context) const {
	if( context.m_structure_type_maps.count(m_name) )
		return nullptr;

	std::vector<llvm::Type*> member_types;

	if(m_members){
		const auto& members = m_members->get_member_list();

		for( const auto& member : members ){
            const auto decl = member->get_variable_decl();
			const auto type = decl->data_type();
			auto llvm_type = get_type_from_context(type, context);
			member_types.push_back(llvm_type);
		}
	}
	auto structure_type = StructType::create(member_types, m_name);

	auto& item = context.m_structure_type_maps[m_name];
	item.m_llvm_type = structure_type;

    bool type_pushed = false;
	int i = 0;
	if (m_members) {
        const auto& members = m_members->get_member_list();

        for (const auto& member : members) {
            const auto decl = member->get_variable_decl();
            const auto name = decl->get_var_name();
            const auto type = decl->data_type();

            item.m_member_types[name] = std::make_pair(i++, type);
            type_pushed = true;
        }
	}

	return nullptr;
}

llvm::Value* AstNode_StructMemberRef::codegen(TSL_Compile_Context& context) const{
	auto value_ptr = get_value_address(context);
	return context.builder->CreateLoad(value_ptr);
}

llvm::Value* AstNode_StructMemberRef::get_value_address(TSL_Compile_Context& context) const{
	const auto var_type = m_var->get_var_type(context);

	if( !context.m_structure_type_maps.count(var_type.m_structure_name) ){
        emit_error("Undefined struct '%s'.", var_type.m_structure_name);
		return nullptr;
	}

	const auto var_value_ptr = m_var->get_value_address(context);
	if( !var_value_ptr )
		return nullptr;

	// access the member meta data
	auto& data_type = context.m_structure_type_maps[var_type.m_structure_name];

	// get the member offset
	auto it = data_type.m_member_types.find(m_member);
	if( it == data_type.m_member_types.end() ){
        emit_error("Undefined member variable '%s' in struct '%s'.", m_member.c_str(), var_type.m_structure_name);
		return nullptr;
	}

	// get the member address
	auto member_value_ptr = context.builder->CreateConstGEP2_32(nullptr, var_value_ptr, 0, it->second.first);
	return  member_value_ptr;
}

DataType AstNode_StructMemberRef::get_var_type(TSL_Compile_Context& context) const{
	const auto var_type = m_var->get_var_type(context);

	if (!context.m_structure_type_maps.count(var_type.m_structure_name)) {
        emit_error("Undefined struct '%s'.", var_type.m_structure_name);
		return DataType();
	}

	const auto var_value_ptr = m_var->get_value_address(context);
	if (!var_value_ptr) 
		return DataType();

	// access the member meta data
	auto& data_type = context.m_structure_type_maps[var_type.m_structure_name];

	// get the member offset
	auto it = data_type.m_member_types.find(m_member);
	if (it == data_type.m_member_types.end()) {
        emit_error("Undefined member variable '%s' in struct '%s'.", m_member.c_str(), var_type.m_structure_name);
		return DataType();
	}

	return it->second.second;
}

llvm::Value* AstNode_Statement_TextureDeclaration::codegen(TSL_Compile_Context& context) const {
    // find the registered texture
    auto it = context.m_shader_resource_table->find(m_handle_name);
    if (it == context.m_shader_resource_table->end()){
        emit_error("Texture handle (%s) not registered.", m_handle_name.c_str());
        return nullptr;
    }
    
    auto addr = it->second;
    auto input_addr = ConstantInt::get(Type::getInt64Ty(*context.context), uintptr_t(addr));
    auto ptr_input_addr = ConstantExpr::getIntToPtr(input_addr, get_int_32_ptr_ty(context));

    // create the global variable
    llvm::Module* module = context.module;
    GlobalVariable* global_input_value = new GlobalVariable(*module, get_int_32_ptr_ty(context), true, GlobalValue::InternalLinkage, ptr_input_addr, "global_input");

    // for debugging purposes
    global_input_value->setName(m_handle_name);

    // push the varible, this one is kind of special in a way it doesn't have a valid data type.
    context.push_var_symbol(m_handle_name, global_input_value, DataType());

    return nullptr;
}

llvm::Value* AstNode_Statement_ShaderResourceHandleDeclaration::codegen(TSL_Compile_Context& context) const {
    // find the registered texture
    auto it = context.m_shader_resource_table->find(m_handle_name);
    if (it == context.m_shader_resource_table->end()) {
        emit_error("Resource handle (%s) not registered %s.", m_handle_name.c_str());
        return nullptr;
    }

    auto addr = it->second;
    auto input_addr = ConstantInt::get(Type::getInt64Ty(*context.context), uintptr_t(addr));
    auto ptr_input_addr = ConstantExpr::getIntToPtr(input_addr, get_int_32_ptr_ty(context));

    // create the global variable
    llvm::Module* module = context.module;
    GlobalVariable* global_input_value = new GlobalVariable(*module, get_int_32_ptr_ty(context), true, GlobalValue::InternalLinkage, ptr_input_addr, "global_input");

    // for debugging purposes
    global_input_value->setName(m_handle_name);

    // push the varible, this one is kind of special in a way it doesn't have a valid data type.
    context.push_var_symbol(m_handle_name, global_input_value, DataType());

    return nullptr;
}

llvm::Value* AstNode_Expression_Texture2DSample::codegen(TSL_Compile_Context& context) const {
    auto texture_handle_addr = context.get_var_symbol(m_texture_handle_name);
    if (texture_handle_addr) {
        auto texture_handle = context.builder->CreateLoad(texture_handle_addr);
        auto th = context.builder->CreatePointerCast(texture_handle, get_int_32_ptr_ty(context));

        if (!m_sample_alpha) {
            auto texture2d_sample_function = context.m_func_symbols["TSL_TEXTURE2D_SAMPLE"].first;

            std::vector<Value*> args(1, th);

            auto float3_struct_ty = context.m_structure_type_maps["float3"].m_llvm_type;
            Value* ret = context.builder->CreateAlloca(float3_struct_ty);
            args.push_back(ret);

            if (m_arguments) {
                const auto& func_args = m_arguments->get_arg_list();
                for (const auto& arg : func_args)
                    args.push_back(arg->codegen(context));
            }
            else {
                emit_error("Texture2d sampling has not argument passed in.");
                return nullptr;
            }

            context.builder->CreateCall(texture2d_sample_function, args);

            return context.builder->CreateLoad(ret);
        }
        else {
            auto texture2d_sample_function = context.m_func_symbols["TSL_TEXTURE2D_SAMPLE_ALPHA"].first;

            std::vector<Value*> args(1, th);

            Value* ret = context.builder->CreateAlloca(get_float_ty(context));
            args.push_back(ret);

            if (m_arguments) {
                const auto& func_args = m_arguments->get_arg_list();
                for (const auto& arg : func_args)
                    args.push_back(arg->codegen(context));
            }
            else {
                emit_error("Texture2d sampling has not argument passed in.");
                return nullptr;
            }

            context.builder->CreateCall(texture2d_sample_function, args);

            return context.builder->CreateLoad(ret);
        }
    }

    emit_error("Texture handle %s not registered.", m_texture_handle_name.c_str());
    return nullptr;
}

llvm::Value* AstNode_SingleGlobalVariableDecl::codegen(TSL_Compile_Context& context) const {
    auto name = m_name;
    auto type = get_type_from_context(m_type, context);
    auto init = m_init_exp.get();

    if (nullptr != context.get_var_symbol(name, true)) {
        emit_error("Redefined variabled named '%s'.", name.c_str());
        return nullptr;
    }

    llvm::Constant* llvm_init = nullptr;
    if (init) {
        auto literal_init = dynamic_cast<const AstNode_Literal*>(init);

        if (nullptr == literal_init)
            emit_warning("Global variable can only be initialized with a constant variable. The initialization of %s will be ignored.");
        else {
            auto llvm_var = literal_init->codegen(context);
            llvm_init = (llvm::Constant*)(llvm_var);
        }
    }

    auto global_var = new GlobalVariable(*context.module, type, true, GlobalValue::InternalLinkage, llvm_init, name.c_str());

    context.push_var_symbol(name, global_var, m_type);

    return nullptr;
}

llvm::Value* AstNode_GlobalArrayDecl::codegen(TSL_Compile_Context& context) const {
    auto name = m_name;

    if (nullptr != context.get_var_symbol(name, true)) {
        emit_error("Redefined variabled named '%s'.", name.c_str());
        return nullptr;
    }

    auto type = get_type_from_context(m_type, context);
    auto cnt = m_cnt->codegen(context);

    if (!is_llvm_integer(cnt)) {
        emit_error("Invalid type of array size, it has to be an integer.", name.c_str());
        return nullptr;
    }

    auto array_cnt = dynamic_cast<const AstNode_Literal_Int*>(m_cnt.get());
    if (array_cnt == nullptr) {
        emit_error("Invalid array count for %s", m_name.c_str());
        return nullptr;
    }

    ArrayType* array_type = nullptr;
    llvm::Constant* llvm_init = nullptr;
    if (m_init) {
        const auto& init_list = m_init->get_init_list();

        if (m_type.m_type == DataTypeEnum::INT) {
            auto i = 0;
            std::vector<int> vec;
            for (auto& var : init_list) {
                if (vec.size() >= array_cnt->m_val) {
                    emit_warning("Too many elements in the array initializer, the extras will be ignored.");
                    break;
                }
                auto index = get_llvm_constant_int(i++, 32, context);
                auto literal_var = dynamic_cast<const AstNode_Literal_Int*>(var.get());
                vec.push_back(literal_var->m_val);
            }
            while (vec.size() < array_cnt->m_val)
                vec.push_back(0);

            llvm_init = ConstantDataArray::get(*context.context, *(new ArrayRef<int>(vec)));
            array_type = ArrayType::get(get_int_32_ty(context), array_cnt->m_val);

            GlobalVariable* v = new GlobalVariable(*context.module, array_type, true, GlobalValue::InternalLinkage, llvm_init, m_name.c_str());
            Value* cast_v = context.builder->CreatePointerCast(v, get_int_32_ptr_ty(context));
            context.push_var_symbol(name, cast_v, m_type);
        }
        else if (m_type.m_type == DataTypeEnum::FLOAT) {
            auto i = 0;
            std::vector<float> vec;
            for (auto& var : init_list) {
                if (vec.size() >= array_cnt->m_val) {
                    emit_warning("Too many elements in the array initializer, the extras will be ignored.");
                    break;
                }
                auto index = get_llvm_constant_int(i++, 32, context);
                auto literal_var = dynamic_cast<const AstNode_Literal_Flt*>(var.get());
                vec.push_back(literal_var->m_val);
            }
            while (vec.size() < array_cnt->m_val)
                vec.push_back(0.0f);

            llvm_init = ConstantDataArray::get(*context.context, *(new ArrayRef<float>(vec)));
            array_type = ArrayType::get(get_float_ty(context), array_cnt->m_val);

            GlobalVariable* v = new GlobalVariable(*context.module, array_type, true, GlobalValue::InternalLinkage, llvm_init, m_name.c_str());
            Value* cast_v = context.builder->CreatePointerCast(v, get_float_ptr_ty(context));
            context.push_var_symbol(name, cast_v, m_type);
        }
    }

    return nullptr;
}

TSL_NAMESPACE_END
