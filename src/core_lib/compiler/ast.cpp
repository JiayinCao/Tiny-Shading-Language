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

using namespace llvm;

TSL_NAMESPACE_BEGIN

llvm::Function* AstNode_FunctionPrototype::codegen( LLVM_Compile_Context& context ) const {
	int arg_cnt = 0;
	AstNode_VariableDecl* variable = m_variables;
	while( variable ){
		++arg_cnt;
		variable = castType<AstNode_VariableDecl>(variable->get_sibling());
	}

    // clear the symbol maps, no global var for now
    context.m_var_symbols.clear();

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

    context.m_func_symbols[m_name] = function;

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
            if( 0 != context.m_var_symbols.count(name) ){
                std::cout << "Duplicated argument named : " << name << std::endl;
                return nullptr;
            }

            auto arg = function->getArg(i);
            if (variable->get_config() & VariableConfig::OUTPUT) {
                context.m_var_symbols[name] = arg;
            } else {
                // allocate the variable on stack
                auto alloc_var = context.builder->CreateAlloca(raw_type, nullptr, name);

                // duplicate the value so that it can be a copy instead of a reference.
                context.builder->CreateStore(arg, alloc_var);
                
                context.m_var_symbols[name] = alloc_var;
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
    return get_llvm_mul(left, right, context);
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
    auto it = context.m_var_symbols.find(m_name);

    // undefined variable referenced here, emit warning
    if (it == context.m_var_symbols.end()) {
        std::cout << "Undefined variable : " << m_name << std::endl;
        return nullptr;
    }

    return context.builder->CreateLoad(it->second);
}

llvm::Value* AstNode_VariableRef::get_value_address(LLVM_Compile_Context& context) const {
    // just find it in the symbol table
    auto it = context.m_var_symbols.find(m_name);

    // undefined variable referenced here, emit warning
    if (it == context.m_var_symbols.end()) {
        std::cout << "Undefined variable : " << m_name << std::endl;
        return nullptr;
    }

    return it->second;
}

llvm::Value* AstNode_Statement_VariableDecls::codegen(LLVM_Compile_Context& context) const {
    AstNode_VariableDecl* decl = m_var_decls;
    while (decl) {
        auto name = decl->get_var_name();
        auto type = get_type_from_context(decl->data_type(), context);
        auto init = decl->get_init();

        // there is duplicated names, emit an warning!!
        // However, there is no log system in the compiler for now, I need to handle this later.
        if (0 != context.m_var_symbols.count(name)) {
            std::cout << "Duplicated argument named : " << name << std::endl;
            return nullptr;
        }

        // allocate the variable on stack
        auto alloc_var = context.builder->CreateAlloca(type, nullptr, name);

		// initialize it if necessary
		if(init){
			Value* init_value = init->codegen(context);

			if(init_value)
				context.builder->CreateStore(init_value, alloc_var);
		}

        context.m_var_symbols[name] = alloc_var;

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

llvm::Value* AstNode_FunctionCall::codegen(LLVM_Compile_Context& context) const {
    auto it = context.m_func_symbols.find(m_name);
    if (it == context.m_func_symbols.end()) {
        std::cout << "Undefined function " << m_name << "." << std::endl;
        return nullptr;
    }

    std::vector<Value*> args;
    AstNode_Expression* node = m_variables;
    while (node) {
        args.push_back(node->codegen(context));
        node = castType<AstNode_Expression>(node->get_sibling());
    }

    return context.builder->CreateCall(it->second, args);
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

    builder.SetInsertPoint(then_bb);
    auto statement = m_true_statements;
    while (statement) {
        statement->codegen(context);
        statement = castType<AstNode_Statement>(statement->get_sibling());
    }
    
    if( nullptr == then_bb->getTerminator() )
        builder.CreateBr(merge_bb);

    if (else_bb) {
        function->getBasicBlockList().push_back(else_bb);
        builder.SetInsertPoint(else_bb);

        auto statement = m_false_statements;
        while (statement) {
            statement->codegen(context);
            statement = castType<AstNode_Statement>(statement->get_sibling());
        }

        if (nullptr == else_bb->getTerminator())
            builder.CreateBr(merge_bb);
    }

    function->getBasicBlockList().push_back(merge_bb);
    builder.SetInsertPoint(merge_bb);

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

    return nullptr;
}

llvm::Value* AstNode_Statement_Loop_DoWhile::codegen(LLVM_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;

    auto function = context.builder->GetInsertBlock()->getParent();
    BasicBlock* loop_bb = BasicBlock::Create(llvm_context, "loop_block", function);
    BasicBlock* loop_end_bb = BasicBlock::Create(llvm_context, "loop_end");

    // push the loop blocks
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

    return nullptr;
}

llvm::Value* AstNode_Stament_Break::codegen(LLVM_Compile_Context& context) const {
    BasicBlock* bb = BasicBlock::Create(*context.context, "next_block");

    auto top = context.m_blocks.top();
    context.builder->CreateBr(top.second);

    auto function = context.builder->GetInsertBlock()->getParent();
    function->getBasicBlockList().push_back(bb);
    context.builder->SetInsertPoint(bb);

    return nullptr;
}

llvm::Value* AstNode_Stament_Continue::codegen(LLVM_Compile_Context& context) const {
    BasicBlock* bb = BasicBlock::Create(*context.context, "next_block");

    auto top = context.m_blocks.top();
    context.builder->CreateBr(top.first);

    auto function = context.builder->GetInsertBlock()->getParent();
    function->getBasicBlockList().push_back(bb);
    context.builder->SetInsertPoint(bb);

    return nullptr;
}

TSL_NAMESPACE_END
