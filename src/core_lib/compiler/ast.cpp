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
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/LLVMContext.h"
#include "ast.h"
#include "llvm_util.h"
#include "global_module.h"

using namespace llvm;

TSL_NAMESPACE_BEGIN

void LLVM_Compile_Context::reset() {
    m_var_symbols.clear();
    m_var_symbols.push_back({});    // this is for global variables
}

llvm::Value* LLVM_Compile_Context::get_var_symbol(const std::string& name, bool only_top_layer) {
    if (only_top_layer) {
        auto top = m_var_symbols.back();
        auto it = top.find(name);
        return it == top.end() ? nullptr : it->second;
    } else {
        auto it = m_var_symbols.rbegin();
        while (it != m_var_symbols.rend()) {
            auto var = it->find(name);
            if (var != it->end())
                return var->second;
            ++it;
        }
    }

    // emit error here, undefined symbol
    return nullptr;
}

llvm::Value* LLVM_Compile_Context::push_var_symbol(const std::string& name, llvm::Value* value) {
    auto top_layer = m_var_symbols.back();

    if (top_layer.count(name)) {
        // emit error here, variabled already defined.
        return nullptr;
    }

    m_var_symbols.back()[name] = value;

    // emit error here, undefined symbol
    return nullptr;
}

void LLVM_Compile_Context::push_var_symbol_layer() {
    m_var_symbols.push_back({});
}

void LLVM_Compile_Context::pop_var_symbol_layer() {
    m_var_symbols.pop_back();
}

llvm::Function* AstNode_FunctionPrototype::codegen( LLVM_Compile_Context& context ) const {
	int arg_cnt = 0;
	AstNode_VariableDecl* variable = m_variables;
	while( variable ){
		++arg_cnt;
		variable = castType<AstNode_VariableDecl>(variable->get_sibling());
	}

    // clear the symbol maps, no global var for now
    context.push_var_symbol_layer();

	// parse argument types
	std::vector<llvm::Type*>	args(arg_cnt);
	variable = m_variables;
	int i = 0;
	while( variable ){
        auto raw_type = get_type_from_context(variable->data_type(), context);
        args[i++] = (variable->get_config() & VariableConfig::OUTPUT) ? raw_type->getPointerTo() : raw_type;
		variable = castType<AstNode_VariableDecl>(variable->get_sibling());
	}

	// parse return types
	auto return_type = get_type_from_context(m_return_type , context);

	// declare the function prototype
	llvm::FunctionType *function_type = llvm::FunctionType::get(return_type, args, false);

	if( !function_type )
		return nullptr;

	// create the function prototype
	llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, m_name, context.module);

	// For debugging purposes, set the name of all arguments
	variable = m_variables;
	for (auto &arg : function->args()) {
		arg.setName(variable->get_var_name());
		variable = castType<AstNode_VariableDecl>(variable->get_sibling());
	}

    // no function overloading for simplicity, at least for now.
    if (context.m_func_symbols.count(m_name) != 0) {
        std::cout << "Duplicated function named : " << m_name << std::endl;
        function->eraseFromParent();
        return nullptr;
    }

    context.m_func_symbols[m_name] = std::make_pair(function, this);

    if( m_body ){
        // create a separate code block
	    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context.context, "entry", function);
	    context.builder->SetInsertPoint(BB);

        // push the argument into the symbol table first
        int i = 0;
        variable = m_variables;
        while (variable) {
            const auto name = variable->get_var_name();
            const auto raw_type = get_type_from_context(variable->data_type(), context);
            const auto init = variable->get_init();

            // there is duplicated names, emit an warning!!
            // However, there is no log system in the compiler for now, I need to handle this later.
            if( nullptr != context.get_var_symbol(name, true) ){
                std::cout << "Duplicated argument named : " << name << std::endl;
                return nullptr;
            }

            auto arg = function->getArg(i);
            if (variable->get_config() & VariableConfig::OUTPUT) {
                context.push_var_symbol(name, arg);
            } else {
                // allocate the variable on stack
                auto alloc_var = context.builder->CreateAlloca(raw_type, nullptr, name);

                // duplicate the value so that it can be a copy instead of a reference.
                context.builder->CreateStore(arg, alloc_var);
                
                context.push_var_symbol(name, alloc_var);
            }

            variable = castType<AstNode_VariableDecl>(variable->get_sibling());
            ++i;
        }

        auto statement = m_body->m_statements;
        while (statement) {
            statement->codegen(context);
            statement = (AstNode_Statement*)statement->get_sibling();
        }

        auto& last_block = function->getBasicBlockList().back();
        if (nullptr == last_block.getTerminator())
            context.builder->CreateRetVoid();
    }

    context.pop_var_symbol_layer();

	return function;
}

llvm::Value* AstNode_FunctionBody::codegen( LLVM_Compile_Context& context ) const{    
	return nullptr;
}

llvm::Value* AstNode_Literal_Int::codegen(LLVM_Compile_Context& context) const {
    return get_llvm_constant_int(m_val, 32, context);
}

llvm::Value* AstNode_Literal_Flt::codegen(LLVM_Compile_Context& context) const {
    return get_llvm_constant_fp(m_val, context);
}

llvm::Value* AstNode_Literal_Bool::codegen(LLVM_Compile_Context& context) const {
    return get_llvm_constant_int((int)m_val, 1, context);
}

llvm::Value* AstNode_Binary_Add::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);
    return get_llvm_add(left, right, context);
}

llvm::Value* AstNode_Binary_Minus::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);
    return get_llvm_sub(left, right, context);
}

llvm::Value* AstNode_Binary_Multi::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

	if(!m_left->is_closure() && !m_right->is_closure())
		return get_llvm_mul(left, right, context);

	// this must be a closure multiplied by a regular expression
	if(m_left->is_closure() && m_right->is_closure()){
		// emit an error, it is illegal to multiply two closures together.
		return nullptr;
	}

	// auto& llvm_context = *context.context;
	auto  module = context.module;
	auto& builder = *context.builder;

	auto closure = m_left->is_closure() ? left : right;
	auto weight = m_left->is_closure() ? right : left;

	auto malloc_function = context.m_func_symbols["malloc"].first;
	if( !malloc_function ){
		// this should not happen at all
		return nullptr;
	}

	const auto closure_tree_node_type = context.m_closure_type_maps["closure_mul"];
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

	/*
	// assign the closure parameter pointer
	auto gep1 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 2);
	auto weight_ptr = builder.CreatePointerCast(gep1, get_float_ptr_ty(context));
	builder.CreateStore(weight, weight_ptr);

	// copy the pointer

	auto gep2 = builder.CreateConstGEP2_32(nullptr, converted_closure_tree_node_ptr, 0, 3);
	auto closure_ptr = builder.CreatePointerCast(gep2, closure->getType()->getPointerTo());
	builder.CreateStore(closure, closure_ptr);
	*/
	return converted_closure_tree_node_ptr;
}

llvm::Value* AstNode_Binary_Div::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);
    return get_llvm_div(left, right, context);
}

llvm::Value* AstNode_Binary_Mod::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFRem(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateSRem(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_And::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    left = convert_to_bool(left, context);
    right = convert_to_bool(right, context);

    return context.builder->CreateAnd(left, right);
}

llvm::Value* AstNode_Binary_Or::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    left = convert_to_bool(left, context);
    right = convert_to_bool(right, context);

    return context.builder->CreateOr(left, right);
}

llvm::Value* AstNode_Binary_Eq::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOEQ(left, right);
    else
        return context.builder->CreateICmpEQ(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Ne::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpONE(left, right);
    else
        return context.builder->CreateICmpNE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_G::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOGT(left, right);
    else
        return context.builder->CreateICmpSGT(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_L::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOLT(left, right);
    else
        return context.builder->CreateICmpSLT(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Ge::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOGE(left, right);
    else
        return context.builder->CreateICmpSGE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Le::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == get_float_ty(context))
        return context.builder->CreateFCmpOLE(left, right);
    else
        return context.builder->CreateICmpSLE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Shl::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateShl(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Shr::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateAShr(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_And::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateAnd(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_Or::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateOr(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_Xor::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType()->isIntegerTy() && right->getType()->isIntegerTy())
        return context.builder->CreateXor(left, right);

    return nullptr;
}

llvm::Value* AstNode_Statement_Return::codegen(LLVM_Compile_Context& context) const {
    if (!m_expression)
        return context.builder->CreateRetVoid();

    auto ret_value = m_expression->codegen(context);
    return context.builder->CreateRet(ret_value);
}

llvm::Value* AstNode_VariableRef::codegen(LLVM_Compile_Context& context) const {
    // just find it in the symbol table
    auto var = context.get_var_symbol(m_name);
    if (!var)
        return var;
    return context.builder->CreateLoad(var);
}

llvm::Value* AstNode_VariableRef::get_value_address(LLVM_Compile_Context& context) const {
    return context.get_var_symbol(m_name);
}

llvm::Value* AstNode_ArrayAccess::codegen(LLVM_Compile_Context& context) const {
    auto var = m_var->get_value_address(context);
    auto index = m_index->codegen(context);
    
    if (!is_llvm_integer(index)) {
        // emit an error
        return nullptr;
    }

    auto value_ptr = context.builder->CreateGEP(var, index);
    return context.builder->CreateLoad(value_ptr);
}

llvm::Value* AstNode_ArrayAccess::get_value_address(LLVM_Compile_Context& context) const {
    auto var = m_var->get_value_address(context);
    auto index = m_index->codegen(context);

    if (!is_llvm_integer(index)) {
        // emit an error
        return nullptr;
    }

    return context.builder->CreateGEP(var, index);
}

llvm::Value* AstNode_SingleVariableDecl::codegen(LLVM_Compile_Context& context) const {
    auto name = m_name;
    auto type = get_type_from_context(m_type, context);
    auto init = m_init_exp;

    // there is duplicated names, emit an warning!!
    // However, there is no log system in the compiler for now, I need to handle this later.
    if (nullptr != context.get_var_symbol(name, true)) {
        std::cout << "Duplicated argument named : " << name << std::endl;
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

    context.push_var_symbol(name, alloc_var);

    return nullptr;
}

llvm::Value* AstNode_ArrayDecl::codegen(LLVM_Compile_Context& context) const {
    auto name = m_name;

    // there is duplicated names, emit an warning!!
    // However, there is no log system in the compiler for now, I need to handle this later.
    if (nullptr != context.get_var_symbol(name, true)) {
        std::cout << "Duplicated argument named : " << name << std::endl;
        return nullptr;
    }
    
    auto type = get_type_from_context(m_type, context);
    auto cnt = m_cnt->codegen(context);

    if (!is_llvm_integer(cnt)) {
        // emit error here, invalid array size
        return nullptr;
    }

    // allocate the variable on stack
    auto alloc_var = context.builder->CreateAlloca(type, cnt, name);

    context.push_var_symbol(name, alloc_var);

    return nullptr;
}

llvm::Value* AstNode_Statement_VariableDecls::codegen(LLVM_Compile_Context& context) const {
    AstNode_VariableDecl* decl = m_var_decls;
    while (decl) {
        decl->codegen(context);
        decl = castType<AstNode_VariableDecl>(decl->get_sibling());
    }

    return nullptr;
}

llvm::Value* AstNode_ExpAssign_Eq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);
    context.builder->CreateStore(to_assign, value_ptr);
    return to_assign;
}

llvm::Value* AstNode_ExpAssign_AddEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_add(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_MinusEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_sub(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_MultiEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_mul(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_DivEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_div(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_ModEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto to_assign = m_expression->codegen(context);

    auto value = context.builder->CreateLoad(value_ptr);
    auto updated_value = get_llvm_mod(value, to_assign, context);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_AndEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, this is not allowed
        return nullptr;
    }

    auto to_assign = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, incorrect data type
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

llvm::Value* AstNode_ExpAssign_OrEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, this is not allowed
        return nullptr;
    }

    auto to_assign = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, incorrect data type
        return nullptr;
    }

    if (to_assign->getType()->getIntegerBitWidth() != value->getType()->getIntegerBitWidth()) {
        // copy = ConstantInt::get(value->getType(), APInt(value->getType()->getIntegerBitWidth(), ))
        to_assign = context.builder->CreateCast(Instruction::Trunc, to_assign, value->getType());
    }

    auto updated_value = context.builder->CreateOr(value, to_assign);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_XorEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, this is not allowed
        return nullptr;
    }

    auto to_assign = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, incorrect data type
        return nullptr;
    }

    if (to_assign->getType()->getIntegerBitWidth() != value->getType()->getIntegerBitWidth()) {
        // copy = ConstantInt::get(value->getType(), APInt(value->getType()->getIntegerBitWidth(), ))
        to_assign = context.builder->CreateCast(Instruction::Trunc, to_assign, value->getType());
    }

    auto updated_value = context.builder->CreateXor(value, to_assign);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_ShlEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, this is not allowed
        return nullptr;
    }

    auto to_shift = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, incorrect data type
        return nullptr;
    }

    auto updated_value = context.builder->CreateShl(value, to_shift);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_ExpAssign_ShrEq::codegen(LLVM_Compile_Context& context) const {
    auto value_ptr = m_var->get_value_address(context);
    auto value = context.builder->CreateLoad(value_ptr);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, this is not allowed
        return nullptr;
    }

    auto to_shift = m_expression->codegen(context);

    if (!value->getType()->isIntegerTy()) {
        // emit an error here, incorrect data type
        return nullptr;
    }

    auto updated_value = context.builder->CreateAShr(value, to_shift);
    context.builder->CreateStore(updated_value, value_ptr);

    return updated_value;
}

llvm::Value* AstNode_Statement_CompoundExpression::codegen(LLVM_Compile_Context& context) const{
	return m_expression->codegen(context);
}

llvm::Value* AstNode_Expression_PreInc::codegen(LLVM_Compile_Context& context) const{
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

llvm::Value* AstNode_Expression_PreDec::codegen(LLVM_Compile_Context& context) const {
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

llvm::Value* AstNode_Expression_PostInc::codegen(LLVM_Compile_Context& context) const {
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

llvm::Value* AstNode_Expression_PostDec::codegen(LLVM_Compile_Context& context) const {
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

llvm::Value* AstNode_Unary_Pos::codegen(LLVM_Compile_Context& context) const {
	return m_exp->codegen(context);
}

llvm::Value* AstNode_Unary_Neg::codegen(LLVM_Compile_Context& context) const{
	auto operand = m_exp->codegen(context);

	if (operand->getType() == get_float_ty(context))
		return context.builder->CreateFNeg(operand);
	if (operand->getType() == get_int_32_ty(context))
		return context.builder->CreateNeg(operand);

	return nullptr;
}

llvm::Value* AstNode_Unary_Not::codegen(LLVM_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;

    auto operand = m_exp->codegen(context);

    // convert to bool if needed
    operand = convert_to_bool(operand, context);

    return builder.CreateNot(operand);
}

llvm::Value* AstNode_Unary_Compl::codegen(LLVM_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;

    auto operand = m_exp->codegen(context);

    if (!operand->getType()->isIntegerTy()) {
        // emit an error here
        return nullptr;
    }

    auto zero = get_llvm_constant_int(0, operand->getType()->getIntegerBitWidth(), context);
    return builder.CreateXor(zero, operand);
}

llvm::Value* AstNode_TypeCast::codegen(LLVM_Compile_Context& context) const {
    // to be implemented.
    auto value = m_exp->codegen(context);
    return value;
}

llvm::Value* AstNode_Expression_MakeClosure::codegen(LLVM_Compile_Context& context) const {
    const auto function_name = "make_closure_" + m_name;

    if (context.m_closures_maps.count(m_name) == 0) {
        // emit an error here, closure not registered
        return nullptr;
    }
        
    // declare the function first.
    auto function = context.m_closures_maps[m_name];

    std::vector<Value*> args;
    AstNode_Expression* node = m_args;
    while (node) {
        args.push_back(node->codegen(context));
        node = castType<AstNode_Expression>(node->get_sibling());
    }

    return context.builder->CreateCall(function, args);
}

llvm::Value* AstNode_FunctionCall::codegen(LLVM_Compile_Context& context) const {
    auto it = context.m_func_symbols.find(m_name);
    if (it == context.m_func_symbols.end()) {
        std::cout << "Undefined function " << m_name << "." << std::endl;
        return nullptr;
    }

    auto ast_func = it->second.second;

    std::vector<Value*> args;
    AstNode_Expression* node = m_variables;
    AstNode_VariableDecl* var_decl = ast_func->m_variables;
    while (node) {
        if (var_decl->get_config() & VariableConfig::OUTPUT) {
            // this node must be a LValue
            AstNode_Lvalue* lvalue = dynamic_cast<AstNode_Lvalue*>(node);
            if (!lvalue) {
                // emit an error here, using a r-value for output
                return nullptr;
            }
            args.push_back(lvalue->get_value_address(context));
        } else {
            args.push_back(node->codegen(context));
        }
        node = castType<AstNode_Expression>(node->get_sibling());
        var_decl = castType< AstNode_VariableDecl>(var_decl->get_sibling());
    }

    return context.builder->CreateCall(it->second.first, args);
}

llvm::Value* AstNode_Ternary::codegen(LLVM_Compile_Context& context) const {
    auto cond = m_condition->codegen(context);
    auto true_exp = m_true_expr->codegen(context);
    auto false_exp = m_false_expr->codegen(context);

    // convert to bool if needed
    cond = convert_to_bool(cond, context);

    return context.builder->CreateSelect(cond, true_exp, false_exp);
}

llvm::Value* AstNode_Statement_Condition::codegen(LLVM_Compile_Context& context) const {
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
    auto statement = m_true_statements;
    while (statement) {
        statement->codegen(context);
        statement = castType<AstNode_Statement>(statement->get_sibling());
    }
    
    if (function->getBasicBlockList().back().getTerminator() == nullptr)
        builder.CreateBr(merge_bb);

    if (else_bb) {
        function->getBasicBlockList().push_back(else_bb);
        builder.SetInsertPoint(else_bb);

        auto statement = m_false_statements;
        while (statement) {
            statement->codegen(context);
            statement = castType<AstNode_Statement>(statement->get_sibling());
        }

        if (function->getBasicBlockList().back().getTerminator() == nullptr)
            builder.CreateBr(merge_bb);
    }

    function->getBasicBlockList().push_back(merge_bb);
    builder.SetInsertPoint(merge_bb);

    context.pop_var_symbol_layer();

    return nullptr;
}

llvm::Value* AstNode_Statement_Loop_While::codegen(LLVM_Compile_Context& context) const {
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

    auto statement = m_statements;
    while (statement) {
        statement->codegen(context);
        statement = castType<AstNode_Statement>(statement->get_sibling());
    }
    builder.CreateBr(loop_begin_bb);

    function->getBasicBlockList().push_back(loop_end_bb);
    builder.SetInsertPoint(loop_end_bb);

    // pop the loop blocks
    context.m_blocks.pop();

    context.pop_var_symbol_layer();

    return nullptr;
}

llvm::Value* AstNode_Statement_Loop_DoWhile::codegen(LLVM_Compile_Context& context) const {
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
    auto statement = m_statements;
    while (statement) {
        statement->codegen(context);
        statement = castType<AstNode_Statement>(statement->get_sibling());
    }

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

llvm::Value* AstNode_Statement_Loop_For::codegen(LLVM_Compile_Context& context) const {
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
    auto statement = m_statements;
    while (statement) {
        statement->codegen(context);
        statement = castType<AstNode_Statement>(statement->get_sibling());
    }
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

llvm::Value* AstNode_ScoppedStatement::codegen(LLVM_Compile_Context& context) const {
    context.push_var_symbol_layer();
    
    if(m_statement)
        m_statement->codegen(context);

    context.pop_var_symbol_layer();
    return nullptr;
}

void AstNode_CompoundStatements::append_statement(AstNode_Statement* statement) {
    m_statements.push_back(statement);
}

llvm::Value* AstNode_CompoundStatements::codegen(LLVM_Compile_Context& context) const {
    for (auto statement : m_statements)
        statement->codegen(context);
    return nullptr;
}

llvm::Value* AstNode_Statement_Break::codegen(LLVM_Compile_Context& context) const {
    BasicBlock* bb = BasicBlock::Create(*context.context, "next_block");

    auto top = context.m_blocks.top();
    context.builder->CreateBr(top.second);

    auto function = context.builder->GetInsertBlock()->getParent();
    function->getBasicBlockList().push_back(bb);
    context.builder->SetInsertPoint(bb);

    return nullptr;
}

llvm::Value* AstNode_Statement_Continue::codegen(LLVM_Compile_Context& context) const {
    BasicBlock* bb = BasicBlock::Create(*context.context, "next_block");

    auto top = context.m_blocks.top();
    context.builder->CreateBr(top.first);

    auto function = context.builder->GetInsertBlock()->getParent();
    function->getBasicBlockList().push_back(bb);
    context.builder->SetInsertPoint(bb);

    return nullptr;
}

TSL_NAMESPACE_END
