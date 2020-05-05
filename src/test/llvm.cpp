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

#include "thirdparty/gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
using namespace llvm;

// this test is purely just to verify llvm is properly setup.
TEST(LLVM, LLVM_Configuration) {
    llvm::LLVMContext TheContext;
	std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("my cool jit", TheContext);
    EXPECT_NE(TheModule.get(), nullptr);
}

/*
 * Manually composing a shader like this through LLVM without Flex and Bison
 *    int return_123(){
 *        return 123;
 *    }
 * And it should return 123 when JITed.
 */
TEST(LLVM, LLVM_JIT) {
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();

	llvm::LLVMContext TheContext;
	std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("my cool jit", TheContext);
	EXPECT_NE(TheModule, nullptr);

	Function *function = Function::Create(FunctionType::get(Type::getInt32Ty(TheContext), {}, false), Function::ExternalLinkage, "return_123", TheModule.get());
	BasicBlock *bb = BasicBlock::Create(TheContext, "EntryBlock", function);
	IRBuilder<> builder(bb);
	builder.CreateRet(builder.getInt32(123));

	llvm::ExecutionEngine* EE = llvm::EngineBuilder(std::move(TheModule)).create();

	std::vector<GenericValue> noargs;
	GenericValue gv = EE->runFunction(function, noargs);
	
	EXPECT_EQ(gv.IntVal, 123);

	delete EE;
	// llvm_shutdown();
}

/*
 * Manually trying to call an external function defined in C++.
 *    float my_proxy_function(){
 *        float input_var = 12.0f;
 *        return external_cpp_function(input_var);
 *    }
 * And it should return what external_cpp_function returns when JITed.
 */

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT float llvm_test_external_cpp_function( float x ) {
	return x * x;
}

TEST(LLVM, LLVM_JIT_Ext_Func) {
	const float input_var = 12.0;

	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();

	llvm::LLVMContext TheContext;
	std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("my cool jit", TheContext);
	EXPECT_NE(TheModule, nullptr);

	// create external function prototype
	// this should perfectly match 'llvm_test_external_cpp_function' defined above.
	std::vector<Type *> proto_args(1, Type::getFloatTy(TheContext));
	Function *ext_function = Function::Create(FunctionType::get(Type::getFloatTy(TheContext), proto_args, false), Function::ExternalLinkage, "llvm_test_external_cpp_function", TheModule.get());

	// the main function to be executed
	Function *function = Function::Create(FunctionType::get(Type::getFloatTy(TheContext), {}, false), Function::ExternalLinkage, "my_proxy_function", TheModule.get());
	BasicBlock *bb = BasicBlock::Create(TheContext, "EntryBlock", function);
	IRBuilder<> builder(bb);

	// call the external defined function in C++, llvm_test_external_cpp_function
	std::vector<Value *> args(1);
	args[0] = ConstantFP::get(TheContext, APFloat(input_var));
	Value* value = builder.CreateCall(ext_function, args, "calltmp");

	// return whatever the call returns
	builder.CreateRet(value);

	// execute the jited function
	llvm::ExecutionEngine* EE = llvm::EngineBuilder(std::move(TheModule)).create();
	std::vector<GenericValue> noargs;
	GenericValue gv = EE->runFunction(function, noargs);
	
	const float expected_result = llvm_test_external_cpp_function(input_var);
	EXPECT_EQ(gv.FloatVal, expected_result);

	delete EE;
}