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
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "ast.h"

TSL_NAMESPACE_ENTER

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
		variable = castType<AstNode_VariableDecl>(variable->getSibling());
	}

	// parse argument types
	std::vector<llvm::Type*>	args(arg_cnt);
	variable = m_variables;
	int i = 0;
	while( variable ){
		args[i++] = llvm_type_from_data_type( variable->dataType() , *context.context );
		variable = castType<AstNode_VariableDecl>(variable->getSibling());
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
		arg.setName(variable->getVarName());
		variable = castType<AstNode_VariableDecl>(variable->getSibling());
	}

	return function;
}

llvm::Function* AstNode_Shader::codegen( LLVM_Compile_Context& context ) const{
	// declare the function prototype
	llvm::Function* function = m_proto->codegen( context );
	if( !function )
		return nullptr;

	// create a separate code block
	llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context.context, "entry", function);
	context.builder->SetInsertPoint(BB);

	context.builder->CreateRetVoid();
	llvm::verifyFunction(*function);
	return function;

	if (llvm::Value *ret_val = m_body->codegen(context)) {
		// Finish off the function.
		context.builder->CreateRet(ret_val);

		// Validate the generated code, checking for consistency.
		llvm::verifyFunction(*function);

		return function;
	}

	// failed to create the function, erase it then.
	function->eraseFromParent();
	return nullptr;
}

llvm::Value* AstNode_FunctionBody::codegen( LLVM_Compile_Context& context ) const{
	return nullptr;
}

TSL_NAMESPACE_LEAVE