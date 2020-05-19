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
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "ast.h"

using namespace llvm;

TSL_NAMESPACE_BEGIN

static inline llvm::Type* llvm_type_from_data_type( const DataType type , llvm::LLVMContext& context ){
	switch(type){
	case DataType::INT:
		return llvm::Type::getInt32Ty(context);
	case DataType::FLOAT:
		return llvm::Type::getFloatTy(context);
	case DataType::FLOAT3:
		return llvm::Type::getFloatPtrTy(context);
	case DataType::FLOAT4:
		return llvm::Type::getFloatPtrTy(context);
    case DataType::DOUBLE:
        return llvm::Type::getDoubleTy(context);
	case DataType::MATRIX:
		return llvm::Type::getFloatPtrTy(context);
	case DataType::VOID:
		return llvm::Type::getVoidTy(context);
	case DataType::BOOL:
		return llvm::Type::getInt1Ty(context);
	default:
		break;
	}
	return nullptr;
}

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
        auto raw_type = llvm_type_from_data_type(variable->data_type(), *context.context);
        args[i++] = (variable->get_config() & VariableConfig::OUTPUT) ? raw_type->getPointerTo() : raw_type;
		variable = castType<AstNode_VariableDecl>(variable->get_sibling());
	}

	// parse return types
	llvm::Type* return_type = llvm_type_from_data_type( m_return_type , *context.context );

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
            const auto raw_type = llvm_type_from_data_type(variable->data_type(), *context.context);
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
    }

	return function;
}

llvm::Value* AstNode_FunctionBody::codegen( LLVM_Compile_Context& context ) const{    
	return nullptr;
}

llvm::Value* AstNode_Expression::codegen(LLVM_Compile_Context& context) const {
    return ConstantFP::get(*context.context, APFloat(1.0f));
}

llvm::Value* AstNode_Literal_Int::codegen(LLVM_Compile_Context& context) const {
    return ConstantInt::get(*context.context, APInt(32, m_val));
}

llvm::Value* AstNode_Literal_Flt::codegen(LLVM_Compile_Context& context) const {
    return ConstantFP::get(*context.context, APFloat(m_val));
}

llvm::Value* AstNode_Binary_Add::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context) && right->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFAdd(left, right);
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
        return context.builder->CreateAdd(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Minus::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context) && right->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFSub(left, right);
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
        return context.builder->CreateSub(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Multi::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context) && right->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFMul(left, right);
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
        return context.builder->CreateMul(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Div::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context) && right->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFDiv(left, right);
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
        return context.builder->CreateSDiv(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Mod::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    // not quite sure what to call for now
//    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
//        return context.builder->CreateSDiv(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_And::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    // not quite sure what to call for now
//    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
//          return context.builder->CreateAnd(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Or::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    // not quite sure what to call for now
//    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
//        return context.builder->CreateOr(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Eq::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFCmpOEQ(left, right);
    else
        return context.builder->CreateICmpEQ(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Ne::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFCmpONE(left, right);
    else
        return context.builder->CreateICmpNE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_G::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFCmpOGT(left, right);
    else
        return context.builder->CreateICmpSGT(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_L::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFCmpOLT(left, right);
    else
        return context.builder->CreateICmpSLT(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Ge::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFCmpOGE(left, right);
    else
        return context.builder->CreateICmpSGE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Le::codegen(LLVM_Compile_Context& context) const {
    auto left = m_left->codegen(context);
    auto right = m_right->codegen(context);

    if (left->getType() == llvm::Type::getFloatTy(*context.context))
        return context.builder->CreateFCmpOLE(left, right);
    else
        return context.builder->CreateICmpSLE(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_And::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
        return context.builder->CreateAnd(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_Or::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
        return context.builder->CreateOr(left, right);

    return nullptr;
}

llvm::Value* AstNode_Binary_Bit_Xor::codegen(LLVM_Compile_Context& context) const {
    Value* left = m_left->codegen(context);
    Value* right = m_right->codegen(context);

    // not quite sure what to call for now
    if (left->getType() == llvm::Type::getInt32Ty(*context.context) && right->getType() == llvm::Type::getInt32Ty(*context.context))
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
        auto type = llvm_type_from_data_type(decl->data_type(), *context.context);
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

llvm::Value* AstNode_ExpAssign::codegen(LLVM_Compile_Context& context) const{
	auto var = m_var->get_value_address(context);
	auto value = m_expression->codegen(context);

	context.builder->CreateStore(value, var);
	return value;
}

llvm::Value* AstNode_Statement_CompoundExpression::codegen(LLVM_Compile_Context& context) const{
	return m_expression->codegen(context);
}

llvm::Value* AstNode_Unary_Pos::codegen(LLVM_Compile_Context& context) const {
	return m_exp->codegen(context);
}

llvm::Value* AstNode_Unary_Neg::codegen(LLVM_Compile_Context& context) const{
	auto operand = m_exp->codegen(context);

	if (operand->getType() == llvm::Type::getFloatTy(*context.context))
		return context.builder->CreateFNeg(operand);
	if (operand->getType() == llvm::Type::getInt32Ty(*context.context))
		return context.builder->CreateNeg(operand);

	return nullptr;
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

    // this is to support implicty bool conversion
    if (cond->getType() == llvm::Type::getFloatTy(*context.context))
        cond = context.builder->CreateFCmpONE(cond, ConstantFP::get(*context.context, APFloat(0.0)));
    else if (!cond->getType()->isIntegerTy(1))
        cond = context.builder->CreateICmpNE(cond, ConstantInt::get(*context.context, APInt(cond->getType()->getIntegerBitWidth(), 0)));

    return context.builder->CreateSelect(cond, true_exp, false_exp);
}

llvm::Value* AstNode_Statement_Conditinon::codegen(LLVM_Compile_Context& context) const {
    auto& llvm_context = *context.context;
    auto& builder = *context.builder;
    
    auto cond = m_condition->codegen(context);

    // this should not happen, but it happens due to incomplete implementation.
    if (!cond)
        return nullptr;

    // this is to support implicty bool conversion
    if (cond->getType() == llvm::Type::getFloatTy(*context.context))
        cond = builder.CreateFCmpONE(cond, ConstantFP::get(llvm_context, APFloat(0.0)));
    else if (!cond->getType()->isIntegerTy(1))
        cond = builder.CreateICmpNE(cond, ConstantInt::get(llvm_context, APInt(cond->getType()->getIntegerBitWidth(), 0)));

    auto function = context.builder->GetInsertBlock()->getParent();

    BasicBlock* then_bb = BasicBlock::Create(llvm_context, "then", function);
    BasicBlock* else_bb = BasicBlock::Create(llvm_context, "else");
    BasicBlock* merge_bb = BasicBlock::Create(llvm_context, "ifcont");

    builder.CreateCondBr(cond, then_bb, else_bb);

    builder.SetInsertPoint(then_bb);
    if(m_true_statements)
        m_true_statements->codegen(context);
    builder.CreateBr(merge_bb);

    function->getBasicBlockList().push_back(else_bb);

    builder.SetInsertPoint(else_bb);
    if(m_false_statements)
        m_false_statements->codegen(context);
    builder.CreateBr(merge_bb);

    function->getBasicBlockList().push_back(merge_bb);
    builder.SetInsertPoint(merge_bb);

    return nullptr;
}

TSL_NAMESPACE_END
